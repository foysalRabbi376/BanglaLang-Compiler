#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <stdexcept>
#include <iomanip>
#include <filesystem>

using namespace std;

// BanglaLang Compiler - C++17
// Source language keywords are intentionally simple and consistent with the BNF in docs/grammar.bnf.

struct Token {
    enum class Kind {
        Identifier, IntLiteral, DoubleLiteral,
        Plus, Minus, Star, Slash, Percent,
        Assign, Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual,
        LParen, RParen, LBrace, RBrace, Semicolon,
        KeywordNumber, KeywordDecimal, KeywordIf, KeywordElse, KeywordWhile,
        KeywordPrint, KeywordStart, KeywordEnd,
        EndOfFile, Invalid
    };
    Kind kind;
    string lexeme;
    int line;
    int column;
};

static string kindName(Token::Kind k) {
    switch (k) {
        case Token::Kind::Identifier: return "identifier";
        case Token::Kind::IntLiteral: return "integer";
        case Token::Kind::DoubleLiteral: return "decimal";
        case Token::Kind::Plus: return "+"; case Token::Kind::Minus: return "-";
        case Token::Kind::Star: return "*"; case Token::Kind::Slash: return "/";
        case Token::Kind::Percent: return "%"; case Token::Kind::Assign: return "=";
        case Token::Kind::Equal: return "=="; case Token::Kind::NotEqual: return "!=";
        case Token::Kind::Less: return "<"; case Token::Kind::LessEqual: return "<=";
        case Token::Kind::Greater: return ">"; case Token::Kind::GreaterEqual: return ">=";
        case Token::Kind::LParen: return "("; case Token::Kind::RParen: return ")";
        case Token::Kind::LBrace: return "{"; case Token::Kind::RBrace: return "}";
        case Token::Kind::Semicolon: return ";"; case Token::Kind::KeywordNumber: return "সংখ্যা";
        case Token::Kind::KeywordDecimal: return "দশমিক"; case Token::Kind::KeywordIf: return "যদি";
        case Token::Kind::KeywordElse: return "নাহলে"; case Token::Kind::KeywordWhile: return "যতক্ষণ";
        case Token::Kind::KeywordPrint: return "দেখাও"; case Token::Kind::KeywordStart: return "শুরু";
        case Token::Kind::KeywordEnd: return "শেষ"; case Token::Kind::EndOfFile: return "EOF";
        default: return "invalid";
    }
}

class Lexer {
    const string &src; size_t pos = 0; int line = 1, col = 1;
    unordered_map<string, Token::Kind> keywords;
    vector<string> errors;
public:
    explicit Lexer(const string& s): src(s) {
        keywords = {
            {"সংখ্যা", Token::Kind::KeywordNumber}, {"দশমিক", Token::Kind::KeywordDecimal},
            {"যদি", Token::Kind::KeywordIf}, {"নাহলে", Token::Kind::KeywordElse},
            {"যতক্ষণ", Token::Kind::KeywordWhile}, {"দেখাও", Token::Kind::KeywordPrint},
            {"শুরু", Token::Kind::KeywordStart}, {"শেষ", Token::Kind::KeywordEnd}
        };
    }
    const vector<string>& getErrors() const { return errors; }

