# HW5 – Intermediate Code Generation

## Course: Compilation Theory  
## Assignment 5 – Intermediate Code Generation  


---

## 📄 Overview

In this assignment, you'll implement translation from FanC (a simplified C-like language) to the intermediate representation (IR) language LLVM IR. The goal is to generate correct and executable LLVM IR code, focusing on:

- Arithmetic and boolean expressions
- Variable management via the stack
- Control flow constructs (`if`, `while`, `break`, `continue`, `return`)
- Function calls and output

---

## 🛠️ Tools and Language

- **LLVM IR** – Intermediate language for code generation.
- **Flex & Bison** – For lexical and syntactic analysis.
- **Provided**: `Makefile`, example tests (`hw5-tests.zip`), and function implementations (`print_functions.llvm`).

---

## 🧠 Main Concepts

- **SSA (Single Static Assignment)**: Each register is assigned once.
- **Registers**: Infinite supply, use `freshVar()` to generate new ones.
- **Stack**: Allocate local variables using `alloca`. Store and load values with `store` and `load`.

---

## ✅ Key LLVM Instructions

| Instruction | Description |
|-------------|-------------|
| `load`, `store` | Memory operations |
| `add`, `sub`, `mul`, `sdiv`, `udiv` | Arithmetic |
| `icmp` | Comparison |
| `br` | Conditional/unconditional jump |
| `phi` | SSA control flow merge |
| `call`, `ret`, `alloca`, `getelementptr` | Function operations |

---

## ⚙️ Project Structure

Required files to submit in a zip named `ID1-ID2.zip`:

- `scanner.lex` – Lexical rules
- `parser.y` – Syntax rules
- Any other required source files
- `Makefile` – **Use the provided one without modifications**
- **Do NOT** include binaries, test files, or the provided Makefile itself

---

## 📌 Compilation & Testing

You must be able to compile and run your code with the following commands:

```bash
unzip ID1-ID2.zip
cp path-to/Makefile .
cp path-to/hw5-tests.zip .
unzip hw5-tests.zip
make
./hw5 < t1.in 2>&1 > t1.ll
lli t1.ll > t1.res
diff t1.res path-to/t1.out
```

A return value of `0` from `diff` means your output matches the expected output.

Use the **`selfcheck`** script on the course site to validate your submission.

---

## 🚨 Runtime Requirements

- **Division by zero**: Must print `"Error division by zero"` and terminate.
- **Short-circuit evaluation**: Required for boolean expressions (`and`, `or`)
- **Uninitialized variables**: Default values — `0` for integers, `false` for booleans.
- **Numeric overflow**: Must remain within valid ranges (`int`: 0x0 to 0xffffffff; `byte`: 0–255)

---

## 🧪 Recommended Workflow

1. Arithmetic expressions  
2. Boolean expressions  
3. Stack variable access  
4. Statement sequences  
5. Control structures (`if`, `while`, etc.)  
6. Print functions (`print`, `printi`)  
7. Function calls

Create a **template LLVM file** to test code snippets independently.

---

## 🧾 Notes

- Use `CodeBuffer` to emit code (see `output.hpp`)
- `printf` and `exit` can be used (declare them in the code)
- Don't use `return` without ensuring the function returns a value

---


---

Good luck! ☺
