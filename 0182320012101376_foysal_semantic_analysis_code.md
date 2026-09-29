# Semantic Analysis — BanglaLang Compiler

## My Part of the Project

In our BanglaLang compiler, my main responsibility was the **semantic analysis** part. The parser checks whether the source code follows the grammar, but grammar alone cannot tell us whether the program makes sense. For example, a variable may be assigned a value of the wrong type, or a variable may be used before it is declared. These checks are handled in the `SemanticAnalyzer` class in `src/main.cpp`.

The semantic analysis is done **after parsing and before code generation**.

### Overall flow

```text
Bangla Source Code
        |
      Lexer
        |
      Tokens
        |
      Parser
        |
       AST
        |
  Semantic Analyzer
        |
   Type / Name Checking
        |
   Code Generator
        |
    Python Code
```

## Semantic Analyzer Code

The following is the semantic-analysis section used in our compiler:

```cpp
enum class Type { Unknown, Int, Double, Bool };
static string typeName(Type t){
    if(t==Type::Int) return "সংখ্যা";
    if(t==Type::Double) return "দশমিক";
    if(t==Type::Bool) return "শর্ত";
    return "অজানা";
}

class SemanticAnalyzer {
    unordered_map<string,Type> symbols;
    vector<string> errors;

public:
    const vector<string>& getErrors() const {
        return errors;
    }

    bool analyze(const vector<unique_ptr<Stmt>>& p) {
        for(auto& s : p)
            checkStmt(s.get());
        return errors.empty();
    }

private:
    void error(const string& s) {
        errors.push_back(s);
    }

    Type exprType(const Expr* e) {
        if(auto x=dynamic_cast<const LiteralExpr*>(e))
            return x->isDouble ? Type::Double : Type::Int;

        if(auto x=dynamic_cast<const VariableExpr*>(e)) {
            auto it=symbols.find(x->name);
            if(it==symbols.end()) {
                error("Variable '"+x->name+"' is not declared.");
                return Type::Unknown;
            }
            return it->second;
        }

        if(auto x=dynamic_cast<const UnaryExpr*>(e)) {
            Type t=exprType(x->rhs.get());
            if(t==Type::Bool)
                error("Unary operator cannot be used with condition value.");
            return t;
        }

        if(auto x=dynamic_cast<const BinaryExpr*>(e)) {
            Type a=exprType(x->left.get());
            Type b=exprType(x->right.get());

            if(a==Type::Unknown || b==Type::Unknown)
                return Type::Unknown;

            if(x->op=="+" || x->op=="-" || x->op=="*" ||
               x->op=="/" || x->op=="%") {

                if((a!=Type::Int && a!=Type::Double) ||
                   (b!=Type::Int && b!=Type::Double)) {
                    error("Arithmetic operator '"+x->op+"' needs numeric operands.");
                    return Type::Unknown;
                }

                if(x->op=="%" && (a!=Type::Int || b!=Type::Int))
                    error("'%' operator is allowed only for সংখ্যা values.");

                return (a==Type::Double || b==Type::Double || x->op=="/")
                       ? Type::Double
                       : Type::Int;
            }

            if(x->op=="==" || x->op=="!=" || x->op=="<" ||
               x->op=="<=" || x->op==">" || x->op==">=") {

                if(!((a==Type::Int || a==Type::Double) &&
                     (b==Type::Int || b==Type::Double)))
                    error("Comparison needs numeric operands.");

                return Type::Bool;
            }
        }

        return Type::Unknown;
    }

    bool assignable(Type target, Type value) {
        return target==value ||
               (target==Type::Double && value==Type::Int);
    }

    void checkStmt(const Stmt* s) {
        if(auto x=dynamic_cast<const VarDeclStmt*>(s)) {
            if(symbols.count(x->name))
                error("Variable '"+x->name+"' is already declared.");

            Type t=(x->type=="সংখ্যা") ? Type::Int : Type::Double;
            symbols[x->name]=t;

            if(x->init) {
                Type v=exprType(x->init.get());
                if(v!=Type::Unknown && !assignable(t,v))
                    error("Type mismatch: '"+x->name+"' is " +
                          typeName(t) + " but expression is " +
                          typeName(v) + ".");
            }

        } else if(auto x=dynamic_cast<const AssignStmt*>(s)) {
            auto it=symbols.find(x->name);

            if(it==symbols.end()) {
                error("Variable '"+x->name+"' is not declared.");
                exprType(x->value.get());
                return;
            }

            Type v=exprType(x->value.get());
            if(v!=Type::Unknown && !assignable(it->second,v))
                error("Type mismatch in assignment to '"+x->name+"'.");

        } else if(auto x=dynamic_cast<const PrintStmt*>(s)) {
            exprType(x->value.get());

        } else if(auto x=dynamic_cast<const IfStmt*>(s)) {
            Type c=exprType(x->cond.get());
            if(c!=Type::Bool)
                error("IF condition must be a comparison expression.");

            for(auto& z:x->thenPart)
                checkStmt(z.get());
            for(auto& z:x->elsePart)
                checkStmt(z.get());

        } else if(auto x=dynamic_cast<const WhileStmt*>(s)) {
            Type c=exprType(x->cond.get());
            if(c!=Type::Bool)
                error("WHILE condition must be a comparison expression.");

            for(auto& z:x->body)
                checkStmt(z.get());
        }
    }
};
```