    vector<Token> tokenize() {
        vector<Token> out;
        while (true) {
            skipSpaceAndComments();
            Token t = nextToken();
            out.push_back(t);
            if (t.kind == Token::Kind::Invalid) {
                errors.push_back("Line " + to_string(t.line) + ": invalid token '" + t.lexeme + "'.");
            }
            if (t.kind == Token::Kind::EndOfFile) break;
        }
        return out;
    }
private:
    bool eof() const { return pos >= src.size(); }
    char peek(size_t off=0) const { return pos+off < src.size() ? src[pos+off] : '\0'; }
    char advance() { char c = peek(); if (c) { pos++; if (c=='\n') { line++; col=1; } else col++; } return c; }
    void skipSpaceAndComments() {
        while (!eof()) {
            if (isspace(static_cast<unsigned char>(peek()))) { advance(); continue; }
            if (peek()=='/' && peek(1)=='/') { while (!eof() && peek()!='\n') advance(); continue; }
            if (peek()=='#') { while (!eof() && peek()!='\n') advance(); continue; }
            break;
        }
    }
    Token make(Token::Kind k, const string& lex, int l, int c) { return Token{k,lex,l,c}; }
    Token nextToken() {
        int l=line,c=col; if (eof()) return make(Token::Kind::EndOfFile,"",l,c);
        char ch=peek();
        auto one=[&](Token::Kind k){ string x(1,advance()); return make(k,x,l,c); };
        switch(ch) {
            case '+': return one(Token::Kind::Plus); case '-': return one(Token::Kind::Minus);
            case '*': return one(Token::Kind::Star); case '/': return one(Token::Kind::Slash);
            case '%': return one(Token::Kind::Percent); case '(': return one(Token::Kind::LParen);
            case ')': return one(Token::Kind::RParen); case '{': return one(Token::Kind::LBrace);
            case '}': return one(Token::Kind::RBrace); case ';': return one(Token::Kind::Semicolon);
            case '=': advance(); if (peek()=='=') { advance(); return make(Token::Kind::Equal,"==",l,c); } return make(Token::Kind::Assign,"=",l,c);
            case '!': advance(); if (peek()=='=') { advance(); return make(Token::Kind::NotEqual,"!=",l,c); } return make(Token::Kind::Invalid,"!",l,c);
            case '<': advance(); if (peek()=='=') { advance(); return make(Token::Kind::LessEqual,"<=",l,c); } return make(Token::Kind::Less,"<",l,c);
            case '>': advance(); if (peek()=='=') { advance(); return make(Token::Kind::GreaterEqual,">=",l,c); } return make(Token::Kind::Greater,">",l,c);
            default: break;
        }
        if (ch >= '0' && ch <= '9') return number();
        // For UTF-8 Bangla identifiers/keywords, each byte is accepted until a delimiter.
        if (!isspace(static_cast<unsigned char>(ch)) && string("+-*/%=!<>(){};#").find(ch) == string::npos) return identifier();
        string bad(1,advance()); return make(Token::Kind::Invalid,bad,l,c);
    }
    Token number() {
        int l=line,c=col; string s; bool dot=false;
        while (!eof()) {
            char ch=peek();
            if (ch>='0' && ch<='9') { s+=advance(); continue; }
            if (ch=='.' && !dot) { dot=true; s+=advance(); continue; }
            break;
        }
        return make(dot?Token::Kind::DoubleLiteral:Token::Kind::IntLiteral,s,l,c);
    }
    Token identifier() {
        int l=line,c=col; string s;
        while (!eof()) {
            char ch=peek();
            if (isspace(static_cast<unsigned char>(ch)) || string("+-*/%=!<>(){};#").find(ch) != string::npos) break;
            s+=advance();
        }
        auto it=keywords.find(s);
        if (it!=keywords.end()) return make(it->second,s,l,c);
        return make(Token::Kind::Identifier,s,l,c);
    }
};

struct Expr { virtual ~Expr() = default; };
struct LiteralExpr: Expr { string value; bool isDouble; LiteralExpr(string v,bool d):value(move(v)),isDouble(d){} };
struct VariableExpr: Expr { string name; explicit VariableExpr(string n):name(move(n)){} };
struct UnaryExpr: Expr { string op; unique_ptr<Expr> rhs; UnaryExpr(string o,unique_ptr<Expr> r):op(move(o)),rhs(move(r)){} };
struct BinaryExpr: Expr { string op; unique_ptr<Expr> left,right; BinaryExpr(string o,unique_ptr<Expr> l,unique_ptr<Expr> r):op(move(o)),left(move(l)),right(move(r)){} };

