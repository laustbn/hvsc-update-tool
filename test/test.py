#!/usr/bin/env python3
from __future__ import annotations

# Code for test harness. Download/prep/test. Diagnosis feature requires
# 3.14. The rest should work with (somewhat) older versions.

import os
import contextlib
import subprocess
import os.path
import shutil
import re
import json
import argparse
import hashlib
import textwrap
from urllib.request import Request, urlopen
from urllib.parse import urlparse
from concurrent.futures import ProcessPoolExecutor


def _ascii(byte: int) -> str:
    return chr(byte) if 32 <= byte <= 126 else "."


# This was cranked out by CoPilot and lightly edited. It seems to work
def hexdiff(a: bytes, b: bytes, *, width: int = 16, base_offset: int = 0) -> str:
    """
    Return a plain-text hex diff of two byte buffers.

    Format (per row):
      offset  a_hex  a_ascii  |  b_hex  b_ascii
              markers-under-a-hex         markers-under-b-hex
    Markers: '^^' under bytes that differ (or exist only on one side).
    """
    out = []
    n = max(len(a), len(b))

    for off in range(0, n, width):
        aa = a[off : off + width]
        bb = b[off : off + width]

        a_hex = []
        b_hex = []
        a_mrk = []
        b_mrk = []
        a_asc = []
        b_asc = []

        for i in range(width):
            ai = aa[i] if i < len(aa) else None
            bi = bb[i] if i < len(bb) else None
            diff = ai != bi

            if ai is None:
                a_hex.append("  ")
                a_mrk.append("  ")
                a_asc.append(" ")
            else:
                a_hex.append(f"{ai:02X}")
                a_mrk.append("^^" if diff else "  ")
                a_asc.append(_ascii(ai))

            if bi is None:
                b_hex.append("  ")
                b_mrk.append("  ")
                b_asc.append(" ")
            else:
                b_hex.append(f"{bi:02X}")
                b_mrk.append("^^" if diff else "  ")
                b_asc.append(_ascii(bi))

        # Optional extra space in the middle like classic hexdumps
        def _join_hex(parts):
            if width == 16:
                return " ".join(parts[:8]) + "  " + " ".join(parts[8:])
            return " ".join(parts)

        a_hex_s = _join_hex(a_hex)
        b_hex_s = _join_hex(b_hex)
        a_mrk_s = _join_hex(a_mrk)
        b_mrk_s = _join_hex(b_mrk)

        out.append(
            f"{base_offset + off:08X}  "
            f"{a_hex_s}  |{''.join(a_asc)}|  ||  "
            f"{b_hex_s}  |{''.join(b_asc)}|"
        )

        # Only show marker line if there is any difference on this row
        if any(m.strip() for m in a_mrk) or any(m.strip() for m in b_mrk):
            out.append(f"{'':8}  {a_mrk_s}  {' ' * (1 + width)}   ||  {b_mrk_s}")

    return "\n".join(out)


@contextlib.contextmanager
def chdir(directory):
    old = os.getcwd()
    os.chdir(directory)
    try:
        yield
    finally:
        os.chdir(old)


HVSC_LOC = "hvsc-archives"
MIRROR = "https://hvsc.brona.dk/HVSC"
REF = "all-hashes.json"

DEFAULT_COVER_EXE = "../build/native-cover/hvsc_update_tool"
DEFAULT_EXE = "../build/native/hvsc_update_tool"
DEFAULT_DEBUG_EXE = "../build/native-debug/hvsc_update_tool"


class UpdateError(Exception):
    """Thrown if there's a hash mismatch when verifying an update."""

    def __init__(self, message, version, all_sums):
        super().__init__(message)
        self.version = version
        self.all_sums = all_sums


def which(exe):
    p = subprocess.run(["which", exe])
    return p.returncode == 0


def llvm_profdata(*args):
    if not hasattr(llvm_profdata, "exe"):
        if which("xcrun"):
            # macOS via Xcode, just assume it exists
            llvm_profdata.exe = ["xcrun", "llvm-profdata"]
        elif which("llvm-profdata"):
            llvm_profdata.exe = ["llvm-profdata"]
        else:
            raise Exception("llvm-profdata not found")
    return subprocess.run(llvm_profdata.exe + list(args)).returncode


