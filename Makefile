# this is makefile for MeMS

all: clean example 

example: example.c mems.h
	gcc -o example example.c -lrt

clean:
	rm -rf example