struct Stmt { virtual ~Stmt() = default; };
struct VarDeclStmt: Stmt { string type,name; unique_ptr<Expr> init; VarDeclStmt(string t,string n,unique_ptr<Expr> i):type(move(t)),name(move(n)),init(move(i)){} };
struct AssignStmt: Stmt { string name; unique_ptr<Expr> value; AssignStmt(string n,unique_ptr<Expr> v):name(move(n)),value(move(v)){} };
struct PrintStmt: Stmt { unique_ptr<Expr> value; explicit PrintStmt(unique_ptr<Expr> v):value(move(v)){} };
struct IfStmt: Stmt { unique_ptr<Expr> cond; vector<unique_ptr<Stmt>> thenPart, elsePart; };
struct WhileStmt: Stmt { unique_ptr<Expr> cond; vector<unique_ptr<Stmt>> body; };

class Parser {
    vector<Token> ts; size_t i=0; vector<string> errors;
public:
    explicit Parser(vector<Token> t):ts(move(t)){}
    const vector<string>& getErrors() const { return errors; }
    vector<unique_ptr<Stmt>> parseProgram() {
        vector<unique_ptr<Stmt>> p;
        if (!match(Token::Kind::KeywordStart)) {
            errors.push_back("Line " + to_string(peek().line) + ": program must start with 'শুরু'.");
            while (!check(Token::Kind::EndOfFile)) advance();
            return p;
        }
        if (!match(Token::Kind::LBrace)) {
            errors.push_back("Line " + to_string(peek().line) + ": '{' expected after 'শুরু'.");
        } else {
            while (!check(Token::Kind::RBrace) && !check(Token::Kind::EndOfFile)) {
                size_t before=i; auto st=parseStatement(); if(st) p.push_back(move(st));
                if(i==before) synchronize();
            }
            expect(Token::Kind::RBrace,"'}' expected before 'শেষ'");
        }
        if (!match(Token::Kind::KeywordEnd)) {
            errors.push_back("Line " + to_string(peek().line) + ": 'শেষ' expected at end of program.");
        }
        if (!check(Token::Kind::EndOfFile)) {
            errors.push_back("Line " + to_string(peek().line) + ": unexpected content after 'শেষ'.");
            while (!check(Token::Kind::EndOfFile)) advance();
        }
        return p;
    }
private:
    const Token& peek(int n=0) const { return ts[min(i+(size_t)n,ts.size()-1)]; }
    const Token& previous() const { return ts[i-1]; }
    bool check(Token::Kind k) const { return peek().kind==k; }
    Token advance(){ if(i<ts.size()) i++; return previous(); }
    bool match(Token::Kind k){ if(check(k)){advance();return true;} return false; }
    void expect(Token::Kind k,const string& msg){ if(!match(k)){ errors.push_back("Line "+to_string(peek().line)+": "+msg+". Found '"+kindName(peek().kind)+"'."); } }

