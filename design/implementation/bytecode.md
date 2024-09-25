# Thusly Design - Bytecode & Instruction Format

> 👉️ These design documents are currently being added to incrementally.

## Table of Contents

- [Abstract](#abstract)
- [Format](#format)
    - [Loading Constants](#loading-constants)
    - [Branching](#branching)
    - [Arithmetic](#arithmetic)

## Abstract

The bytecode instructions written by the compiler adhere to the format supported by the virtual machine (VM). It specifies the size of an instruction as well as what the various bits of the instruction will be interpreted as.

> [!TIP]
> Try out the web-based [interactive playground](https://thusly.netlify.app/) to write some Thusly code and see the bytecode it compiles to!

## Format

The Thusly VM has different instruction formats depending on the type of operation to perform, and the size of an instruction ranges from 8 bits (1 byte) to 24 bits (3 bytes).

In order for the VM to *decode* the instruction and know which type of operation to perform, the first 8 bits of an instruction always represent the *opcode*.

Some of the supported instructions are explained below.

### Loading Constants

When, for instance, a number literal is used in the source code, the VM will know to use, or load, that constant by the opcode `OP_CONSTANT`. However, the opcode itself does not indicate which constant to load, so all `OP_CONSTANT` instructions also have an 8-bit operand representing the *location* of where to get the actual constant.

| Opcode (8 bits) | Operand Size | Total Size | Example Instruction |
|:----------------|:------------:|:----------:|:-------------------:|
| OP_CONSTANT     | 8 bits       | 16 bits    | OP_CONSTANT 3       |

For some commonly used constants such as `true`, `false`, and `none`, the instruction set supports dedicated operations without an operand to bypass retrieving the constant from a different location.

| Opcode (8 bits)   | Operand Size | Total Size | Example Instruction |
|:------------------|:------------:|:----------:|:-------------------:|
| OP_CONSTANT_TRUE  | 0            | 8 bits     | OP_CONSTANT_TRUE    |
| OP_CONSTANT_FALSE | 0            | 8 bits     | OP_CONSTANT_FALSE   |
| OP_CONSTANT_NONE  | 0            | 8 bits     | OP_CONSTANT_NONE    |

### Branching

Since Thusly has branching constructs such as `if`, `foreach`, and `while`, the compiler needs to write instructions to *jump* over some of the bytecode. To know how far to jump, all jump instructions have a 16-bit operand representing the offset (in bytes) from the current instruction, i.e. how many bytes to jump forward or backward in the bytecode.


| Opcode (8 bits) | Operand Size | Total Size | Example Instruction |
|:----------------|:------------:|:----------:|:-------------------:|
| OP_JUMP_FWD     | 16 bits      | 24 bits    | OP_JUMP_FWD 10      |
| OP_JUMP_BWD     | 16 bits      | 24 bits    | OP_JUMP_BWD 13      |

### Arithmetic

When the parser parses a binary expression, let's say `1.2 + 3`, the corresponding bytecode should coincide with the stack-based nature of the VM internals. In short, the VM only operates on values on its stack, so constants used must first be loaded and pushed onto the stack.

Let's say we first load the constant `1.2`:

```
Stack top

1.2

Stack bottom
```

Then we load the constant `3`:

```
Stack top

3
1.2

Stack bottom
```

Since the left and right operands of the `+` operator are now on the stack, a bytecode instruction to perform a binary operation must only specify the type of operation--the opcode--*without* any operands. When decoding an addition instruction (`OP_ADD`), the VM expects the operands of the binary expression to be on the top of the stack. Specifically, the right one at the very top, and the left one immediately below.

The VM can then pop those values from the stack, perform the appropriate operation, and push the result back onto the stack. (You can read more about the VM internals in [vm.md](./vm.md)).

Thus, when parsing and compiling `1.2 + 3`, in addition to `OP_CONSTANT` instructions explained in an [earlier section](#loading-constants), an `OP_ADD` instruction is also used:

| Opcode (8 bits) | Operand Size | Total Size | Example Instruction |
|:----------------|:------------:|:----------:|:-------------------:|
| OP_ADD          | 0            | 8 bits     | OP_ADD              |

The complete bytecode for `1.2 + 3` becomes:

```
OP_CONSTANT 0     # Load the constant at location 0 (1.2)
OP_CONSTANT 1     # Load the constant at location 1 (3)
OP_ADD            # Add the two top-most constants
```

Let's extend the arithmetic example to understand the order of the bytecode even better, using the following expression:

```
-(1.2 + 3 * 4)
```

The opcodes that will be used for the resulting bytecode are:

| Opcode (8 bits) | Operand Size | Total Size | Example Instruction |
|:----------------|:------------:|:----------:|:-------------------:|
| OP_CONSTANT     | 8 bits       | 16 bits    | OP_CONSTANT 3       |
| OP_ADD          | 0            | 8 bits     | OP_ADD              |
| OP_MULTIPLY     | 0            | 8 bits     | OP_MULTIPLY         |
| OP_NEGATE       | 0            | 8 bits     | OP_NEGATE           |

From the mathematical rules of the order of operations, which Thusly follows, the result of `3 * 4` should be added to `1.2`, the sum of which should then be negated. (To understand how the expression is parsed and compiled in the correct order, see [parser-compiler.md](./parser-compiler.md)).

The complete bytecode for `-(1.2 + 3 * 4)` becomes:

```
OP_CONSTANT 0     # Load the constant at location 0 (1.2)
OP_CONSTANT 1     # Load the constant at location 1 (3)
OP_CONSTANT 2     # Load the constant at location 2 (4)
OP_MULTIPLY       # Multiply the two top-most constants
OP_ADD            # Add the two top-most constants
OP_NEGATE         # Negate the top-most constant
```

**Walk-Through:**

1. After the first three `OP_CONSTANT` instructions have been executed by the VM, the stack contains each value:

    ```
    Stack top

    4
    3
    1.2

    Stack bottom
    ```

1. When `OP_MULTIPLY` is executed, `4` and `3` are popped, and the result of `3 * 4` is pushed onto the stack:

    ```
    Stack top

    12
    1.2

    Stack bottom
    ```

1. When `OP_ADD` is executed, `12` and `1.2` are popped, and the result of `1.2 + 12` is pushed onto the stack.

    ```
    Stack top

    13.2

    Stack bottom
    ```

1. When `OP_NEGATE` is executed, `13.2` is popped, and the result of `-13.2` is pushed onto the stack.

    ```
    Stack top

    -13.2

    Stack bottom
    ```

Remember, you can always write some Thusly code in the [interactive playground](https://thusly.netlify.app/) and see the bytecode it compiles to!
