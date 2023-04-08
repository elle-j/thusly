# Thusly

A general-purpose programming language coming to life through a one-pass compiler and a stack-based virtual machine.

> ⚠️ The language is currently being developed but has a working initial implementation of a small subset (see [Milestones](#milestones)). This README is also in development.

## Table of Contents

- [Language](#language)
    - [Characteristics](#characteristics)
    - [Early Syntax](#early-syntax)
- [Milestones](#milestones)
    - [Implemented Functionality](#implemented-functionality)
- [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Building the Project](#building-the-project)
    - [Running Code](#running-code)
- [License](#license)

## Language

### Characteristics

* General-purpose
* Interpreted
* Dynamically typed
* Imperative
* Garbage collected

### Early Syntax 

The design and development are currently on-going and are highly subject to change. The following is a subset of the language:

<img src="design/code-snippet.svg" width="600" alt="A snippet of Thusly code.">

Whitespace is semantically insignificant except for newline characters on non-blank lines.

## Milestones

- [x] The terminals in the initial [grammar](design/grammar.txt) can be identified from user input via a multi-line file or single-line REPL input and then tokenized.
  * To try out the tokenizer in isolation and get printouts of the tokens produced use [PR #2](https://github.com/elle-j/thusly/pull/2) (significant newline characters will be printed as newlines).
- [x] One-line arithmetic expressions using `number` (double) can be executed.
  - [x] Addition (`+`)
  - [x] Subtraction (`-`)
  - [x] Multiplication (`*`)
  - [x] Division (`/`)
  - [x] Unary negation (`-`)
  - [x] Precedence altering (`()`)
- [x] One-line comparison, equality, and logical negation expressions using `number`, `boolean`, `text` (string), and `none` can be executed.
  - [x] Equal to (`=`)
  - [x] Not equal to (`!=`)
  - [x] Greater than (`>`)
  - [x] Greater than or equal to (`>=`)
  - [x] Less than (`<`)
  - [x] Less than or equal to (`<=`)
  - [x] Logical not (`not`)
- [x] Concatenation of `text` literals using `+` can be executed.
- [ ] Support variable declarations and assignments.
- [ ] TODO (more milestones will be added here)

### Implemented Functionality

This section is for briefly demonstrating implemented functionality thus far and expected behavior when [running your code](#getting-started).

By inputing a **one-line expression** from either a file or via the REPL, the VM will interpret it and output the result.

**Table 1: Valid user input**

| Example input              | Expected output | Expected precedence parsing   |
|----------------------------|-----------------|-------------------------------|
| 1 + 2 * 3 / 4              | 2.5             | 1 + ((2 * 3) / 4)             |
| (1 + 2) * 3 / 4            | 2.25            | ((1 + 2) * 3) / 4             |
| 1 + -2 - -3                | 2               | (1 + (-2)) - (-3)             |
| 1 > 2 = 3 > 4              | true            | (1 > 2) = (3 > 4)             |
| false != not(1 + 2 >= 3)   | false           | false != (not((1 + 2) >= 3))  |
| "he" + "llo" = "hello"     | true            | ("he" + "llo") = "hello"      |
| "keep " + "on " + "coding" | keep on coding  | ("keep " + "on ") + "coding"  |

**Table 2: Invalid user input**

| Example invalid input      | Error type   | Expected error reason                        |
|----------------------------|--------------|----------------------------------------------|
| "one" + 2                  | Runtime      | `+` operates on `number` only or `text` only |
| "one" < 2                  | Runtime      | `<` operates on `number` only                |
| !true                      | Compile      | `!` is only allowed in `!=` (use `not`)      |

## Getting Started

### Prerequisites

* A C compiler (e.g. Clang or GCC)
* [CMake](https://cmake.org/) version 3.20 or later

### Building the Project

Run the below command to make Thusly come to life. It will create a top-level `bin` directory where the configured and compiled project will be located along with the executable binary `cthusly`.

```sh
./build.sh
```

> If permission is denied, first add executable permission to the build script by running:
> `chmod +x build.sh`.

### Running Code

Once you have [built](#building-the-project) the project you can go ahead and feed it some code to interpret thusly (..get it?):

**Usage example** (use the flag `-h` or `--help`):

```
$ ./bin/cthusly --help

Usage: ./bin/cthusly [options] [path]

    REPL (interactive prompt) starts if no [path] is provided

    -h, --help                Show usage
```

**Interpret code from a file:**
```sh
./bin/cthusly path/to/your/file
```

**Start the REPL (interactive prompt):**
```sh
./bin/cthusly
```

Example:

```
$ ./bin/cthusly

> "he" + "llo" = "hello"
true
> (1 + 2) * 3 / 4
2.25
> 
```

> **Enable/disable debug output:**
>
> Comment or uncomment the following macros in [src/common.h](src/common.h) to disable or enable printouts (then [rebuild](#building-the-project) the project):
>
> **DEBUG_COMPILATION**
>   - Prints the entire bytecode produced by the compiler.
>
> **DEBUG_EXECUTION**
>   - Prints the VM execution steps including its stack state.

## License

This software is licensed under the terms of the [MIT license](LICENSE).