    unique_ptr<Stmt> parseStatement(){
        if (check(Token::Kind::KeywordNumber) || check(Token::Kind::KeywordDecimal)) return parseDecl();
        if (check(Token::Kind::Identifier)) return parseAssign();
        if (check(Token::Kind::KeywordPrint)) return parsePrint();
        if (check(Token::Kind::KeywordIf)) return parseIf();
        if (check(Token::Kind::KeywordWhile)) return parseWhile();
        errors.push_back("Line "+to_string(peek().line)+": unexpected token '"+kindName(peek().kind)+"'.");
        return nullptr;
    }
    unique_ptr<Stmt> parseDecl(){
        string type; if (match(Token::Kind::KeywordNumber)) type="সংখ্যা"; else { match(Token::Kind::KeywordDecimal); type="দশমিক"; }
        if(!check(Token::Kind::Identifier)){ errors.push_back("Line "+to_string(peek().line)+": variable name expected."); synchronize(); return nullptr; }
        string n=advance().lexeme; unique_ptr<Expr> init;
        if(match(Token::Kind::Assign)) init=parseExpression();
        expect(Token::Kind::Semicolon,"';' expected after declaration");
        return make_unique<VarDeclStmt>(type,n,move(init));
    }
    unique_ptr<Stmt> parseAssign(){
        string n=advance().lexeme;
        expect(Token::Kind::Assign,"'=' expected in assignment");
        auto e=parseExpression(); expect(Token::Kind::Semicolon,"';' expected after assignment");
        return make_unique<AssignStmt>(n,move(e));
    }
    unique_ptr<Stmt> parsePrint(){
        advance(); expect(Token::Kind::LParen,"'(' expected after দেখাও");
        auto e=parseExpression(); expect(Token::Kind::RParen,"')' expected after expression");
        expect(Token::Kind::Semicolon,"';' expected after print statement");
        return make_unique<PrintStmt>(move(e));
    }
    vector<unique_ptr<Stmt>> parseBlock(){
        vector<unique_ptr<Stmt>> b;
        expect(Token::Kind::LBrace,"'{' expected");
        while(!check(Token::Kind::RBrace) && !check(Token::Kind::EndOfFile)){
            size_t before=i; auto st=parseStatement(); if(st) b.push_back(move(st)); if(i==before) synchronize();
        }
        expect(Token::Kind::RBrace,"'}' expected"); return b;
    }
    unique_ptr<Stmt> parseIf(){
        advance(); expect(Token::Kind::LParen,"'(' expected after যদি"); auto c=parseExpression(); expect(Token::Kind::RParen,"')' expected after condition");
        auto st=make_unique<IfStmt>(); st->cond=move(c); st->thenPart=parseBlock();
        if(match(Token::Kind::KeywordElse)) st->elsePart=parseBlock();
        return st;
    }
    unique_ptr<Stmt> parseWhile(){
        advance(); expect(Token::Kind::LParen,"'(' expected after যতক্ষণ"); auto c=parseExpression(); expect(Token::Kind::RParen,"')' expected after condition");
        auto st=make_unique<WhileStmt>(); st->cond=move(c); st->body=parseBlock(); return st;
    }

    unique_ptr<Expr> parseExpression(){ return parseEquality(); }
    unique_ptr<Expr> parseEquality(){
        auto e=parseComparison(); while(check(Token::Kind::Equal)||check(Token::Kind::NotEqual)){string op=advance().lexeme; auto r=parseComparison(); e=make_unique<BinaryExpr>(op,move(e),move(r));} return e;
    }
    unique_ptr<Expr> parseComparison(){
        auto e=parseTerm(); while(check(Token::Kind::Less)||check(Token::Kind::LessEqual)||check(Token::Kind::Greater)||check(Token::Kind::GreaterEqual)){string op=advance().lexeme;auto r=parseTerm();e=make_unique<BinaryExpr>(op,move(e),move(r));}return e;
    }
    unique_ptr<Expr> parseTerm(){
        auto e=parseFactor(); while(check(Token::Kind::Plus)||check(Token::Kind::Minus)){string op=advance().lexeme;auto r=parseFactor();e=make_unique<BinaryExpr>(op,move(e),move(r));}return e;
    }
    unique_ptr<Expr> parseFactor(){
        auto e=parseUnary(); while(check(Token::Kind::Star)||check(Token::Kind::Slash)||check(Token::Kind::Percent)){string op=advance().lexeme;auto r=parseUnary();e=make_unique<BinaryExpr>(op,move(e),move(r));}return e;
    }
    unique_ptr<Expr> parseUnary(){
        if(check(Token::Kind::Minus)){advance();return make_unique<UnaryExpr>("-",parseUnary());}
        if(check(Token::Kind::Plus)){advance();return parseUnary();}
        return parsePrimary();
    }
    unique_ptr<Expr> parsePrimary(){
        if(check(Token::Kind::IntLiteral)){auto t=advance();return make_unique<LiteralExpr>(t.lexeme,false);}
        if(check(Token::Kind::DoubleLiteral)){auto t=advance();return make_unique<LiteralExpr>(t.lexeme,true);}
        if(check(Token::Kind::Identifier)){return make_unique<VariableExpr>(advance().lexeme);}
        if(match(Token::Kind::LParen)){auto e=parseExpression();expect(Token::Kind::RParen,"')' expected");return e;}
        errors.push_back("Line "+to_string(peek().line)+": expression expected."); return make_unique<LiteralExpr>("0",false);
    }
    void synchronize(){
        while(!check(Token::Kind::EndOfFile) && !check(Token::Kind::Semicolon) && !check(Token::Kind::RBrace)) advance();
        if(check(Token::Kind::Semicolon)) advance();
    }
};

