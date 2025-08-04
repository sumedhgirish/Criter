# CWriter

## About the library

This is a single header library implementing the "writer" monad in C. The aim is make a seamless library that
adds log history to any data in C, without obstructing the control flow, introducing unnecessary and complicated
logic or large amount of redundant code.  

It is also easily extendible, meaning you can add your own flavour to logs, and make the library fit for your
specific use case.  

If you are confused about what monads are, or want to get your hands dirty with some examples, please see usage
section of the README below.  

## Installation

...uhh, we don't do that around here. The only file you need is the [header](./writer.h) file, and some copy-pasting
skills. Put the header file in a proper location and you are good to go!  

#### Aside: Single Header Libraries
This C project is written fully within a single header file, which contains both the declaration, and the
implementation of the code. Normally, when the file is included, it behaves like a normal header file. We
can access the implementation by defining the ```WRITER_IMPLEMENTATION``` macro _before_ including the file.
**Care should be taken that the implementation part does not get included in multiple different places
in a project.** This can be avoided by only including the implementation macro along with the main function in
projects.  

## Usage

The usage is split into 3 sections. The first section gives some motivation behind the design choices for the API, and what the
library tries to achieve and hence, what should and should not be used for. The second section shows the documentation for each
function, along with a brief explanation about how to use it. The third section shows how to use the library and its API via some
examples.

### Section 1: Motivation

This project takes its motivation from this [video](https://www.youtube.com/watch?v=C2w45qRc3aU) about monads and why they are
useful. It is recommended that you watch this video if you are not familiar with what monads are or how to use them.

### Section 2: Examples

The library api is mainly composed of 4 functions.

 > ```Wrap```
 > ```Log```
 > ```Unit```
 > ```Display```

#### Wrap
```{c}
DataWithLogs Wrap(DataType type, Any data);
```
    Exactly like its name, the ```Wrap``` function allows you to wrap any value to structure that supports logging into it, like ```DataWithLogs```.
    It takes in 2 parameters, a ```DataType``` that specifies the type of the value to be wrapped, and an ```Any``` value, which is specified by that
    type, and returns wrapped data.

#### Log
```{c}
void Log(DataWithLogs* to_ptr, LogLevel priority, const char* format, ...);
```
    The Log function allows to add logs to ```DataWithLogs```. It works exactly like printf function, except that it also takes in a ```LogLevel```
    along with the other parameters taken by the printf function, along with a pointer to the Data to which you want to add Logs to.


