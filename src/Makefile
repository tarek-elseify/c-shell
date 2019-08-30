# Tarek Elseify
# February 4, 2019
# Lab 2 - Developing a Linux Shell
# CIS 3207 SEC 001
# tug35668@temple.edu
#
# This is the makefile for the shell program

CC=gcc
CFLAGS=-std=c11

all: clean myshell

myshell: shell.c built_in.c shell.h
	$(CC) -o myshell shell.c built_in.c

clean:
	$(RM) -rf myshell *.*~