enum class Type { Unknown, Int, Double, Bool };
static string typeName(Type t){ if(t==Type::Int)return"সংখ্যা"; if(t==Type::Double)return"দশমিক"; if(t==Type::Bool)return"শর্ত"; return"অজানা"; }

class SemanticAnalyzer {
    unordered_map<string,Type> symbols; vector<string> errors;
public:
    const vector<string>& getErrors()const{return errors;}
    bool analyze(const vector<unique_ptr<Stmt>>& p){ for(auto& s:p) checkStmt(s.get()); return errors.empty(); }
private:
    void error(const string&s){errors.push_back(s);} 
    Type exprType(const Expr* e){
        if(auto x=dynamic_cast<const LiteralExpr*>(e)) return x->isDouble?Type::Double:Type::Int;
        if(auto x=dynamic_cast<const VariableExpr*>(e)){auto it=symbols.find(x->name);if(it==symbols.end()){error("Variable '"+x->name+"' is not declared.");return Type::Unknown;}return it->second;}
        if(auto x=dynamic_cast<const UnaryExpr*>(e)){Type t=exprType(x->rhs.get()); if(t==Type::Bool) error("Unary operator cannot be used with condition value."); return t;}
        if(auto x=dynamic_cast<const BinaryExpr*>(e)){
            Type a=exprType(x->left.get()), b=exprType(x->right.get());
            if(a==Type::Unknown||b==Type::Unknown)return Type::Unknown;
            if(x->op=="+"||x->op=="-"||x->op=="*"||x->op=="/"||x->op=="%"){
                if((a!=Type::Int&&a!=Type::Double)||(b!=Type::Int&&b!=Type::Double)){error("Arithmetic operator '"+x->op+"' needs numeric operands.");return Type::Unknown;}
                if(x->op=="%" && (a!=Type::Int||b!=Type::Int)) error("'%' operator is allowed only for সংখ্যা values.");
                return (a==Type::Double||b==Type::Double||x->op=="/")?Type::Double:Type::Int;
            }
            if(x->op=="=="||x->op=="!="||x->op=="<"||x->op=="<="||x->op==">"||x->op==">="){
                if(!((a==Type::Int||a==Type::Double)&&(b==Type::Int||b==Type::Double))) error("Comparison needs numeric operands.");
                return Type::Bool;
            }
        }
        return Type::Unknown;
    }
    bool assignable(Type target,Type value){return target==value || (target==Type::Double && value==Type::Int);}
    void checkStmt(const Stmt* s){
        if(auto x=dynamic_cast<const VarDeclStmt*>(s)){
            if(symbols.count(x->name)) error("Variable '"+x->name+"' is already declared.");
            Type t=(x->type=="সংখ্যা")?Type::Int:Type::Double; symbols[x->name]=t;
            if(x->init){Type v=exprType(x->init.get());if(v!=Type::Unknown&&!assignable(t,v))error("Type mismatch: '"+x->name+"' is "+typeName(t)+" but expression is "+typeName(v)+".");}
        } else if(auto x=dynamic_cast<const AssignStmt*>(s)){
            auto it=symbols.find(x->name); if(it==symbols.end()){error("Variable '"+x->name+"' is not declared."); exprType(x->value.get()); return;} Type v=exprType(x->value.get()); if(v!=Type::Unknown&&!assignable(it->second,v))error("Type mismatch in assignment to '"+x->name+"'.");
        } else if(auto x=dynamic_cast<const PrintStmt*>(s)){exprType(x->value.get());}
        else if(auto x=dynamic_cast<const IfStmt*>(s)){Type c=exprType(x->cond.get());if(c!=Type::Bool)error("IF condition must be a comparison expression.");for(auto&z:x->thenPart)checkStmt(z.get());for(auto&z:x->elsePart)checkStmt(z.get());}
        else if(auto x=dynamic_cast<const WhileStmt*>(s)){Type c=exprType(x->cond.get());if(c!=Type::Bool)error("WHILE condition must be a comparison expression.");for(auto&z:x->body)checkStmt(z.get());}
    }
};

