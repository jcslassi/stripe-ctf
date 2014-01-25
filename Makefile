all: bloom

bloom: level0.c
	cc -O3 -o level0 -Wall -pedantic level0.c
