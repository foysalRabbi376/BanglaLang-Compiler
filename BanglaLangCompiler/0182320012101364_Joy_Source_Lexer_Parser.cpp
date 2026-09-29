
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cctype>
#include <stdexcept>
#include <algorithm>

using namespace std;


// ============================================================
// TOKEN
// ============================================================

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
        : kind(k), lexeme(std::move(l)), line(ln), column(col) {}
};


// ============================================================
// TOKEN NAME
// ============================================================

static string kindName(Token::Kind k) {
    switch (k) {
        case Token::Kind::Identifier: return "Identifier";
        case Token::Kind::IntLiteral: return "IntLiteral";
        case Token::Kind::DoubleLiteral: return "DoubleLiteral";

        case Token::Kind::Plus: return "Plus";
        case Token::Kind::Minus: return "Minus";
        case Token::Kind::Star: return "Star";
        case Token::Kind::Slash: return "Slash";
        case Token::Kind::Percent: return "Percent";

        case Token::Kind::Equal: return "Equal";
        case Token::Kind::EqualEqual: return "EqualEqual";
        case Token::Kind::BangEqual: return "BangEqual";
        case Token::Kind::Less: return "Less";
        case Token::Kind::LessEqual: return "LessEqual";
        case Token::Kind::Greater: return "Greater";
        case Token::Kind::GreaterEqual: return "GreaterEqual";

        case Token::Kind::LeftParen: return "LeftParen";
        case Token::Kind::RightParen: return "RightParen";
        case Token::Kind::LeftBrace: return "LeftBrace";
        case Token::Kind::RightBrace: return "RightBrace";
        case Token::Kind::Comma: return "Comma";
        case Token::Kind::Semicolon: return "Semicolon";

        case Token::Kind::KwNumber: return "KwNumber";
        case Token::Kind::KwDecimal: return "KwDecimal";
        case Token::Kind::KwIf: return "KwIf";
        case Token::Kind::KwElse: return "KwElse";
        case Token::Kind::KwWhile: return "KwWhile";
        case Token::Kind::KwPrint: return "KwPrint";
        case Token::Kind::KwStart: return "KwStart";
        case Token::Kind::KwEnd: return "KwEnd";

        case Token::Kind::EndOfFile: return "EOF";
        case Token::Kind::Invalid: return "Invalid";
    }

    return "Unknown";
}


// ============================================================
// LEXER
// ============================================================

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

    void skipWhitespaceAndComments() {
        while (true) {

            // Whitespace
            while (isspace(static_cast<unsigned char>(peekChar()))) {
                advance();
            }

            // Single-line comment
            if (peekChar() == '/' && peekChar(1) == '/') {
                while (peekChar() != '\n' && peekChar() != '\0') {
                    advance();
                }
                continue;
            }

            break;
        }
    }

    Token makeToken(Token::Kind kind,
                    const string& lexeme,
                    int startLine,
                    int startColumn) {
        return Token(kind, lexeme, startLine, startColumn);
    }

    Token number() {
        int startLine = line;
        int startColumn = column;

        string value;
        bool hasDot = false;

        while (isdigit(static_cast<unsigned char>(peekChar()))) {
            value += advance();
        }

        if (peekChar() == '.' &&
            isdigit(static_cast<unsigned char>(peekChar(1)))) {

            hasDot = true;
            value += advance();

            while (isdigit(static_cast<unsigned char>(peekChar()))) {
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

public:

    explicit Lexer(string source)
        : src(std::move(source)) {}

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

            // Number
            if (isdigit(static_cast<unsigned char>(c))) {
                tokens.push_back(number());
                continue;
            }

            // Identifier / keyword
            if (isalnum(static_cast<unsigned char>(c)) ||
                c == '_') {

                tokens.push_back(identifier());
                continue;
            }

            // Operators and punctuation
            switch (c) {

                case '+':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::Plus,
                                  "+",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case '-':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::Minus,
                                  "-",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case '*':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::Star,
                                  "*",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case '/':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::Slash,
                                  "/",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case '%':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::Percent,
                                  "%",
                                  startLine,
                                  startColumn)
                    );
                    break;

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

                case '!':
                    advance();

                    if (peekChar() == '=') {
                        advance();

                        tokens.push_back(
                            makeToken(
                                Token::Kind::BangEqual,
                                "!=",
                                startLine,
                                startColumn
                            )
                        );
                    } else {
                        tokens.push_back(
                            makeToken(
                                Token::Kind::Invalid,
                                "!",
                                startLine,
                                startColumn
                            )
                        );
                    }

                    break;

                case '<':
                    advance();

                    if (peekChar() == '=') {
                        advance();

                        tokens.push_back(
                            makeToken(
                                Token::Kind::LessEqual,
                                "<=",
                                startLine,
                                startColumn
                            )
                        );
                    } else {
                        tokens.push_back(
                            makeToken(
                                Token::Kind::Less,
                                "<",
                                startLine,
                                startColumn
                            )
                        );
                    }

                    break;

                case '>':
                    advance();

                    if (peekChar() == '=') {
                        advance();

                        tokens.push_back(
                            makeToken(
                                Token::Kind::GreaterEqual,
                                ">=",
                                startLine,
                                startColumn
                            )
                        );
                    } else {
                        tokens.push_back(
                            makeToken(
                                Token::Kind::Greater,
                                ">",
                                startLine,
                                startColumn
                            )
                        );
                    }

                    break;

                case '(':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::LeftParen,
                                  "(",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case ')':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::RightParen,
                                  ")",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case '{':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::LeftBrace,
                                  "{",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case '}':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::RightBrace,
                                  "}",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case ',':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::Comma,
                                  ",",
                                  startLine,
                                  startColumn)
                    );
                    break;

                case ';':
                    advance();
                    tokens.push_back(
                        makeToken(Token::Kind::Semicolon,
                                  ";",
                                  startLine,
                                  startColumn)
                    );
                    break;

                default:
                    advance();

                    tokens.push_back(
                        makeToken(
                            Token::Kind::Invalid,
                            string(1, c),
                            startLine,
                            startColumn
                        )
                    );

                    break;
            }
        }

        return tokens;
    }
};


