In our BanglaLang Compiler project, my main responsibility was the source code handling, lexical analysis (Lexer), and syntax analysis (Parser) parts.

The source code handling part reads the BanglaLang source program from an input file and passes it to the compiler pipeline.

The Lexer converts the source code into a sequence of tokens.

The Parser takes those tokens and checks whether the program follows the grammar of our language. It also builds the required Abstract Syntax Tree (AST) structures.

My contribution can be summarized as:

1. Source Code Handling
2. Lexical Analysis (Lexer)
3. Syntax Analysis (Parser)
4. AST structures required by the Parser


## Overall Compiler Flow


Bangla Source Code
        |
        v
 Source File Handling
        |
        v
      Lexer
        |
        v
      Tokens
        |
        v
      Parser
        |
        v
       AST
        |
        v
 Semantic Analyzer
        |
        v
  Code Generator
        |
        v
   Python Code

The Lexer and Parser are important early stages of the compiler.

The Lexer identifies the individual elements of the source program, while the Parser checks how those elements are arranged according to the language grammar.


1. Source Code Handling

What I implemented

I handled the source program input part of the compiler.

The compiler receives a source file path as input. The source file is opened and its complete contents are read into a string.

The source code is then passed to the Lexer.

The basic flow is:

Source File
     |
     v
  readFile()
     |
     v
Source Code String
     |
     v
   Lexer
Source Code Handling Code
static string readFile(const string& path) {
    ifstream f(path, ios::binary);

    if(!f)
        throw runtime_error("Cannot open source file: " + path);

    stringstream ss;
    ss << f.rdbuf();

    return ss.str();
}
How it works

The readFile() function receives the source file path.

First, an ifstream object opens the file.

ifstream f(path, ios::binary);

If the file cannot be opened, an error is generated:

if(!f)
    throw runtime_error("Cannot open source file: " + path);

Then the complete file content is read using stringstream:

stringstream ss;
ss << f.rdbuf();

Finally, the source program is returned as a string:

return ss.str();

This source string becomes the input of the Lexer.

2. Lexical Analysis — Lexer

What is Lexical Analysis?

Lexical analysis is the first major stage of the compiler.

The Lexer reads the source program character by character and converts it into meaningful tokens.

For example:

সংখ্যা x = 10;

can be divided into tokens such as:

সংখ্যা    -> Keyword
x         -> Identifier
=         -> Assignment Operator
10        -> Integer Literal
;         -> Semicolon

The Parser does not directly work with the raw source characters. It works with these tokens.

Main responsibilities of my Lexer

My Lexer handles:

Keywords
Identifiers
Integer literals
Decimal literals
Arithmetic operators
Assignment operator
Comparison operators
Parentheses
Braces
Commas
Semicolons
Whitespace
Single-line comments
Line and column tracking
Invalid characters
End-of-file token

3. Token Structure

The Lexer uses a Token structure.

struct Token {
    enum class Kind {
        Identifier,
        IntLiteral,
        DoubleLiteral,

        Plus,
        Minus,
        Star,
        Slash,
        Percent,

        Equal,
        EqualEqual,
        BangEqual,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,

        LeftParen,
        RightParen,
        LeftBrace,
        RightBrace,
        Comma,
        Semicolon,

        KwNumber,
        KwDecimal,
        KwIf,
        KwElse,
        KwWhile,
        KwPrint,
        KwStart,
        KwEnd,

        EndOfFile,
        Invalid
    };

    Kind kind;
    string lexeme;
    int line;
    int column;

    Token(Kind k, string l, int ln, int col)
        : kind(k),
          lexeme(std::move(l)),
          line(ln),
          column(col) {}
};

Each token contains:

kind — type of the token
lexeme — actual text
line — line number
column — column number

4. BanglaLang Keywords

The Lexer contains a keyword table.

unordered_map<string, Token::Kind> keywords = {
    {"সংখ্যা", Token::Kind::KwNumber},
    {"দশমিক", Token::Kind::KwDecimal},
    {"যদি", Token::Kind::KwIf},
    {"নাহলে", Token::Kind::KwElse},
    {"যতক্ষণ", Token::Kind::KwWhile},
    {"দেখাও", Token::Kind::KwPrint},
    {"শুরু", Token::Kind::KwStart},
    {"শেষ", Token::Kind::KwEnd}
};

