# BanglaLang Compiler (CSE-4114)

A small, team-designed Bangla programming language compiler implemented in C++17. The compiler translates `.bng` source code into executable Python 3 code.

## Why this design
- Compiler implementation language: C++17 (allowed by the course requirement).
- Source language uses custom Bangla keywords.
- Two types: `সংখ্যা` (integer) and `দশমিক` (decimal).
- Arithmetic precedence: unary -> `* / %` -> `+ -` -> comparison -> equality.
- Statements: declaration, assignment, `দেখাও`, `যদি`/`নাহলে`, `যতক্ষণ`.
- Syntax errors are collected and the parser synchronizes at `;` or `}`.
- Semantic errors are reported before target code generation.
- Generated target is normal Python 3 code.

## Build

### Linux / macOS
```bash
g++ -std=c++17 -O2 -Wall -Wextra -o banglacc src/main.cpp
```

### Windows (MinGW g++)
```bat
g++ -std=c++17 -O2 -Wall -Wextra -o banglacc.exe src\main.cpp
```

Visual Studio users can create a C++17 Console project and add `src/main.cpp`.

## Compile a BanglaLang program
```bash
./banglacc examples/hello.bng -o hello.py
python3 hello.py
```
Windows:
```bat
banglacc.exe examples\hello.bng -o hello.py
python hello.py
```

## Example language
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

## Team demo order
1. Show the `.bng` source.
2. Explain lexer -> parser -> semantic analysis -> code generation.
3. Compile it with `banglacc`.
4. Open the generated Python file.
5. Run the Python target.
6. Run `type_error.bng` to show type checking.
7. Run `syntax_error.bng` to show error recovery/reporting.
