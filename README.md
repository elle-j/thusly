# Thusly

A stack-based virtual machine for the **Thusly** general-purpose programming language.

> ⚠️ The language is in a design phase but has a working initial implementation of a small subset (see [Milestones](#milestones)). This README is also in development.

## Table of Contents

- [Language](#language)
    - [Characteristics](#characteristics)
    - [Early Syntax](#early-syntax)
- [Milestones](#milestones)
    - [Example of Current State](#example-of-current-state)
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

Whitespace is semantically insignificant; however, newlines on non-blank lines are significant and denote either the start of a block or end of a statement.

## Milestones

- [x] The terminals in the initial [grammar](design/grammar.txt) can be identified from user input via a multi-line file or single-line REPL input and then tokenized.
  * To try out the tokenizer and get printouts of the tokens produced use [PR #2](https://github.com/elle-j/thusly/pull/2).
- [x] One-line arithmetic expressions using `number`s (`double`) can be parsed and compiled.
  - [x] Addition (`+`)
  - [x] Subtraction (`-`)
  - [x] Multiplication (`*`)
  - [x] Division (`/`)
  - [x] Unary negation (`-`)
  - [x] Precedence altering (`()`)
- [x] One-line arithmetic expressions using `number`s can be executed.
  - [x] Same expressions as above.
- [ ] Support `none`, `boolean`, and `text` data types.
- [ ] Support variable declarations and assignments.
- [ ] TODO (more milestones will go here)

### Example of Current State

When [running your code](#getting-started) by inputing a **one-line expression** from either a file or via the REPL, the VM will interpret it and output the result.

| Example input    | Expected output | Expected precedence parsing |
|------------------|-----------------|-----------------------------|
| 1 + 2 * 3 / 4    | 2.5             | 1 + ((2 * 3) / 4)           |
| (1 + 2) * 3 / 4  | 2.25            | ((1 + 2) * 3) / 4           |
| 1 + -2 - -3      | 2               | (1 + (-2)) - (-3)           |

> Note: A debug flag is enabled which also prints the entire bytecode produced by the compiler, as well as the execution steps by the VM including the stack state.

## Getting Started

### Prerequisites

* A C compiler (e.g. Clang or GCC)
* [CMake](https://cmake.org/) version 3.20 or higher

### Building the Project

Run the below command to make Thusly come to life. It will create a top-level `bin` directory where the configured and compiled project will be located along with the executable binary `cthusly`.

```sh
./build.sh
```

> If permission is denied, first add executable permission to the build script by running `chmod +x build.sh`.

### Running Code

Once you have [built](#building-the-project) the project you can go ahead and feed it some code to interpret thusly (..get it?):

**Interpret code from a file:**
```sh
./bin/cthusly path/to/file
```

**Start the REPL (interactive prompt):**
```sh
./bin/cthusly
```

## License

This software is licensed under the terms of the [MIT license](LICENSE).