// ============================================================
// AST - EXPRESSIONS
// ============================================================

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

    UnaryExpr(Token operation,
              unique_ptr<Expr> expression)
        : op(std::move(operation)),
          right(std::move(expression)) {}
};

struct BinaryExpr : Expr {
    unique_ptr<Expr> left;
    Token op;
    unique_ptr<Expr> right;

    BinaryExpr(unique_ptr<Expr> l,
               Token operation,
               unique_ptr<Expr> r)
        : left(std::move(l)),
          op(std::move(operation)),
          right(std::move(r)) {}
};


// ============================================================
// AST - STATEMENTS
// ============================================================

struct Stmt {
    virtual ~Stmt() = default;
};

struct VarDeclStmt : Stmt {
    string type;
    string name;
    unique_ptr<Expr> initializer;

    VarDeclStmt(string t,
                string n,
                unique_ptr<Expr> init)
        : type(std::move(t)),
          name(std::move(n)),
          initializer(std::move(init)) {}
};

struct AssignStmt : Stmt {
    string name;
    unique_ptr<Expr> value;

    AssignStmt(string n,
               unique_ptr<Expr> v)
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


// ============================================================
// PARSER
// ============================================================

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

    const Token& consume(
        Token::Kind kind,
        const string& message
    ) {

        if (check(kind))
            return advance();

        throw runtime_error(
            message +
            " at line " +
            to_string(peek().line)
        );
    }

public:

    explicit Parser(vector<Token> tokens)
        : ts(std::move(tokens)) {}

    vector<unique_ptr<Stmt>> parseProgram() {

        vector<unique_ptr<Stmt>> statements;

        if (match(Token::Kind::KwStart)) {
            while (!check(Token::Kind::KwEnd) &&
                   !check(Token::Kind::EndOfFile)) {

                try {
                    statements.push_back(parseStatement());
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
                statements.push_back(parseStatement());
            }
            catch (const exception&) {
                synchronize();
            }
        }

        return statements;
    }


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


    unique_ptr<Stmt> parsePrint() {

        advance();

        auto expression = parseExpression();

        match(Token::Kind::Semicolon);

        return make_unique<PrintStmt>(
            std::move(expression)
        );
    }


    vector<unique_ptr<Stmt>> parseBlock() {

        vector<unique_ptr<Stmt>> statements;

        consume(
            Token::Kind::LeftBrace,
            "Expected {"
        );

        while (!check(Token::Kind::RightBrace) &&
               !check(Token::Kind::EndOfFile)) {

            statements.push_back(parseStatement());
        }

        consume(
            Token::Kind::RightBrace,
            "Expected }"
        );

        return statements;
    }


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


    unique_ptr<Expr> parseExpression() {
        return parseEquality();
    }


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
};


// ============================================================
// SOURCE CODE HANDLING
// ============================================================

static string readFile(const string& path) {

    ifstream f(path, ios::binary);

    if (!f) {
        throw runtime_error(
            "Cannot open source file: " + path
        );
    }

    stringstream ss;
    ss << f.rdbuf();

    return ss.str();
}
