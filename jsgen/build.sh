#!/bin/bash
set -e
cc jsgen.c -o jsgen

echo "Build complete."
./jsgen