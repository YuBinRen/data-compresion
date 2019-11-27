#!/usr/bin/env bash

tmp_archived=$(mktemp)
tmp_unarchived=$(mktemp)

echo "Compressing..."
bazel-bin/demo/lzw_compress $1 $tmp_archived 

echo "Decompressing..."
bazel-bin/demo/lzw_decompress $tmp_archived $tmp_unarchived > /dev/null
echo "Done."

sha256_in=($(sha256sum -b $1))
sha256_out=($(sha256sum -b $tmp_unarchived))

rm $tmp_archived $tmp_unarchived

if [ "$sha256_in" == "$sha256_out" ]; then
  echo "Success."
else
  echo "Failed."
  exit 1
fi