This allows the Lexer to distinguish a keyword from a normal identifier.

For example:

সংখ্যা

is recognized as:

KwNumber

while:

x

is recognized as:

Identifier
        
5. Lexer Class

The main Lexer class is responsible for scanning the complete source code.

class Lexer {
    string src;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    unordered_map<string, Token::Kind> keywords = {
        {"সংখ্যা", Token::Kind::KwNumber},
        {"দশমিক", Token::Kind::KwDecimal},
        {"যদি", Token::Kind::KwIf},
        {"নাহলে", Token::Kind::KwElse},
        {"যতক্ষণ", Token::Kind::KwWhile},
        {"দেখাও", Token::Kind::KwPrint},
        {"শুরু", Token::Kind::KwStart},
        {"শেষ", Token::Kind::KwEnd}
    };

    char peekChar(size_t offset = 0) const {
        if (pos + offset >= src.size())
            return '\0';

        return src[pos + offset];
    }

    char advance() {
        if (pos >= src.size())
            return '\0';

        char c = src[pos++];

        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }

        return c;
    }

The peekChar() function looks at the current character without consuming it.

The advance() function consumes one character and updates the line and column position.

6. Handling Whitespace and Comments

Whitespace and comments should not normally become meaningful tokens.

Therefore, the Lexer skips them.

void skipWhitespaceAndComments() {
    while (true) {

        while (isspace(
            static_cast<unsigned char>(peekChar()))) {

            advance();
        }

        if (peekChar() == '/' &&
            peekChar(1) == '/') {

            while (peekChar() != '\n' &&
                   peekChar() != '\0') {

                advance();
            }

            continue;
        }

        break;
    }
}

For example:

// this is a comment
সংখ্যা x = 10;

The comment is ignored and only the actual program is tokenized.

7. Recognizing Numbers

The Lexer supports both integer and decimal literals.

Token number() {
    int startLine = line;
    int startColumn = column;

    string value;
    bool hasDot = false;

    while (isdigit(
        static_cast<unsigned char>(peekChar()))) {

        value += advance();
    }

    if (peekChar() == '.' &&
        isdigit(
            static_cast<unsigned char>(peekChar(1)))) {

        hasDot = true;
        value += advance();

        while (isdigit(
            static_cast<unsigned char>(peekChar()))) {

            value += advance();
        }
    }

    if (hasDot) {
        return makeToken(
            Token::Kind::DoubleLiteral,
            value,
            startLine,
            startColumn
        );
    }

    return makeToken(
        Token::Kind::IntLiteral,
        value,
        startLine,
        startColumn
    );
}

For example:

10

becomes:

IntLiteral

and:

10.5

becomes:

DoubleLiteral
        
8. Recognizing Identifiers and Keywords
        
Token identifier() {
    int startLine = line;
    int startColumn = column;

    string value;

    while (peekChar() != '\0') {

        unsigned char c =
            static_cast<unsigned char>(peekChar());

        if (isalnum(c) || peekChar() == '_') {
            value += advance();
        } else {
            break;
        }
    }

    auto it = keywords.find(value);

    if (it != keywords.end()) {
        return makeToken(
            it->second,
            value,
            startLine,
            startColumn
        );
    }

    return makeToken(
        Token::Kind::Identifier,
        value,
        startLine,
        startColumn
    );
}

The Lexer first collects the complete word.

Then it checks whether the word exists in the keyword table.

If it exists, it becomes a keyword token.

Otherwise, it becomes an identifier.

9. Operators and Punctuation

The Lexer recognizes operators such as:

+  -  *  /  %
=  ==  !=
<  <=
>  >=

It also recognizes:

(  )
{  }
,  ;

For example:

case '=':
    advance();

    if (peekChar() == '=') {
        advance();

        tokens.push_back(
            makeToken(
                Token::Kind::EqualEqual,
                "==",
                startLine,
                startColumn
            )
        );
    } else {
        tokens.push_back(
            makeToken(
                Token::Kind::Equal,
                "=",
                startLine,
                startColumn
            )
        );
    }

    break;

This allows the Lexer to distinguish:

=

from:

==
10. Complete Tokenization Process

The main tokenize() function repeatedly scans the source code.

