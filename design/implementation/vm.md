# Thusly Design - Virtual Machine (VM)

> 👉️ These design documents are currently being added to incrementally.

## Table of Contents

> **TODO:** INSERT TABLE OF CONTENTS

## Abstract

The virtual machine, or VM, evaluates expressions and executes statements by decoding and executing the instructions in the compiled program in a sequential order, unless the instruction itself is to jump to a different section of the program.

It specifies the [instruction format](./bytecode.md) of the bytecode which the compiler must adhere to when writing the instructions, which is based on the stack-based architecture of the VM.

## Decoding an Instruction

In order to know which type of operation to perform, each bytecode instruction reserves the first 8 bits to the *opcode*, which could be any of the operations supported by the VM.

Some instructions allow operands, such as loading a constant (e.g. `OP_CONSTANT 0`) or jumping over bytes in the bytecode (e.g. `OP_JUMP_FWD 10`), but most instructions only contain the opcode itself, such as multiplying (`OP_MULTIPLY`). Hence, an instruction can vary in size, from 8 bits (1 byte) to 24 bits (3 bytes), and depending on the opcode, the remaining bits are interpreted differently. Figuring out how the bits in the current instruction should be interpreted is what the VM does when it *decodes* the instruction.

To better understand the bytecode format, it is recommended to read the [bytecode design documentation](./bytecode.md) first, but here are some examples of the opcodes supported, the size of their operands if any, and example instructions:

| Opcode (8 bits) | Operand Size | Total Size | Example Instruction |
|:----------------|:------------:|:----------:|:-------------------:|
| OP_CONSTANT     | 8 bits       | 16 bits    | OP_CONSTANT 3       |
| OP_ADD          | 0            | 8 bits     | OP_ADD              |
| OP_SUBTRACT     | 0            | 8 bits     | OP_SUBTRACT         |
| OP_MULTIPLY     | 0            | 8 bits     | OP_MULTIPLY         |
| OP_NEGATE       | 0            | 8 bits     | OP_NEGATE           |
| OP_JUMP_FWD     | 16 bits      | 24 bits    | OP_JUMP_FWD 10      |
| OP_JUMP_BWD     | 16 bits      | 24 bits    | OP_JUMP_BWD 13      |
| OP_POP          | 0            | 8 bits     | OP_POP              |

### Program Counter

The VM stores a byte (`uint8_t`) pointer to the next instruction to be executed, sometimes referred to as a *program counter (PC)* or *instruction pointer (IP)*. Thus, when decoding the instruction, that pointer is dereferenced to get the first byte of the instruction, i.e. the opcode.

Once the opcode has been retrieved, the VM may keep reading the next byte, or bytes, depending on the corresponding operand size expected for that type of instruction, before starting to execute the appropriate logic.

As bytes are read, the program counter is incremented to finally point to the start of the next instruction once the current instruction has been decoded.

## Operand Stack

The VM uses a last-in-first-out (LIFO) stack for the operands of the operators used, as well as for the result of an evaluated expression. Thus, it operates only on values on the stack, which are always the `ThuslyValue` representation.

