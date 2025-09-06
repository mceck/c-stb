#!/bin/bash
set -e
gcc -o tests/test tests/tests.c -lcurl
tests/test