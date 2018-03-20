#!/usr/bin/make -f
# -*- mode:makefile -*-

CC := mpicc
DIREXEC := ejecutables/
RUN := mpirun

dirs:
	mkdir -p $(DIREXEC)

red_toroide: dirs
	$(CC) red_toroide.c -o $(DIREXEC)toroide

red_hipercubo: dirs
	$(CC) red_hipercubo.c -o $(DIREXEC)hipercubo

all: red_toroide red_hipercubo

run_hipercubo: red_hipercubo
	$(RUN) -n 16 $(DIREXEC)hipercubo

run_toroide: red_toroide
	$(RUN) -n 9 $(DIREXEC)toroide

clean:
	rm -rf $(DIREXEC)

