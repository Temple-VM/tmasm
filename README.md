<p align="center">
	<h1 align="center">TMASM</h2>
	<p align="center">An assembler for the Temple VM</p>
</p>
<p align="center">
	<a href="./LICENSE">
		<img alt="License" src="https://img.shields.io/badge/license-GPL-blue?color=7aca00"/>
	</a>
	<a href="https://github.com/LordOfTrident/tmasm/issues">
		<img alt="Issues" src="https://img.shields.io/github/issues/LordOfTrident/tmasm?color=0088ff"/>
	</a>
	<a href="https://github.com/LordOfTrident/tmasm/pulls">
		<img alt="GitHub pull requests" src="https://img.shields.io/github/issues-pr/LordOfTrident/tmasm?color=0088ff"/>
	</a>
	<br><br><br>
</p>

## Table of contents
* [Introduction](#introduction)
* [Quickstart](#quickstart)
* [Compile the examples](#compile-and-run-the-examples)
* [Milestones](#Milestones)
* [Documentation](#documentation)
* [Bugs](#bugs)
* [Dependencies](#dependencies)
* [Make](#make)

## Introduction
An assembler for the [Temple VM](https://github.com/lordoftrident/temple) written in C.

## Quickstart
```sh
$ make
$ make install
$ ./bin/app
```

## Compile and run the examples
To compile the examples, run `make example`. All the binaries will be generated in the `./examples`
folder.
To run a binary, you need to install the [Temple VM](https://github.com/lordoftrident/temple)

## Milestones
- [X] Labels, compile all instructions from Temple 1.0.0 and generate an executable
- [ ] Data segment
- [ ] Include keyword

## Documentation
Coming soon.

## Bugs
If you find any bugs, please create an issue and report them.

## Dependencies
- A unix system

## Make
Run `make all` to see all the make rules.