def llvm_cov(*args):
    if not hasattr(llvm_cov, "exe"):
        if which("xcrun"):
            llvm_cov.exe = ["xcrun", "llvm-cov"]
        elif which("llvm-cov"):
            llvm_cov.exe = ["llvm-cov"]
        else:
            raise Exception("llvm-cov not found")
    return subprocess.run(llvm_cov.exe + list(args)).returncode


def unrar(*args):
    if not hasattr(unrar, "exe"):
        if which("rar"):
            unrar.exe = ["rar", "x", "-y", "-inul"]
        elif which("unrar"):
            unrar.exe = ["unrar", "x", "-y", "-inul"]
        elif which("7z"):
            unrar.exe = ["7z", "x", "-y"]
        else:
            raise Exception("RAR unpacker not found")
    return subprocess.run(unrar.exe + list(args)).returncode


def get_buffer(url):
    print(f"Downloading {url}")
    req = Request(url)
    with urlopen(req) as response:
        return response.read()


def get(url):
    req = Request(url)
    with urlopen(req) as response:
        # Try to get filename from server and fall back to parsing URL
        cd = response.headers.get("Content-Disposition")
        filename = None
        if cd:
            m = re.search(r'filename="?([^\"]+)"?', cd)
            if m:
                filename = m.group(1)
        if not filename:
            filename = os.path.basename(urlparse(url).path)
        if not filename:
            raise Exception(f"Failed to determine filename for download of {url}")
        print(f"Downloading {filename}")
        with open(filename, "wb") as f:
            while True:
                chunk = response.read(8192)
                if not chunk:
                    break
                f.write(chunk)


def hvsc_all_in_one(v):
    return f"HVSC_{v}-all-of-them.rar"


def hvsc_update(v):
    return f"HVSC_Update_{v}.rar"


def download_file(mirror, filename):
    # TODO: should check size...
    if os.path.isfile(filename):
        return
    url = f"{mirror}/{filename}"
    get(url)


def download_update(mirror, version):
    filename = hvsc_update(version)
    download_file(mirror, filename)


def download_all_in_one(mirror, version):
    filename = hvsc_all_in_one(version)
    download_file(mirror, filename)


def unpack_update(version):
    filename = hvsc_update(version)
    if not os.path.isdir("C64Music"):
        raise Exception(f"Are we in the right place? {os.getcwd()}")
    with chdir("C64Music"):
        if unrar(f"../../{filename}") != 0:
            raise Exception(f"Failed unpacking {filename}")


def unpack_all_in_one(version, where):
    filename = hvsc_all_in_one(version)
    if os.path.isdir(where):
        return
    os.mkdir(where)
    with chdir(where):
        print(f"Unpack {filename}")
        unrar(f"../{filename}")


def shasum_file(filename):
    """Python implementation of shasum -b"""
    h = hashlib.sha1()
    with open(filename, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)

    unix_filename = filename.replace("\\", "/")
    return f"{h.hexdigest()} *{unix_filename}"


def shasum_buffer(buf):
    """Python implementation of shasum -b of stdin"""
    h = hashlib.sha1()
    h.update(buf)

    return f"{h.hexdigest()} *-"


def generate_hash(directory):
    # See README
    src_files = []
    with chdir(f"{directory}/C64Music"):
        for root, dirs, files in os.walk("."):
            if os.path.relpath(root, ".") == "update":
                dirs.clear()
                continue
            for file in files:
                path = os.path.join(root, file)
                rel_path = os.path.relpath(path, ".")
                if rel_path == "readme.1st":
                    continue
                src_files.append(path)

        with ProcessPoolExecutor() as executor:
            all_sums = sorted(executor.map(shasum_file, src_files))

        # Identical to output from shasum
        combined = "\n".join(all_sums) + "\n"

        final_sum = shasum_buffer(combined.encode("utf-8"))

        sha = final_sum[0:40]
        if not re.match(r"[0-9a-f]{40}", sha):
            raise Exception(f"{sha} is not a SHA-1 hash?")
        print(sha)
        return (sha, all_sums)


def generate_hashes(versions):
    hashes = dict()
    for v in versions:
        (h, all_sums) = generate_hash(f"{v}")
        hashes[v] = h
    return hashes