vector<Token> tokenize() {

    vector<Token> tokens;

    while (true) {

        skipWhitespaceAndComments();

        int startLine = line;
        int startColumn = column;

        char c = peekChar();

        if (c == '\0') {

            tokens.emplace_back(
                Token::Kind::EndOfFile,
                "",
                line,
                column
            );

            break;
        }

        if (isdigit(
            static_cast<unsigned char>(c))) {

            tokens.push_back(number());
            continue;
        }

        if (isalnum(
            static_cast<unsigned char>(c)) ||
            c == '_') {

            tokens.push_back(identifier());
            continue;
        }

        // Operators and punctuation are handled here.
        // ...

    }

    return tokens;
}

At the end, the Lexer adds an EndOfFile token.

11. Example of Lexical Analysis

Suppose the source program is:

শুরু

সংখ্যা x = 10;
দশমিক y = 2.5;

দেখাও x + y;

শেষ

The Lexer produces tokens conceptually like:

KwStart
KwNumber
Identifier(x)
Equal
IntLiteral(10)
Semicolon

KwDecimal
Identifier(y)
Equal
DoubleLiteral(2.5)
Semicolon

KwPrint
Identifier(x)
Plus
Identifier(y)
Semicolon

KwEnd
EOF

These tokens are then passed to the Parser.

12. Syntax Analysis — Parser

What is Syntax Analysis?

Syntax analysis is the stage after lexical analysis.

The Parser receives the tokens generated by the Lexer and checks whether they follow the grammar of BanglaLang.

For example:

সংখ্যা x = 10;

has a valid structure:

Declaration
   |
   +-- Type: সংখ্যা
   |
   +-- Identifier: x
   |
   +-- =
   |
   +-- Expression: 10

The Parser also creates AST nodes that represent the structure of the program.

13. AST Expression Structures

The Parser uses several expression structures.

struct Expr {
    virtual ~Expr() = default;
};

struct LiteralExpr : Expr {
    string value;

    explicit LiteralExpr(string v)
        : value(std::move(v)) {}
};

struct VariableExpr : Expr {
    string name;

    explicit VariableExpr(string n)
        : name(std::move(n)) {}
};

struct UnaryExpr : Expr {
    Token op;
    unique_ptr<Expr> right;

    UnaryExpr(
        Token operation,
        unique_ptr<Expr> expression
    )
        : op(std::move(operation)),
          right(std::move(expression)) {}
};

struct BinaryExpr : Expr {
    unique_ptr<Expr> left;
    Token op;
    unique_ptr<Expr> right;

    BinaryExpr(
        unique_ptr<Expr> l,
        Token operation,
        unique_ptr<Expr> r
    )
        : left(std::move(l)),
          op(std::move(operation)),
          right(std::move(r)) {}
};

These structures represent:

Literal values
Variables
Unary operations
Binary operations

14. AST Statement Structures

The Parser also creates statement nodes.

struct Stmt {
    virtual ~Stmt() = default;
};

struct VarDeclStmt : Stmt {
    string type;
    string name;
    unique_ptr<Expr> initializer;

    VarDeclStmt(
        string t,
        string n,
        unique_ptr<Expr> init
    )
        : type(std::move(t)),
          name(std::move(n)),
          initializer(std::move(init)) {}
};

struct AssignStmt : Stmt {
    string name;
    unique_ptr<Expr> value;

    AssignStmt(
        string n,
        unique_ptr<Expr> v
    )
        : name(std::move(n)),
          value(std::move(v)) {}
};

struct PrintStmt : Stmt {
    unique_ptr<Expr> expression;

    explicit PrintStmt(unique_ptr<Expr> e)
        : expression(std::move(e)) {}
};

struct IfStmt : Stmt {
    unique_ptr<Expr> condition;

    vector<unique_ptr<Stmt>> thenBranch;
    vector<unique_ptr<Stmt>> elseBranch;

    IfStmt(
        unique_ptr<Expr> c,
        vector<unique_ptr<Stmt>> t,
        vector<unique_ptr<Stmt>> e
    )
        : condition(std::move(c)),
          thenBranch(std::move(t)),
          elseBranch(std::move(e)) {}
};

struct WhileStmt : Stmt {
    unique_ptr<Expr> condition;
    vector<unique_ptr<Stmt>> body;

