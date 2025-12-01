# Testing

This directory contains a test harness for running successive updates and
verifying the state of the C64Music directory after each.

To save space each entry in `all-hashes.json` contains a "master" checksum for
that version of the HVSC. It's constructed by hashing each file and then hashing
the resulting hashes in a consistent order to arrive at a single hash.

```
{ "<version>": "hash", "<version2>": "hash2", ... }
```

A hash is constructed by running:

```
find . -type f -not -path "./update/*" -and -not -path "./readme.1st" -print0 \
     | xargs -0 shasum -b | sort | shasum -b
```

Against `C64Music` from that version.

`test.py` can both generate this file (which is slow as it requires downloading
and unpacking all-in-one archives) and verify updates against it.

The test harness was tested on Linux/macOS, but probably works on Windows in a
suitable UNIX-like environment. Run `test.py` for more information.
