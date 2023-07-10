#!/bin/bash
#
# Quick test of update tool. Unpack collection, run a few updates, and then
# compare against a "golden" all-in-one. This script works on UNIX systems and
# in the "Git-bash" terminal on Windows (Msys), but YMMV.
set -e

# Updates to apply
APPLY=$(seq 73 79)
MIRROR=https://hvsc.brona.dk/HVSC

# Base HVSC
BASE=HVSC_72-all-of-them.7z

if [[ $(uname) == *MINGW* ]]; then
  TOOL=$PWD/build/native/Release/hvsc_update_tool.exe
  TOOL_7Z="/c/Program Files/7-Zip/7z.exe"
  TOOL_DOWNLOAD="curl -O"
else
  TOOL_7Z=$(which 7z)
  TOOL_DOWNLOAD=$(which wget)
  TOOL=$PWD/build/native/hvsc_update_tool
fi

if [[ ! -f $TOOL ]]; then
  echo "Need a copy of the tool in $TOOL"
  exit 1
fi

cd test/

if [[ ! -e $BASE ]]; then
  $TOOL_DOWNLOAD $MIRROR/$BASE
fi

# Download updates
for up in $APPLY; do
  fn=HVSC_Update_$up.7z
  if [[ -e $fn ]]; then continue; fi
  $TOOL_DOWNLOAD $MIRROR/$fn
done


rm -rf C64Music
"$TOOL_7Z" x -y $BASE

# Disables prompting in the update tool
export HVSC_NO_PROMPT=1

pushd C64Music
for up in $APPLY; do
  fn=HVSC_Update_$up.7z
  "$TOOL_7Z" x -y ../$fn
  pushd update
  $TOOL
  popd
  rm -rf update
done

# Out of C64Music
popd
set +e

# shasum everything then compare to a golden reference
find C64Music -type f | xargs shasum -b | sort > test.sums

# CRLF breaks the comparison. This isn't needed on Linux, so ignore if it fails
dos2unix test.sums || true

# Now compare
ls -l test.sums ../ref/*.sums
shasum test.sums ../ref/*.sums

diff test.sums ../ref/ref79.sums


echo
echo "Done. Compare sums"