For instance, as seen in the above table in the [Decoding an Instruction section](#decoding-an-instruction), the instruction for addition is only `OP_ADD` *without* any operands. This is because the operands of an add expression, such as `1.2 + 3`, are expected to already be on the stack from earlier bytecode instructions. Specifically, `3` should be at the top, and `1.2` immediately below.

An `OP_ADD` instruction will then pop the top (`3`, which was last in and now first out) then pop the top again (`1.2`). These operands are then used to perform the operation `1.2 + 3` on the underlying C values, whereafter the result `4.2` is pushed onto the stack as a `ThuslyValue`.

The bytecode below for `1.2 + 3` shows the instructions for first loading (pushing onto the stack) the constants `1.2` and `3` from a [constant pool](./parser-compiler.md#constant-pool), then performing the addition, and finally discarding the result of the evaluated expression by popping it off the stack.

```
OP_CONSTANT 0     # Load the constant from location 0 (1.2)
OP_CONSTANT 1     # Load the constant from location 1 (3)
OP_ADD            # Add the two top-most values
OP_POP            # Discard the result
```

The stack in the Thusly VM is implemented using an array. To keep track of the top of the stack, the VM stores a `ThuslyValue` pointer pointing to the slot immediately following the top of the stack. (Due to C internals, it does not point to the current top since an empty array would cause it to point immediately *before* the array which is undefined behavior, unlike pointing immediately *after*.)

Once the two `OP_CONSTANT` instructions have been executed, the stack contains the two values (`sp` below signifies the stack pointer):


|        | Index 0 | Index 1 (top) | Index 2 |
|:-------|:-------:|:-------------:|:-------:|
| Value  | 1.2     | 3             | -       |
|        |         |               | sp      |

Then, `OP_ADD` pops the values and pushes the result onto the stack:

|        | Index 0 (top) | Index 1 |
|:-------|:-------------:|:-------:|
| Value  | 4.2           | -       |
|        |               | sp      |

Lastly, `OP_POP` discards the result by popping the top of the stack.

|        | Index 0 |
|:-------|:-------:|
| Value  | -       |
|        | sp      |

More elaborate examples can be seen in [Walk-Through of Examples](#walk-through-of-examples).

## Walk-Through of Examples

These examples demonstrate some of the details of the VM's steps when executing bytecode instructions.

> [!TIP]
> To more easily follow along, you can first read the overview of the [bytecode format](./bytecode.md).
>
> You can also try out the web-based [interactive playground](https://thusly.netlify.app/) to write some Thusly code and see both the bytecode it compiles to and the VM's operand stack as it is executing!

* Examples:
  * [Arithmetic](#example-arithmetic)

### Example: Arithmetic

**Source code:**

```
1.2 + 3 * 4 / -5
```

**Bytecode generated by the compiler:**

```
OP_CONSTANT 0
OP_CONSTANT 1
OP_CONSTANT 2
OP_MULTIPLY
OP_CONSTANT 3
OP_NEGATE
OP_DIVIDE
OP_ADD
OP_POP
```

**Constant pool built by the compiler:**

|        | Index 0 | Index 1 | Index 2 | Index 3 |
|:-------|:-------:|:-------:|:-------:|:-------:|
| Value  | 1.2     | 3       | 4       | 5       |

**VM execution process:**

**Instruction #1** (`OP_CONSTANT 0`):

```
          Byte Offset    Instruction

next ->   0              OP_CONSTANT 0
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_CONSTANT`, the next byte pointed to (`0`) is read and treated as a [constant pool](./parser-compiler.md#constant-pool) index.
1. `next` is advanced to the next *byte*.
1. The `ThuslyValue` (`1.2`) is retrieved from the constant pool at index `0` and pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0 (top) | Index 1       |
    |:-------|:-------------:|:-------------:|
    | Value  | 1.2           | -             |
    |        |               | sp            |

**Instruction #2** (`OP_CONSTANT 1`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
next ->   2              OP_CONSTANT 1
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_CONSTANT`, the next byte pointed to (`1`) is read and treated as a constant pool index.
1. `next` is advanced to the next *byte*.
1. The `ThuslyValue` (`3`) is retrieved from the constant pool at index `1` and pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0       | Index 1 (top) | Index 2       |
    |:-------|:-------------:|:-------------:|:-------------:|
    | Value  | 1.2           | 3             | -             |
    |        |               |               | sp            |

**Instruction #3** (`OP_CONSTANT 2`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
          2              OP_CONSTANT 1
next ->   4              OP_CONSTANT 2
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_CONSTANT`, the next byte pointed to (`2`) is read and treated as a constant pool index.
1. `next` is advanced to the next *byte*.
1. The `ThuslyValue` (`4`) is retrieved from the constant pool at index `2` and pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0       | Index 1       | Index 2 (top) | Index 3       |
    |:-------|:-------------:|:-------------:|:-------------:|:-------------:|
    | Value  | 1.2           | 3             | 4             | -             |
    |        |               |               |               | sp            |

**Instruction #4** (`OP_MULTIPLY`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
          2              OP_CONSTANT 1
          4              OP_CONSTANT 2
next ->   6              OP_MULTIPLY
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_MULTIPLY`, the two top-most values on the stack are popped, first `4` then `3`.
1. For each pop, the stack pointer (`sp`) is moved back to the previous stack slot.
1. The result of the evaluation `3 * 4` is pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0       | Index 1 (top) | Index 2       |
    |:-------|:-------------:|:-------------:|:-------------:|
    | Value  | 1.2           | 12            | -             |
    |        |               |               | sp            |

**Instruction #5** (`OP_CONSTANT 3`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
          2              OP_CONSTANT 1
          4              OP_CONSTANT 2
          6              OP_MULTIPLY
next ->   7              OP_CONSTANT 3
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_CONSTANT`, the next byte pointed to (`3`) is read and treated as a constant pool index.
1. `next` is advanced to the next *byte*.
1. The `ThuslyValue` (`5`) is retrieved from the constant pool at index `3` and pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0       | Index 1       | Index 2 (top) | Index 3       |
    |:-------|:-------------:|:-------------:|:-------------:|:-------------:|
    | Value  | 1.2           | 12            | 5             | -             |
    |        |               |               |               | sp            |

**Instruction #6** (`OP_NEGATE`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
          2              OP_CONSTANT 1
          4              OP_CONSTANT 2
          6              OP_MULTIPLY
          7              OP_CONSTANT 3
next ->   9              OP_NEGATE
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_NEGATE`, the top-most value on the stack (`5`) is popped.
1. The stack pointer (`sp`) is moved back to the previous stack slot.
1. The result of the evaluation `-5` is pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0       | Index 1       | Index 2 (top) | Index 3       |
    |:-------|:-------------:|:-------------:|:-------------:|:-------------:|
    | Value  | 1.2           | 12            | -5            | -             |
    |        |               |               |               | sp            |

**Instruction #7** (`OP_DIVIDE`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
          2              OP_CONSTANT 1
          4              OP_CONSTANT 2
          6              OP_MULTIPLY
          7              OP_CONSTANT 3
          9              OP_NEGATE
next ->   10             OP_DIVIDE
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_DIVIDE`, the two top-most values on the stack are popped, first `-5` then `12`.
1. For each pop, the stack pointer (`sp`) is moved back to the previous stack slot.
1. The result of the evaluation `12 / -5` is pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0       | Index 1 (top) | Index 2       |
    |:-------|:-------------:|:-------------:|:-------------:|
    | Value  | 1.2           | -2.4          | -             |
    |        |               |               | sp            |

**Instruction #8** (`OP_ADD`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
          2              OP_CONSTANT 1
          4              OP_CONSTANT 2
          6              OP_MULTIPLY
          7              OP_CONSTANT 3
          9              OP_NEGATE
          10             OP_DIVIDE
next ->   11             OP_ADD
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_ADD`, the two top-most values on the stack are popped, first `-2.4` then `1.2`.
1. For each pop, the stack pointer (`sp`) is moved back to the previous stack slot.
1. The result of the evaluation `1.2 + -2.4` is pushed onto the stack.
1. The stack pointer (`sp`) is advanced to the next stack slot.

* **Current state:**
  * Stack:

    |        | Index 0 (top) | Index 1       |
    |:-------|:-------------:|:-------------:|
    | Value  | -1.2          | -             |
    |        |               | sp            |

**Instruction #9** (`OP_POP`):

```
          Byte Offset    Instruction

          0              OP_CONSTANT 0
          2              OP_CONSTANT 1
          4              OP_CONSTANT 2
          6              OP_MULTIPLY
          7              OP_CONSTANT 3
          9              OP_NEGATE
          10             OP_DIVIDE
          11             OP_ADD
next ->   12             OP_POP
```

1. The instruction is decoded by reading the first byte (the opcode) pointed to by the program counter (`next`).
1. `next` is advanced to the next *byte*.
1. Since the opcode is `OP_POP`, the top-most value on the stack (`-1.2`) is popped.
1. The stack pointer (`sp`) is moved back to the previous stack slot.

* **Current state:**
  * Stack:

    |        | Index 0       |
    |:-------|:-------------:|
    | Value  | -             |
    |        | sp            |
