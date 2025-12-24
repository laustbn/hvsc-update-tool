#!/usr/bin/env python3

# Code for test harness. Download/prep/test. This was written using Python
# 3.12, but probably runs with earlier versions.

import os
import contextlib
import subprocess
import os.path
import shutil
import re
import json
import argparse
import hashlib
from concurrent.futures import ProcessPoolExecutor


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


def which(exe):
    p = subprocess.run(["which", exe])
    return p.returncode == 0


def unrar(*args):
    if not hasattr(unrar, "exe"):
        if which("rar"):
            unrar.exe = ["rar", "x", "-y", "-inul"]
        elif which("unrar"):
            unrar.exe = ["unrar", "x", "-y", "-inul"]
        else:
            raise Exception("RAR unpacker not found")
    return subprocess.run(unrar.exe + list(args)).returncode


def get(url):
    if not hasattr(get, "exe"):
        if which("curl"):
            get.exe = ["curl", "-O"]
        elif which("wget"):
            get.exe = ["wget"]
        else:
            raise Exception("Downloader not found")

    return subprocess.run(get.exe + [url]).returncode


def hvsc_all_in_one(v):
    return f"HVSC_{v}-all-of-them.rar"


def hvsc_update(v):
    return f"HVSC_Update_{v}.rar"


def download_file(mirror, filename):
    # TODO: should check size...
    if os.path.isfile(filename):
        return
    url = f"{mirror}/{filename}"
    ret = get(url)
    if ret != 0:
        raise Exception(f"Download of {url} returned {ret}")


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
        return sha


def generate_hashes(versions):
    hashes = dict()
    for v in versions:
        hashes[v] = generate_hash(f"{v}")
    return hashes


def prepare_for_updates(versions):
    base = versions[0]
    test_dir = "test"
    if os.path.isdir(test_dir):
        shutil.rmtree(test_dir)

    download_all_in_one(MIRROR, base)
    unpack_all_in_one(base, test_dir)
    return generate_hash(test_dir)


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


def run_test(versions, exe, use_wine=False, use_debugger=False):
    with open(REF, "r") as file:
        hashes = json.load(file)

    for v in versions:
        if not f"{v}" in hashes:
            print(f"Missing sum for version {v}")
            exit(1)

    with chdir(HVSC_LOC):
        base_hash = prepare_for_updates(versions)
        if base_hash != hashes[f"{versions[0]}"]:
            raise Exception(
                f"Base hash mismatch. Got {base_hash}, expected {hashes[f"{versions[0]}"]}"
            )
        else:
            print("Base hash OK")

        os.environ["HVSC_NO_PROMPT"] = "1"
        try:
            for v in versions[1:]:
                h = perform_update(v, exe, use_wine, use_debugger)
                if h != hashes[f"{v}"]:
                    raise Exception(
                        f"Hash mismatch. Got {h}, expected {hashes[f"{v}"]}"
                    )
        except Exception as e:
            print(f"Caught while running update {v}")
            raise e

    print("All updates verified")


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
    parser = argparse.ArgumentParser(
        prog="test.py", formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        "--exe",
        default="../build/native/hvsc_update_tool",
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

    parser.add_argument("action", help="test|prepare")

    args = parser.parse_args()

    if not os.path.isdir(HVSC_LOC):
        os.mkdir(HVSC_LOC)

    if args.action == "prepare":
        prepare(range(44, 84))
    elif args.action == "test":
        abs_exe = prepare_exe(args.exe)
        versions = parse_versions_range(args.versions)
        # Always teardown/recreate Wine prefix
        if args.wine:
            os.environ["WINEPREFIX"] = f"{os.getcwd()}/wine_prefix"
            subprocess.run(["wineboot", "--shutdown"])
            shutil.rmtree(os.environ["WINEPREFIX"])
            subprocess.run(["wineboot"])
        run_test(versions, abs_exe, args.wine, args.debug)
    else:
        print("Must specify action")
        exit(1)


if __name__ == "__main__":
    main()