## How I implemented it

### 1. Type system

At first I defined a small type system:

```cpp
enum class Type { Unknown, Int, Double, Bool };
```

Here:

- `Int` represents `সংখ্যা`.
- `Double` represents `দশমিক`.
- `Bool` represents a comparison result.
- `Unknown` is used when the compiler cannot determine a valid type because an earlier semantic error occurred.

I kept `typeName()` so that error messages can use the language's own type names.

### 2. Symbol table

The main data structure for semantic analysis is:

```cpp
unordered_map<string,Type> symbols;
```

I used the variable name as the key and the declared type as the value.

For example, after processing:

```text
সংখ্যা a = 10;
দশমিক b = 2.5;
```

the table conceptually contains:

```text
a -> Int
b -> Double
```

This table is used to detect undeclared variables, duplicate declarations and invalid assignments.

### 3. Checking expressions

The `exprType()` function finds the type of an expression.

For a literal:

```cpp
if(auto x=dynamic_cast<const LiteralExpr*>(e))
    return x->isDouble ? Type::Double : Type::Int;
```

So an integer literal becomes `Int`, while a decimal literal becomes `Double`.

For a variable, I search the symbol table. If the name is missing, an error is recorded:

```cpp
if(it==symbols.end()) {
    error("Variable '"+x->name+"' is not declared.");
    return Type::Unknown;
}
```

### 4. Arithmetic checking

For `+`, `-`, `*`, `/` and `%`, both operands must be numeric (`Int` or `Double`).

The result is normally `Int`, but a decimal operand produces `Double`. Division is also treated as `Double` in this toy language.

The `%` operator has a stricter rule: both operands must be integers.

### 5. Comparison checking

For operators such as:

```text
==  !=  <  <=  >  >=
```

I check that both sides are numeric. The result type is then:

```cpp
Type::Bool
```

That makes it possible to validate conditions in `যদি` and `যতক্ষণ`.

### 6. Assignment compatibility

This helper function handles assignment rules:

```cpp
bool assignable(Type target, Type value) {
    return target==value ||
           (target==Type::Double && value==Type::Int);
}
```

So these are accepted:

```text
সংখ্যা a = 10;
দশমিক b = 10;
b = a;
```

But an integer variable cannot receive a decimal value.

### 7. Statement checking

The `checkStmt()` function handles the semantic rules for each statement kind.

For a variable declaration, it checks duplicate names and the type of the initializer.

For assignment, it first checks whether the variable exists and then checks whether the new value is compatible with the declared type.

For `দেখাও`, it still checks the expression so an undeclared variable is caught.

For `যদি` and `যতক্ষণ`, the condition must produce `Bool`, and then the statements inside their blocks are checked recursively.

### 8. Collecting errors

I did not stop at the first semantic error. The `errors` vector stores the problems found during the analysis. After checking the program, `main()` prints the collected errors and stops before code generation.

This is useful because the user gets a readable compiler message instead of a crash.

## Example: type mismatch

A test program such as:

```text
শুরু {
    সংখ্যা x = 10;
    x = 3.5;
}
শেষ
```

produces:

```text
Compilation failed with 1 semantic error(s):
  - Type mismatch in assignment to 'x'.
```

The important point is that the program is syntactically valid. The problem is found later during semantic analysis because `x` was declared as `সংখ্যা` but the assigned value is decimal.

## Where the semantic phase is called

In `main()`, the order is:

```cpp
Lexer lex(source);
auto tokens=lex.tokenize();

Parser parser(tokens);
auto program=parser.parseProgram();

SemanticAnalyzer sem;
sem.analyze(program);

CodeGenerator gen;
string code=gen.generate(program);
```

Before generation, semantic errors are checked:

```cpp
if(!sem.getErrors().empty()) {
    cerr << "Compilation failed with "
         << sem.getErrors().size()
         << " semantic error(s):\n";

    for(auto& e:sem.getErrors())
        cerr << "  - " << e << "\n";

    return 3;
}
```

So the basic rule is:

```text
Parse successfully
      ↓
Semantic check
      ↓
No semantic error?
      ↓
Generate Python code
```

## What I would explain in the viva

**Q: What is semantic analysis?**  
Semantic analysis checks the meaning of a syntactically correct program. In our compiler it mainly checks declarations, variable usage, type compatibility and conditions.

**Q: What is your symbol table?**  
`unordered_map<string,Type> symbols` stores each declared variable with its type.

**Q: Why do you need `exprType()`?**  
Because the compiler needs to know the type of an expression before deciding whether an operation or assignment is valid.

**Q: Why is `assignable()` needed?**  
It keeps the assignment rule in one place. Same-type assignment is allowed, and `Int` to `Double` conversion is also allowed.

**Q: What is an example of a semantic error?**  
Assigning a decimal value to a `সংখ্যা` variable, or using a variable before declaring it.

**Q: When does semantic analysis happen?**  
After parsing and before code generation.

## My contribution summary

My part connects the parsed program with the code generator. The parser tells us that the structure is valid, and the semantic analyzer checks whether the program is meaningful. Only after these checks pass do we generate the Python target code.