    WhileStmt(
        unique_ptr<Expr> c,
        vector<unique_ptr<Stmt>> b
    )
        : condition(std::move(c)),
          body(std::move(b)) {}
};

15. Parser Class

The Parser stores the generated tokens and keeps track of the current token.

class Parser {

    vector<Token> ts;
    size_t i = 0;

    const Token& peek(int n = 0) const {
        return ts[
            min(
                i + static_cast<size_t>(n),
                ts.size() - 1
            )
        ];
    }

    bool check(Token::Kind kind) const {
        return peek().kind == kind;
    }

    const Token& advance() {
        if (i < ts.size())
            i++;

        return ts[i - 1];
    }

    bool match(Token::Kind kind) {

        if (check(kind)) {
            advance();
            return true;
        }

        return false;
    }

The main helper functions are:

peek() — looks at a token
check() — checks token type
advance() — moves to the next token
match() — checks and consumes a token
consume() — requires a particular token and reports an error otherwise
16. Parsing the Program
vector<unique_ptr<Stmt>> parseProgram() {

    vector<unique_ptr<Stmt>> statements;

    if (match(Token::Kind::KwStart)) {

        while (!check(Token::Kind::KwEnd) &&
               !check(Token::Kind::EndOfFile)) {

            try {
                statements.push_back(
                    parseStatement()
                );
            }
            catch (const exception&) {
                synchronize();
            }
        }

        consume(
            Token::Kind::KwEnd,
            "Expected শেষ"
        );
    }

    while (!check(Token::Kind::EndOfFile)) {

        try {
            statements.push_back(
                parseStatement()
            );
        }
        catch (const exception&) {
            synchronize();
        }
    }

    return statements;
}

The Parser recognizes the BanglaLang program structure using:

শুরু
   |
   |---- statements
   |
শেষ

17. Parsing Statements

The Parser determines the statement type based on the current token.

unique_ptr<Stmt> parseStatement() {

    if (check(Token::Kind::KwNumber) ||
        check(Token::Kind::KwDecimal)) {

        return parseDecl();
    }

    if (check(Token::Kind::KwPrint)) {
        return parsePrint();
    }

    if (check(Token::Kind::KwIf)) {
        return parseIf();
    }

    if (check(Token::Kind::KwWhile)) {
        return parseWhile();
    }

    if (check(Token::Kind::Identifier)) {
        return parseAssign();
    }

    throw runtime_error(
        "Unexpected token: " +
        peek().lexeme
    );
}

So the Parser supports:

Declaration
Assignment
Print
If-else
While

18. Parsing Variable Declaration

For a declaration such as:

সংখ্যা x = 10;

the Parser uses:

unique_ptr<Stmt> parseDecl() {

    Token typeToken = advance();

    Token name = consume(
        Token::Kind::Identifier,
        "Expected identifier"
    );

    consume(
        Token::Kind::Equal,
        "Expected ="
    );

    auto initializer = parseExpression();

    match(Token::Kind::Semicolon);

    return make_unique<VarDeclStmt>(
        typeToken.lexeme,
        name.lexeme,
        std::move(initializer)
    );
}

The Parser therefore recognizes:

Type
  |
Identifier
  |
 =
  |
Expression
  |
 ;
19. Parsing Assignment

For:

x = 20;

the Parser uses:

unique_ptr<Stmt> parseAssign() {

    Token name = advance();

    consume(
        Token::Kind::Equal,
        "Expected ="
    );

    auto value = parseExpression();

    match(Token::Kind::Semicolon);

    return make_unique<AssignStmt>(
        name.lexeme,
        std::move(value)
    );
}

20. Parsing Print Statement

For:

দেখাও x;

the Parser uses:

unique_ptr<Stmt> parsePrint() {

    advance();

    auto expression = parseExpression();

    match(Token::Kind::Semicolon);

    return make_unique<PrintStmt>(
        std::move(expression)
    );
}

21. Parsing If-Else

The Parser also supports conditional statements.

unique_ptr<Stmt> parseIf() {

    advance();

    consume(
        Token::Kind::LeftParen,
        "Expected ("
    );

    auto condition = parseExpression();

    consume(
        Token::Kind::RightParen,
        "Expected )"
    );

    auto thenBranch = parseBlock();

    vector<unique_ptr<Stmt>> elseBranch;

    if (match(Token::Kind::KwElse)) {
        elseBranch = parseBlock();
    }

    return make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}

22. Parsing While Loop

The Parser supports:

যতক্ষণ (condition) {
    ...
}

using:

unique_ptr<Stmt> parseWhile() {

    advance();

    consume(
        Token::Kind::LeftParen,
        "Expected ("
    );

    auto condition = parseExpression();

    consume(
        Token::Kind::RightParen,
        "Expected )"
    );

    auto body = parseBlock();

    return make_unique<WhileStmt>(
        std::move(condition),
        std::move(body)
    );
}

23. Parsing Blocks

Blocks are enclosed by { and }.

vector<unique_ptr<Stmt>> parseBlock() {

    vector<unique_ptr<Stmt>> statements;

    consume(
        Token::Kind::LeftBrace,
        "Expected {"
    );

    while (!check(Token::Kind::RightBrace) &&
           !check(Token::Kind::EndOfFile)) {

        statements.push_back(
            parseStatement()
        );
    }

    consume(
        Token::Kind::RightBrace,
        "Expected }"
    );

    return statements;
}
24. Expression Parsing

The Parser uses multiple levels of functions to implement operator precedence.

The order is:

Expression
    |
Equality
    |
Comparison
    |
Term
    |
Factor
    |
Unary
    |
Primary

This allows expressions such as:

a + b * 5

to be interpreted correctly.

Multiplication is handled before addition.

25. Equality and Comparison
        
unique_ptr<Expr> parseExpression() {
    return parseEquality();
}

Equality:

unique_ptr<Expr> parseEquality() {

    auto expr = parseComparison();

    while (check(Token::Kind::EqualEqual) ||
           check(Token::Kind::BangEqual)) {

        Token op = advance();

        auto right = parseComparison();

        expr = make_unique<BinaryExpr>(
            std::move(expr),
            std::move(op),
            std::move(right)
        );
    }

    return expr;
}

Comparison:

unique_ptr<Expr> parseComparison() {

    auto expr = parseTerm();

    while (check(Token::Kind::Less) ||
           check(Token::Kind::LessEqual) ||
           check(Token::Kind::Greater) ||
           check(Token::Kind::GreaterEqual)) {

        Token op = advance();

        auto right = parseTerm();

        expr = make_unique<BinaryExpr>(
            std::move(expr),
            std::move(op),
            std::move(right)
        );
    }

    return expr;
}
        
26. Arithmetic Expression Parsing

Addition and subtraction:

unique_ptr<Expr> parseTerm() {

    auto expr = parseFactor();

    while (check(Token::Kind::Plus) ||
           check(Token::Kind::Minus)) {

        Token op = advance();

        auto right = parseFactor();

        expr = make_unique<BinaryExpr>(
            std::move(expr),
            std::move(op),
            std::move(right)
        );
    }

    return expr;
}

Multiplication, division and modulus:

unique_ptr<Expr> parseFactor() {

    auto expr = parseUnary();

    while (check(Token::Kind::Star) ||
           check(Token::Kind::Slash) ||
           check(Token::Kind::Percent)) {

        Token op = advance();

        auto right = parseUnary();

        expr = make_unique<BinaryExpr>(
            std::move(expr),
            std::move(op),
            std::move(right)
        );
    }

    return expr;
}
        
27. Unary Expression

The Parser supports unary minus.

unique_ptr<Expr> parseUnary() {

    if (check(Token::Kind::Minus)) {

        Token op = advance();

        auto right = parseUnary();

        return make_unique<UnaryExpr>(
            std::move(op),
            std::move(right)
        );
    }

    return parsePrimary();
}
        
28. Primary Expressions

The Parser recognizes literals, variables and parenthesized expressions.

unique_ptr<Expr> parsePrimary() {

    if (check(Token::Kind::IntLiteral) ||
        check(Token::Kind::DoubleLiteral)) {

        Token value = advance();

        return make_unique<LiteralExpr>(
            value.lexeme
        );
    }

    if (check(Token::Kind::Identifier)) {

        Token name = advance();

        return make_unique<VariableExpr>(
            name.lexeme
        );
    }

    if (match(Token::Kind::LeftParen)) {

        auto expr = parseExpression();

        consume(
            Token::Kind::RightParen,
            "Expected )"
        );

        return expr;
    }

    throw runtime_error(
        "Expected expression at line " +
        to_string(peek().line)
    );
}
        
29. Error Handling

The Parser reports syntax errors when the token sequence does not follow the expected grammar.

For example, if a declaration is:

সংখ্যা = 10;

the Parser expects an identifier after সংখ্যা.

Therefore, it reports an error instead of creating an invalid AST.

30. Error Synchronization

The Parser also contains an error recovery mechanism.

void synchronize() {

    if (check(Token::Kind::EndOfFile))
        return;

    advance();

    while (!check(Token::Kind::EndOfFile)) {

        if (check(Token::Kind::Semicolon)) {
            advance();
            return;
        }

        if (check(Token::Kind::KwNumber) ||
            check(Token::Kind::KwDecimal) ||
            check(Token::Kind::KwPrint) ||
            check(Token::Kind::KwIf) ||
            check(Token::Kind::KwWhile) ||
            check(Token::Kind::KwEnd)) {

            return;
        }

        advance();
    }
}

This allows the Parser to skip an invalid portion and continue parsing later statements instead of immediately stopping the whole process.

31. Complete Compiler Pipeline Used by My Part

The source code passes through my implemented stages in the following order:

Input File
    |
    v
readFile()
    |
    v
Source Code
    |
    v
Lexer
    |
    v
Token List
    |
    v
Parser
    |
    v
AST
    |
    v
Semantic Analysis
    |
    v
Code Generation
32. Example Program

A simple BanglaLang program can look like:

শুরু

সংখ্যা x = 10;
দশমিক y = 2.5;

x = x + 5;

দেখাও x;
দেখাও y;

শেষ

The processing is:

Source Code
     |
     v
Lexer
     |
     +---- Keywords
     +---- Identifiers
     +---- Literals
     +---- Operators
     +---- Punctuation
     |
     v
Tokens
     |
     v
Parser
     |
     +---- Declaration
     +---- Assignment
     +---- Print
     +---- Expression
     |
     v
AST
        
33. Example of Lexer Output

For:

সংখ্যা x = 10;

the Lexer conceptually generates:

KwNumber
Identifier
Equal
IntLiteral
Semicolon

For:

x + 5

it generates:

Identifier
Plus
IntLiteral
34. Example of Parser Structure

For:

সংখ্যা x = 10 + 5;

the Parser builds a structure conceptually like:

VarDeclStmt
 |
 +-- type = সংখ্যা
 |
 +-- name = x
 |
 +-- initializer
       |
       +-- BinaryExpr (+)
              |
              +-- LiteralExpr(10)
              |
              +-- LiteralExpr(5)
        
35. How the Lexer and Parser Work Together

The Lexer does not decide whether the complete program is grammatically correct.

It only identifies tokens.

For example:

সংখ্যা x = 10;

becomes:

Keyword Identifier = Number ;

Then the Parser checks whether this sequence is valid according to the grammar.

So:

Lexer
  |
  | "What are these pieces?"
  v
Tokens
  |
  | "Are these pieces arranged correctly?"
  v
Parser
  |
  v
AST
36. Difference Between Lexer and Parser
Lexer

The Lexer identifies:

Keywords
Identifiers
Numbers
Operators
Punctuation

Its main question is:

"What are these characters?"

Parser

The Parser identifies:

Declarations
Assignments
Expressions
Conditions
Loops
Program structure

Its main question is:

"Is this sequence of tokens grammatically correct?"

37. My Contribution Summary

My contribution to the BanglaLang Compiler focused on the first major stages of compilation.

I implemented:

Source Code Handling
Token structure and token recognition
Lexical Analysis
Keyword and identifier recognition
Literal recognition
Operator and punctuation recognition
Whitespace and comment handling
Line and column tracking
Syntax Analysis
Variable declaration parsing
Assignment parsing
Print statement parsing
If-else parsing
While loop parsing
Expression parsing with operator precedence
Syntax error handling and synchronization
AST structures required by the Parser

        Final Contribution Statement

My main contribution to the BanglaLang Compiler was implementing the source code input process, Lexer and Parser.

The Lexer converts BanglaLang source code into tokens, while the Parser validates the token sequence according to the language grammar and constructs the required AST structures.

These stages prepare the program for the later Semantic Analysis and Code Generation phases.
