#!/bin/sh

# Add any build steps you need here
make
# Warming
echo "Warming..."
if [ -f ./level0 ]; then
    strip level0
    ./level0 < long.txt > /dev/null
    ./level0 < long.txt > /dev/null
    ./level0 < long.txt > /dev/null
    ./level0 < long.txt > /dev/null
    ./level0 < long.txt > /dev/null
fi    
