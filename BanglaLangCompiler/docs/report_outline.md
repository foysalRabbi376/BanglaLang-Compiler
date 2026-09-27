# Final Report Outline

## 1. The Pitch
### 1.1 Language name
**BanglaLang** — a beginner-friendly toy programming language that uses Bangla keywords while keeping familiar programming structures.

### 1.2 Problem addressed
Students who are new to programming may understand problem statements more comfortably when the visible language keywords are in Bangla. BanglaLang provides a small bridge between natural-language understanding and formal programming syntax.

### 1.3 Real-world relevance
The current compiler is intentionally a toy language. The practical direction is educational tooling: Bangla-first examples, classroom exercises, and a possible future learning environment.

### 1.4 Long-term vision
Future versions could add functions, strings, boolean literals, arrays, input, modules, better Unicode identifiers, and an editor with syntax highlighting.

## 2. Compiler Design
Pipeline:

**Source (.bng) -> Lexer -> Tokens -> Recursive-Descent Parser -> AST -> Semantic Analyzer -> Python Code Generator -> .py**

### Major classes
- `Lexer`: converts source text into tokens and reports lexical errors.
- `Parser`: validates syntax and builds the AST using precedence-based recursive descent.
- `SemanticAnalyzer`: maintains the symbol table and performs declaration/type checks.
- `CodeGenerator`: walks the AST and writes Python 3 code.
- `Token`: stores token kind, lexeme, line and column.
- AST nodes: `Expr`, `LiteralExpr`, `VariableExpr`, `UnaryExpr`, `BinaryExpr`, `Stmt`, `VarDeclStmt`, `AssignStmt`, `PrintStmt`, `IfStmt`, `WhileStmt`.

### Error handling
The parser synchronizes at the next semicolon or closing brace, allowing the compiler to continue collecting later errors instead of stopping at the first syntax issue. Target code is generated only when lexical, syntax and semantic errors are clear.

## 3. Language Grammar
See `docs/grammar.bnf`.

## 4. Testing Plan
- Correct arithmetic precedence.
- Integer and decimal declarations.
- Assignment.
- IF/ELSE comparison.
- WHILE loop.
- Undeclared variable.
- Invalid type assignment.
- Missing semicolon.
- Unexpected token.
- Missing brace.

## 5. Limitations
This is a toy compiler. It does not currently implement functions, strings, input, arrays, or a full Unicode identifier specification. It intentionally focuses on the minimum acceptable feature set and a clear compiler pipeline.