class CodeGenerator {
    ostringstream out; int indent=0;
    string ind()const{return string(indent*4,' ');} 
    string expr(const Expr* e){
        if(auto x=dynamic_cast<const LiteralExpr*>(e))return x->value;
        if(auto x=dynamic_cast<const VariableExpr*>(e))return x->name;
        if(auto x=dynamic_cast<const UnaryExpr*>(e))return "("+x->op+expr(x->rhs.get())+")";
        if(auto x=dynamic_cast<const BinaryExpr*>(e))return "("+expr(x->left.get())+" "+x->op+" "+expr(x->right.get())+")";
        return "0";
    }
    void stmts(const vector<unique_ptr<Stmt>>& p){for(auto& s:p) stmt(s.get());}
    void stmt(const Stmt* s){
        if(auto x=dynamic_cast<const VarDeclStmt*>(s)){out<<ind()<<"# "<<x->type<<" "<<x->name<<"\n"; out<<ind()<<x->name<<" = "<<(x->init?expr(x->init.get()):"0")<<"\n";}
        else if(auto x=dynamic_cast<const AssignStmt*>(s))out<<ind()<<x->name<<" = "<<expr(x->value.get())<<"\n";
        else if(auto x=dynamic_cast<const PrintStmt*>(s))out<<ind()<<"print("<<expr(x->value.get())<<")\n";
        else if(auto x=dynamic_cast<const IfStmt*>(s)){out<<ind()<<"if "<<expr(x->cond.get())<<":\n";indent++;if(x->thenPart.empty())out<<ind()<<"pass\n";else stmts(x->thenPart);indent--;if(!x->elsePart.empty()){out<<ind()<<"else:\n";indent++;stmts(x->elsePart);indent--;}}
        else if(auto x=dynamic_cast<const WhileStmt*>(s)){out<<ind()<<"while "<<expr(x->cond.get())<<":\n";indent++;if(x->body.empty())out<<ind()<<"pass\n";else stmts(x->body);indent--;}
    }
public:
    string generate(const vector<unique_ptr<Stmt>>& p){out<<"# Generated by BanglaLang Compiler\n# Target: Python 3\n\n";stmts(p);return out.str();}
};

static string readFile(const string& path){ifstream f(path,ios::binary);if(!f)throw runtime_error("Cannot open source file: "+path);stringstream ss;ss<<f.rdbuf();return ss.str();}
static void writeFile(const string& path,const string& data){ofstream f(path,ios::binary);if(!f)throw runtime_error("Cannot write output file: "+path);f<<data;}

int main(int argc,char**argv){
    if(argc<2){cerr<<"Usage: banglacc <source.bng> [-o output.py]\n";return 1;}
    string sourcePath=argv[1], outPath="program.py";
    for(int a=2;a<argc;a++){string x=argv[a];if(x=="-o"&&a+1<argc)outPath=argv[++a];else if(x=="--help"){cout<<"Usage: banglacc <source.bng> [-o output.py]\n";return 0;}}
    try{
        string source=readFile(sourcePath);
        Lexer lex(source);auto tokens=lex.tokenize();
        Parser parser(tokens);auto program=parser.parseProgram();
        vector<string> errors=lex.getErrors();errors.insert(errors.end(),parser.getErrors().begin(),parser.getErrors().end());
        if(!errors.empty()){
            cerr<<"Compilation failed with "<<errors.size()<<" error(s):\n";for(auto&e:errors)cerr<<"  - "<<e<<"\n";return 2;
        }
        SemanticAnalyzer sem;sem.analyze(program);if(!sem.getErrors().empty()){cerr<<"Compilation failed with "<<sem.getErrors().size()<<" semantic error(s):\n";for(auto&e:sem.getErrors())cerr<<"  - "<<e<<"\n";return 3;}
        CodeGenerator gen;string code=gen.generate(program);writeFile(outPath,code);
        cout<<"Compilation successful.\nGenerated: "<<outPath<<"\n";
        return 0;
    }catch(const exception& e){cerr<<"Compiler error: "<<e.what()<<"\n";return 10;}
}