def prepare_for_updates(versions):
    base = versions[0]
    test_dir = "test"
    if os.path.isdir(test_dir):
        shutil.rmtree(test_dir)

    download_all_in_one(MIRROR, base)
    unpack_all_in_one(base, test_dir)
    return generate_hash(test_dir)[0]


def load_zstd_sums(fn):
    # Needs Python 3.14
    from compression import zstd

    with zstd.open(fn) as f:
        sums = json.load(f)
    return sums


def diff_file(sum_line: str, version: int):
    # sum_line has the format
    # ^SHA1SHA1SHA1SHA1SHA1SHA1SHA1SHA1SHA1 *./dir/filename
    fn = sum_line.split(" ")[1].removeprefix("*.")
    url = f"{MIRROR}/debug/archive/{version}/C64Music{fn}"

    ref = get_buffer(url)
    local = f"{HVSC_LOC}/test/C64Music{fn}"
    with open(local, "rb") as f:
        actual = f.read()

    print("Showing diff of EXPECTED | ACTUAL")
    print(hexdiff(ref, actual))


def diagnose_mismatch(e: UpdateError):
    """Attempt to diagnose a mismatch between expected sum and actual"""
    # Download all sums for update
    version = e.version
    sum_fn = f"sums_version_{version}.json.zst"
    download_file(f"{MIRROR}/debug/", sum_fn)

    # Read sums. Convert to set since we want to have efficient look ups.
    expected_sums = set(load_zstd_sums(sum_fn))
    actual_sums = set(e.all_sums)

    identical = []
    # remove any files that are correct
    for s in actual_sums:
        if s in expected_sums:
            identical.append(s)

    for s in identical:
        actual_sums.remove(s)
        expected_sums.remove(s)

    # print (some of) what remains
    print(f"{len(actual_sums)} sums found but not expected")
    print(f"{len(expected_sums)} sums expected but not found")

    # We still don't have anything to go on except the hashes, so just print the
    # diff of a random file. Reference files are downloaded from a server for
    # convenience and not cached.
    print("Random file with incorrect hash:")
    tmp = actual_sums.pop()
    print(tmp)
    diff_file(tmp, version)


# Download all-in-ones, unpack, and generate checksums. Slow so it doesn't run
# by default.
def prepare(versions):
    with chdir(HVSC_LOC):
        for v in versions:
            download_all_in_one(MIRROR, v)
            unpack_all_in_one(v, f"{v}")
        hashes = generate_hashes(versions)
        hashes_str = json.dumps(hashes, indent=2)
    with open(REF, "w") as file:
        file.write(hashes_str)


def perform_update(version, exe, use_wine, use_debugger):
    download_update(MIRROR, version)
    test_dir = "test"
    with chdir(test_dir):
        unpack_update(version)
        with chdir("C64Music/update"):
            if use_wine:
                subprocess.run(["wine", exe], check=True)
            else:
                args = [exe]
                if use_debugger:
                    args = ["lldb", "--batch", "-o", "run", "--"] + args
                subprocess.run(args, check=True)
        return generate_hash(".")


def cover_fn(v):
    return f"hvsc-update-tool-update-{v}.profraw"


def run_test(versions, exe, use_wine=False, use_debugger=False, cover=False):
    with open(REF, "r") as file:
        hashes = json.load(file)

    for v in versions:
        if f"{v}" not in hashes:
            print(f"Missing sum for version {v}")
            exit(1)

    with chdir(HVSC_LOC):
        base_hash = prepare_for_updates(versions)
        if base_hash != hashes[f"{versions[0]}"]:
            raise Exception(
                f"Base hash mismatch. Got {base_hash}, expected {hashes[f'{versions[0]}']}"
            )
        else:
            print("Base hash OK")

        os.environ["HVSC_NO_PROMPT"] = "1"

        try:
            for v in versions[1:]:
                if cover:
                    os.environ["LLVM_PROFILE_FILE"] = os.path.abspath(cover_fn(v))
                (h, all_sums) = perform_update(v, exe, use_wine, use_debugger)
                if h != hashes[f"{v}"]:
                    raise UpdateError(
                        f"Hash mismatch. Got {h}, expected {hashes[f'{v}']}",
                        v,
                        all_sums,
                    )
        except Exception as e:
            print(f"Caught while running update {v}")
            raise e

    print("All updates verified")


