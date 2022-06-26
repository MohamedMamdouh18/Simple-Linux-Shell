# Simple-Linux-Shell
Simple linux shell using C programming language. 

## Table of Contents

- [Introduction](#Introduction)
    - [What is Shell](#What-is-Shell)
    - [What is it for](#What-is-it-for)
- [Supported Commands](#Supported-Commands)
- [Used Libraries](#Used-Libraries)
- [User Manual](#User-Manual)
    - [Download](#Download)
    - [Compilation](#Compilation)
    - [Running](#Running)
- [Features](#Features)

## Introduction
### What is Shell
A shell is a command-line interpreter, it is the computer program that provides a user interface to access the services of the operating system.
### What is it for
This consists of interpreting orders. It incorporates features such as process control, input/output redirection, law listing and reading, protection, communications, and a command language for writing batch programs or scripts. All Unix-type systems have at least one interpreter compatible with the Bourne shell. The Bourne shell program is found within the Unix file hierarchy at /bin/sh.

## Supported Commands
- built-in commands :
     - cd 
     - echo 
     - export
- non built-in commands :
    - ls
    - mv
    - cp
    - mkdir
    - etc...

## Used Libraries
- stdio.h
- unistd.h
- pwd.h
- string.h
- sys/wait.h
- stdlib.h
- search.h
- fcntl.h

## User Manual
### Download
You can clone this repository this way: 
```
git clone https://github.com/MohamedMamdouh18/Simple-Linux-Shell.git
```
### Compilation
- by typing `gcc -o shell main.c` in your command line
- or by typing `make` and the `Makefile` file will do rest the work.
```
mohamed@mohamed:~/Desktop/shell$ make
gcc -o shell main.c
```
### Running
after comilation phase type `./shell` and the shell will run.
```
mohamed@mohamed:~/Desktop/shell$ ./shell
```
and the files in the folder should be like

![image](https://drive.google.com/uc?export=view&id=1kVPEJcrWf4XFTwMZvG4F51X8cVhd9WQB)

## Features
- The shell is detecting the user's name and the pc name and print them colored like the original shell
![image](https://drive.google.com/uc?export=view&id=1-vklIZbIwvuJttR0ozUCHHlENWttvBzc)
- It can handle background commands by typing `&` after the command
```
mohamed@mohamed:/home/mohamed/Desktop/shell$ ls -h -l &
mohamed@mohamed:/home/mohamed/Desktop/shell$ total 40K
-rw-r--r-- 1 mohamed mohamed   0 يون 25 16:50 log.txt
-rw-rw-r-- 1 mohamed mohamed 13K مار 24 23:41 main.c
-rw-rw-r-- 1 mohamed mohamed  25 يون 25 14:17 Makefile
-rwxrwxr-x 1 mohamed mohamed 19K يون 25 16:50 shell
```
then after the background command is finished it will write `Background Child Terminated` in `log.txt` file


