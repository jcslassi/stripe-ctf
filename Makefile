all: bloom

CLANG := clang
GCC := gcc
CFLAGS := -std=c99
REMOTE_HOST := ubuntu@es_prod-indexer-0
OPTIMIZER_SETTINGS := -m64 -Ofast -flto -march=native -funroll-loops

bloom: level0.c
	#gcc $(CFLAGS) -c $(OPTIMIZER_SETTINGS) -o level0 level0.c
	gcc -m64 -march=native -Ofast -funroll-loops -o level0 level0.c

tiny: tiny.asm
	nasm -f macho -o tiny.o --prefix _ tiny.asm
	ld -o tiny tiny.o -arch i386 -lc -macosx_version_min 10.6 -e _main -no_pie
  
                                                 