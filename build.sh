#!/bin/sh

# Add any build steps you need here
git clone https://github.com/armon/libart.git
cd libart
cd deps/check-0.9.8/
./configure
make
cd ../../..

gcc -o libart/src/art.o -c -g -std=c99 -D_GNU_SOURCE -Wall -Werror -O3 -pthread -Ilibart/src/ libart/src/art.c
gcc -o l0 -c -g -std=c99 -D_GNU_SOURCE -Wall -Werror -O3 -pthread -Ilibart/src/ level0.c
gcc -Ofast -o level0 libart/src/art.o -Llibart/deps/check-0.9.8/src l0
