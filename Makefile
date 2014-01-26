all: bloom

bloom: level0.c
	gcc -Ofast -o level0 level0.c

tiny: tiny.asm
	nasm -f macho -o tiny.o --prefix _ tiny.asm
	ld -o tiny tiny.o -arch i386 -lc -macosx_version_min 10.6 -e _main -no_pie
                                                                                                                                                                                                                                  