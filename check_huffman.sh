#!/usr/bin/env bash

tmp_archived=$(mktemp)
tmp_unarchived=$(mktemp)

echo "Archive Temp: $tmp_archived"
echo "Decoded Temp: $tmp_unarchived"

echo "Compressing..."
bazel-bin/demo/huffman_encode $1 $tmp_archived

echo "Decompressing..."
bazel-bin/demo/huffman_decode $tmp_archived $tmp_unarchived
echo "Done."

sha256_in=($(sha256sum -b $1))
sha256_out=($(sha256sum -b $tmp_unarchived))


if [ "$sha256_in" == "$sha256_out" ]; then
  echo "Success."
else
  echo "Failed."
  exit 1
fi
