#!/bin/sh

# Add any build steps you need here
make
# Warming
echo "Warming..."
./level0 < long.txt > /dev/null
./level0 < long.txt > /dev/null
./level0 < long.txt > /dev/null
