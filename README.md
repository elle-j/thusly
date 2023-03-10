# Thusly

A stack-based virtual machine for the **Thusly** general-purpose programming language.

> ⚠️ The language is in a design phase but has a working initial implementation of a small subset (see [Milestones](#milestones)). This README is also in development.

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

Whitespace is semantically insignificant; however, newlines on non-blank lines are significant.

## Milestones

- [x] The terminals in the initial [grammar](design/grammar.txt) can be identified from user input via a multi-line file or single-line REPL input and then tokenized.
  * To try out the tokenizer in isolation and get printouts of the tokens produced use [PR #2](https://github.com/elle-j/thusly/pull/2) (significant newline characters will be printed as newlines).
- [x] One-line arithmetic expressions using `number` (`double`) can be parsed and compiled.
  - [x] Addition (`+`)
  - [x] Subtraction (`-`)
  - [x] Multiplication (`*`)
  - [x] Division (`/`)
  - [x] Unary negation (`-`)
  - [x] Precedence altering (`()`)
- [x] One-line arithmetic expressions using `number` can be executed.
- [x] One-line comparison, equality, and logical negation expressions using `number`, `boolean`, and `none` can be executed.
  - [x] Equal to (`=`)
  - [x] Not equal to (`!=`)
  - [x] Greater than (`>=`)
  - [x] Greater than or equal to (`>=`)
  - [x] Less than (`<=`)
  - [x] Less than or equal to (`<=`)
  - [x] Logical not (`not`)
- [ ] Support `text` (`string`) data type.
- [ ] Support variable declarations and assignments.
- [ ] TODO (more milestones will be added here)

### Implemented Functionality

This section is for briefly demonstrating implemented functionality and what to expect when [running your code](#getting-started).

By inputing a **one-line expression** from either a file or via the REPL, the VM will interpret it and output the result.

| Example input            | Expected output | Expected precedence parsing   |
|--------------------------|-----------------|-------------------------------|
| 1 + 2 * 3 / 4            | 2.5             | 1 + ((2 * 3) / 4)             |
| (1 + 2) * 3 / 4          | 2.25            | ((1 + 2) * 3) / 4             |
| 1 + -2 - -3              | 2               | (1 + (-2)) - (-3)             |
| 1 > 2 = 3 > 4            | true            | (1 > 2) = (3 > 4)             |
| false != not(1 + 2 >= 3) | false           | false != (not((1 + 2) >= 3))  |

> Note: A debug flag is enabled which also prints the entire bytecode produced by the compiler, as well as the execution steps by the VM including its stack state.

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
