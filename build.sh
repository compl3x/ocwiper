#!/bin/bash
echo "cwipe compile script v0.1"
echo "GCC Version: $(gcc -dumpversion)"
echo "Output: $(pwd)/out/wipe"
mkdir -p out && gcc -o ./out/ocwiper wipe.c mtwist.c