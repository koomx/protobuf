#!/usr/bin/env bash

# Regenerate the checked-in C++ for built-in protos.
# sharkc is compiled from these files, so CMake cannot generate them.

set -eu

root=$(cd "$(dirname "$0")" && pwd)
cd "$root"

if [[ ! -f shark/descriptor.proto ]]; then
  echo "Could not find shark/descriptor.proto. Run this script from the repository." >&2
  exit 1
fi

runtime_protos=(
  shark/any.proto
  shark/api.proto
  shark/duration.proto
  shark/empty.proto
  shark/field_mask.proto
  shark/source_context.proto
  shark/struct.proto
  shark/timestamp.proto
  shark/type.proto
  shark/wrappers.proto
)

build_dir=build
sharkc="$build_dir/shark/sharkc"

cmake --preset=vcpkg

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

round=1
while true; do
  echo "Round $round"
  cmake --build "$build_dir" --target sharkc

  "$sharkc" -I . --cpp_out="$tmp" "${runtime_protos[@]}"
  "$sharkc" -I . --cpp_out="dllexport_decl=SHARK_EXPORT:$tmp" shark/descriptor.proto
  "$sharkc" -I . --cpp_out="dllexport_decl=PROTOC_EXPORT:$tmp" shark/compiler/plugin.proto

  changed=0
  for proto in "${runtime_protos[@]}" shark/descriptor.proto shark/compiler/plugin.proto; do
    base=${proto%.proto}
    if ! cmp -s "$base.pb.h" "$tmp/$base.pb.h" || ! cmp -s "$base.pb.cc" "$tmp/$base.pb.cc"; then
      cp "$tmp/$base.pb.h" "$base.pb.h"
      cp "$tmp/$base.pb.cc" "$base.pb.cc"
      changed=1
    fi
  done

  if [[ $changed -eq 0 ]]; then
    echo "Built-in protos are up to date."
    break
  fi

  round=$((round + 1))
  if [[ $round -gt 5 ]]; then
    echo "Built-in protos were still changing after 5 rounds." >&2
    exit 1
  fi
done
