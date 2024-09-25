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

> **TODO:** ADD EXAMPLE