def generate_coverage_report(versions, exe):
    """Generate a code coverage HTML report"""
    with chdir(HVSC_LOC):
        # merge all .profraw into .profdata
        cover_files = []
        for v in versions[1:]:
            cover_files.append(cover_fn(v))

        ret = llvm_profdata("merge", "-sparse", "-o", "hvsc.profdata", *cover_files)

        if ret != 0:
            raise Exception("Failed to run llvm-profdata")

        # Generate HTML report
        shutil.rmtree("coverage-html", ignore_errors=True)
        ret = llvm_cov(
            "show",
            exe,
            "-instr-profile=hvsc.profdata",
            "-format=html",
            "-output-dir=coverage-html",
        )

        if ret != 0:
            raise Exception("Failed to run llvm-cov")


# Ensure exe exists and convert path into absolute
def prepare_exe(fn):
    if fn == "unset":
        raise Exception("Must specify exe")
    if not os.path.isfile(fn):
        raise Exception(f"{fn} does not exist")
    # We chdir all over the place, so make it absolute
    return os.path.abspath(fn)


def parse_versions_range(versions):
    r = eval(f"range({versions})")
    # Range isn't inclusive
    rl = list(r)
    rl.append(rl[-1] + 1)
    return rl


def main():
    # See https://docs.python.org/3/library/urllib.request.html
    os.environ["no_proxy"] = "*"

    parser = argparse.ArgumentParser(
        prog="test.py",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        epilog=textwrap.dedent(
            f"""\
        When --cover is specified, the default exe is {DEFAULT_COVER_EXE}\
        """
        ),
    )
    parser.add_argument(
        "--exe",
        default=DEFAULT_EXE,
        help="Update executable to test",
    )
    parser.add_argument(
        "--versions",
        default="44,83",
        help="Range of versions to test. n is unpacked; n+1 to m applied as updates",
        metavar="n,m",
    )
    parser.add_argument(
        "--wine",
        action="store_true",
        default=False,
        help="Setup Wine prefix and invoke via Wine.",
    )

    parser.add_argument(
        "--debug",
        action="store_true",
        default=False,
        help="Wrap update tool in debugger (with autostart)",
    )

    parser.add_argument(
        "--diagnose",
        action="store_true",
        default=False,
        help="If checksum verification fails, attempt to find files with differences",
    )

    parser.add_argument(
        "--cover",
        action="store_true",
        default=False,
        help="Setup environment to capture code coverage data",
    )

    parser.add_argument("action", help="test|prepare")

    args = parser.parse_args()

    if not os.path.isdir(HVSC_LOC):
        os.mkdir(HVSC_LOC)

    if args.action == "prepare":
        prepare(range(44, 84))
    elif args.action == "test":
        if args.cover and args.debug:
            print("Cannot specify cover and debug at the same time")
            exit(1)
        # Change defaults depending on build type, but allow override
        if args.cover and args.exe == parser.get_default("exe"):
            # Change the default exe
            abs_exe = prepare_exe(DEFAULT_COVER_EXE)
        elif args.debug and args.exe == parser.get_default("exe"):
            abs_exe = prepare_exe(DEFAULT_DEBUG_EXE)
        else:
            abs_exe = prepare_exe(args.exe)

        versions = parse_versions_range(args.versions)
        # Always teardown/recreate Wine prefix
        if args.wine:
            os.environ["WINEPREFIX"] = f"{os.getcwd()}/wine_prefix"
            subprocess.run(["wineboot", "--shutdown"])
            shutil.rmtree(os.environ["WINEPREFIX"])
            subprocess.run(["wineboot"])
        try:
            run_test(versions, abs_exe, args.wine, args.debug, args.cover)
        except UpdateError as e:
            print(e)
            if args.diagnose:
                diagnose_mismatch(e)
                exit(1)
            else:
                print("Run with --diagnose for more information")

        if args.cover:
            generate_coverage_report(versions, abs_exe)

    else:
        print("Must specify action")
        exit(1)


if __name__ == "__main__":
    main()
