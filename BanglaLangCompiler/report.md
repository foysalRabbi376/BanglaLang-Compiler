# BanglaLang Compiler
### CSE-4114 — Compiler Design and Construction Sessional

## 1. The Pitch

### 1.1 Project idea
BanglaLang is a small programming language designed with Bangla keywords and familiar programming structures. The language is intentionally small so that students can understand the complete compiler pipeline from source code to executable target code.

The source files use the `.bng` extension. A BanglaLang program starts with `শুরু {` and ends with `} শেষ`.

### 1.2 Problem the language addresses
Most beginner programming environments expose the control words of the language in English. BanglaLang explores a simple educational alternative: the visible keywords can be written in Bangla while the underlying compiler concepts remain standard compiler-design concepts such as lexical analysis, parsing, semantic analysis, AST construction and code generation.

### 1.3 Real-world relevance
The current version is a toy compiler, but the project can be used as a learning-oriented foundation for Bangla-first programming examples and classroom exercises. A future environment could combine the language with an editor, syntax highlighting and beginner-friendly diagnostics.

### 1.4 Long-term vision
A later version could add functions, input/output, strings, boolean literals, arrays, modules, a proper Unicode identifier specification, a richer standard library and an integrated editor.

## 2. Language Overview

### 2.1 Keywords

| Bangla keyword | Meaning |
|---|---|
| `শুরু` | program start |
| `শেষ` | program end |
| `সংখ্যা` | integer type |
| `দশমিক` | decimal type |
| `যদি` | IF |
| `নাহলে` | ELSE |
| `যতক্ষণ` | WHILE |
| `দেখাও` | print |

### 2.2 Data types

`সংখ্যা` stores integer values. `দশমিক` stores decimal values. The semantic analyzer checks declarations and assignments; assigning a `সংখ্যা` value to a `দশমিক` variable is allowed as a widening conversion, while assigning a decimal value to an integer variable is rejected.

### 2.3 Example program

```text
শুরু {
    সংখ্যা x = 10;
    সংখ্যা y = 5;
    দশমিক average = (x + y) / 2;
    দেখাও(average);

    যদি (x > y) {
        দেখাও(x);
    } নাহলে {
        দেখাও(y);
    }

    যতক্ষণ (x > 6) {
        x = x - 1;
        দেখাও(x);
    }
}
শেষ
```

Expected output:

```text
7.5
10
9
8
7
6
```

## 3. Compiler Design

### 3.1 Architecture

```text
Bangla source (.bng)
        |
        v
     Lexer
        |
        v
     Tokens
        |
        v
  Recursive-Descent Parser
        |
        v
       AST
        |
        v
 Semantic Analyzer
        |
        v
 Validated AST
        |
        v
 Code Generator
        |
        v
 Python 3 target (.py)
```

### 3.2 Lexical analysis
The `Lexer` scans the source string from left to right. It recognizes Bangla keywords, identifiers, integer literals, decimal literals, operators and punctuation. It records line and column information and collects invalid-token errors instead of terminating the process unexpectedly.

### 3.3 Syntax analysis
The `Parser` is a recursive-descent parser. Expression functions are split by precedence level: equality, comparison, addition/subtraction, multiplication/division/modulo, unary operators and primary expressions. This structure makes the required arithmetic precedence explicit.

### 3.4 Abstract Syntax Tree
The AST contains expression nodes (`LiteralExpr`, `VariableExpr`, `UnaryExpr`, `BinaryExpr`) and statement nodes (`VarDeclStmt`, `AssignStmt`, `PrintStmt`, `IfStmt`, `WhileStmt`). The parser builds the AST while checking the source grammar.

### 3.5 Semantic analysis
The `SemanticAnalyzer` maintains a symbol table mapping variable names to their types. It checks duplicate declarations, use of undeclared variables, assignment compatibility and operand types. It also validates that IF and WHILE conditions are comparison expressions.

### 3.6 Code generation
The `CodeGenerator` walks the validated AST and emits ordinary Python 3 code. BanglaLang `দেখাও` becomes `print(...)`, IF/ELSE becomes Python `if`/`else`, and `যতক্ষণ` becomes Python `while`.

### 3.7 Error recovery
For syntax errors, the parser moves forward until a semicolon, closing brace or end-of-file. This allows it to continue looking for later syntax problems rather than crashing immediately at the first malformed statement. Target code is emitted only when lexical, syntax and semantic checks succeed.

## 4. Major Classes — UML

The UML source is available in `docs/uml.puml` and covers the compiler's major classes and their relationships.

## 5. Formal Grammar

The complete BNF is provided in `docs/grammar.bnf`. The core structure is:

```text
<program> ::= "শুরু" "{" <statement-list> "}" "শেষ"
<statement-list> ::= <statement> <statement-list> | ε
<statement> ::= <declaration> ";"
              | <assignment> ";"
              | <print-statement> ";"
              | <if-statement>
              | <while-statement>
```

The grammar document contains the complete expression, condition, type and statement productions.

## 6. Requirement Coverage

| Requirement | Implementation |
|---|---|
| Two data types with type checking | `সংখ্যা`, `দশমিক` + semantic analysis |
| Basic arithmetic with precedence | `+ - * / %`, unary minus, precedence-based parser |
| Assignment | `identifier = expression;` |
| IF-ELSE | `যদি (...) { ... } নাহলে { ... }` |
| WHILE | `যতক্ষণ (...) { ... }` |
| Basic syntax error recovery | Synchronization at `;` / `}` |
| No compiler runtime crashes on malformed input | Errors are collected and reported with exit codes |
| Executable target code | Python 3 file generation |
| Compiler implementation language | C++17 |

## 7. Testing

### Test A — valid program
`examples/hello.bng`

Result: compiles successfully and the generated `hello.py` executes with the expected output.

### Test B — type error
`examples/type_error.bng`

Result: compilation stops before code generation and reports the invalid decimal-to-integer assignment.

### Test C — syntax error
`examples/syntax_error.bng`

Result: the missing semicolon is reported; the parser identifies the next declaration as an unexpected continuation instead of terminating the process with a crash.

### Test D — undeclared variable
A separate test using `y` before declaration produces a semantic error.

## 8. How to Build and Run

### Linux / macOS
```bash
g++ -std=c++17 -O2 -Wall -Wextra -o banglacc src/main.cpp
./banglacc examples/hello.bng -o examples/hello.py
python3 examples/hello.py
```

### Windows MinGW
```bat
g++ -std=c++17 -O2 -Wall -Wextra -o banglacc.exe src\main.cpp
banglacc.exe examples\hello.bng -o examples\hello.py
python examples\hello.py
```

## 9. Limitations of the Current Toy Version

The compiler does not currently implement functions, strings, arrays, user input or a complete language-level boolean type. The project intentionally focuses on a small, teachable feature set that matches the core sessional requirements.

## 10. Future Roadmap

1. Add `সত্য` and `মিথ্যা` boolean literals.
2. Add strings and string concatenation.
3. Add user input.
4. Add functions and return statements.
5. Add arrays and indexing.
6. Add a richer standard library.
7. Add an editor with syntax highlighting and compiler diagnostics.
8. Consider WebAssembly as a future target if the project scope is expanded.
