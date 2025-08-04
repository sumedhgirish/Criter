# CWriter

## About the library

This is a single header library implementing the "writer" monad in C. The aim is make a seamless library that
adds log history to any data in C, without obstructing the control flow or code, or introducing unnecessary
complicated logic or large amount of redundant code.  

It is also easily extendible, meaning you can add your own flavour to logs, and make the library fit for your
specific use case.  

If you are confused about what monads are, or want to get your hands dirty with some examples, please see usage
section of the README.  

## Installation

...uhh, we don't do that around here. The only file you need is the [header](./writer.h) file, and some copy-pasting
skills. Put the header file in a proper location and you are good to go!  

#### Aside: Single Header Libraries
This C project is written fully within a single header file, which contains both the declaration, and the
implementation of the code. Normally, when the file is included, it behaves like a normal header file. We
can access the implementation by defining the ```WRITER_IMPLEMENTATION``` macro _before_ including the file.
**Care should be taken that the implementation part does not get included in multiple different places
in a project.** This can be avoided by only including the implementation along with the main function in
projects.

## Usage

todo
