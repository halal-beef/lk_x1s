#!/bin/sh
set -e

./build.sh c1s
./build.sh c2s
./build.sh r8s
./build.sh x1s
./build.sh y2s
./build.sh z3s
echo All done!
