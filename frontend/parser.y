%{
  #include "ast.hpp"
  #include <memory>
  #include <cstring>
  #include <stdarg.h>

  #include "../include/convert_ast_to_ir.h"

  using namespace std;
  using namespace Ast;
  vector<pNode> root; /* the top level root node of our final AST */

  extern int yylineno;
  extern int yylex();
  extern void yyerror(const char *s);
  extern void setFileName(const char *name);
  char filename[256];
%}

%union {
  Vector<DefAST*>* defList;
  DefAST* def;
  Node* initVal;
  Node* funcDef;
  Vector<TypedSym>* FuncFParamList;
  TypedSym* funcFParam;
  Node* block;
  Node* stmt;
  Node* returnStmt;
  Node* selectStmt;
  Node* iterationStmt;
  Node* lVal;
  Vector<pNode>* args;

  Node* exp;

  TYPE ty;
  UnaryType op;
  std::string* token;
  int int_val;
  float float_val;
};

// %union is not used, and %token and %type expect genuine types, not type tags
%type <args> CompUnit;
%type <args> DeclDef;
%type <args> Decl;
%type <defList> DefList;
%type <def> Def;
%type <args> ArrayIndexes;
%type <args> InitValList;
%type <initVal> InitVal;
%type <funcDef> FuncDef;
%type <FuncFParamList> FuncFParamList
%type <funcFParam> FuncFParam;
%type <block> Block;
%type <args> BlockItemList;
%type <args> BlockItem;
%type <stmt> Stmt;
%type <returnStmt> ReturnStmt;
%type <selectStmt> SelectStmt;
%type <iterationStmt> IterationStmt;
%type <lVal> LVal;
%type <exp> PrimaryExp;
%type <exp> Number;
%type <exp> UnaryExp;
%type <exp> Call;
%type <args> Args;
%type <exp> MulExp;
%type <exp> Exp AddExp;
%type <exp> RelExp;
%type <exp> EqExp;
%type <exp> LAndExp;
%type <exp> Cond LOrExp;

%type <ty> BType
%type <op> UnaryOp

// %token 定义终结符的语义值类型
%token <int_val> INT           // 指定INT字面量的语义值是type_int，有词法分析得到的数值
%token <float_val> FLOAT       // 指定FLOAT字面量的语义值是type_float，有词法分析得到的数值
%token <token> ID              // 指定ID
%token GTE LTE GT LT EQ NEQ    // 关系运算
%token INTTYPE FLOATTYPE VOID  // 数据类型
%token CONST RETURN IF ELSE WHILE BREAK CONTINUE
%token LP RP LB RB LC RC COMMA SEMICOLON
%token NOT ASSIGN MINUS ADD MUL DIV MOD AND OR
// Unused tokens
/* %token POS NEG */

%left ASSIGN
%left OR AND
%left EQ NEQ
%left GTE LTE GT LT
%left ADD MINUS
%left MOD MUL DIV
%right NOT
// Unused tokens
/* %right POS NEG */

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

%start Program

%%
Program:
  CompUnit {
    root = *$1;
  };

// 编译单元
CompUnit:
  CompUnit DeclDef {
    $$ = $1;
    $$->insert($$->end(), $2->begin(), $2->end());
    delete $2;
  }|
  DeclDef {
    $$ = $1;
  };

//声明或者函数定义
DeclDef:
  Decl {
    $$ = $1;
  }|
  FuncDef {
    $$ = new Vector<pNode>();
    $$->push_back(pNode($1));
  };

// 变量或常量声明
Decl:
  CONST BType DefList SEMICOLON {
    $$ = new Vector<pNode>();
    for (auto&& def : *$3) {
      $$->push_back(def->create(true, $2));
    }
    delete $3;
  }|
  BType DefList SEMICOLON {
    $$ = new Vector<pNode>();
    for (auto&& def : *$2) {
      $$->push_back(def->create(false, $1));
    }
    delete $2;
  };

// 基本类型
BType:
  INTTYPE {
    $$ = TYPE_INT;
  }|
  FLOATTYPE {
    $$ = TYPE_FLOAT;
  };

// 定义列表
DefList:
  Def {
    $$ = new Vector<DefAST*>();
    $$->push_back($1);
  }|
  DefList COMMA Def {
    $$ = $1;
    $$->push_back($3);
  };

// 定义
Def:
  ID ArrayIndexes ASSIGN InitVal { // int a[10] = {{0, 0}, {1, 1}}
    $$ = new DefAST();
    $$->id = *$1;
    delete $1; 
    $$->indexes = *$2;
    delete $2;
    $$->initVal = pNode($4);
  }|
  ID ASSIGN InitVal {
    $$ = new DefAST();
    $$->id = *$1;
    delete $1; 
    $$->initVal = pNode($3);
  }|
  ID ArrayIndexes {
    $$ = new DefAST();
    $$->id = *$1;
    delete $1; 
    $$->indexes = *$2;
    delete $2;
  }|
  ID {
    $$ = new DefAST();
    $$->id = *$1;
    delete $1; 
  };

// 数组
ArrayIndexes:
  LB Exp RB {
    $$ = new Vector<pNode>();
    $$->push_back(pNode($2));
  }|
  ArrayIndexes LB Exp RB {
    $$ = $1;
    $$->push_back(pNode($3));
  };


// 变量或常量初值
InitVal:
  Exp {
    $$ = $1;
  }|
  LC RC { // { }
    $$ = new ArrayDefNode(NULL, {});
  }|
  LC InitValList RC { // {1, 2, 3}
    $$ = new ArrayDefNode(NULL, *$2);
    delete $2;
  };

// 变量列表
InitValList: // {1, 1, 1, 1} 
  InitValList COMMA InitVal {
    $$ = $1;
    $$->push_back(pNode($3));
  }|
  InitVal {
    $$ = new Vector<pNode>();
    $$->push_back(pNode($1));
  };

// 函数定义
FuncDef:
  BType ID LP FuncFParamList RP Block {
    $$ = new FuncDefNode(NULL, toTypedSym($2, $1), *$4, pNode($6));
    delete $4;
  }|
  BType ID LP RP Block {
    $$ = new FuncDefNode(NULL, toTypedSym($2, $1), {}, pNode($5));
  }|
  VOID ID LP FuncFParamList RP Block {
    $$ = new FuncDefNode(NULL, toTypedSym($2, TYPE_VOID), *$4, pNode($6));
    delete $4;
  }|
  VOID ID LP RP Block {
    $$ = new FuncDefNode(NULL, toTypedSym($2, TYPE_VOID), {}, pNode($5));
  };

// 函数形参列表
FuncFParamList:
  FuncFParam {
    $$ = new Vector<TypedSym>();
    $$->push_back(*$1);
    delete $1;
  }|
  FuncFParamList COMMA FuncFParam {
    $$ = $1;
    $$->push_back(*$3);
    delete $3;
  };

// 函数形参
FuncFParam:
  BType ID {
    $$ = new TypedSym(toTypedSym($2, $1));
  }|
  BType ID LB RB { // TODO
    $$ = new TypedSym(toTypedSym($2, make_pointer_type(toPTYPE($1))));
  }|
  BType ID LB RB ArrayIndexes { // TODO
    auto innerTy = toPTYPE($1);
    for(auto && i : *$5) { // vector<pNode>
      auto immValue = AstToIr::Convertor::constant_eval(i);
      if(is_imm_integer(immValue.ty)) {
        innerTy = make_array_type(innerTy, immValue.val.uval);
      } else {
        puts("index must be integer");
        exit(0);
      }
    }
    $$ = new TypedSym(toTypedSym($2, make_pointer_type(innerTy)));
  };

// 语句块
Block:
  LC RC {
    $$ = new BlockNode(NULL, {});
  }|
  LC BlockItemList RC {
    $$ = new BlockNode(NULL, AstProg($2->begin(), $2->end()));
    delete $2;
  };

// 语句块项列表
BlockItemList:
  BlockItem {
    $$ = $1;
  }|
  BlockItemList BlockItem {
    $$ = $1;
    $$->insert($$->end(), $2->begin(), $2->end());
    delete $2;
  };

BlockItem:
  Decl {
    $$ = $1;
  }|
  Stmt {
    $$ = new Vector<pNode>();
    $$->push_back(pNode($1));
  };

Stmt:
  SEMICOLON {
    $$ = NULL;
  }|
  LVal ASSIGN Exp SEMICOLON {
    $$ = new AssignNode(NULL, pNode($1), pNode($3));
  }|
  Exp SEMICOLON {
    $$ = $1;
  }|
  CONTINUE SEMICOLON {
    $$ = new BreakNode(NULL);
  }|
  BREAK SEMICOLON {
    $$ = new BreakNode(NULL);
  }|
  Block {
    $$ = $1;
  }|
  ReturnStmt {
    $$ = $1;
  }|
  SelectStmt {
    $$ = $1;
  }|
  IterationStmt {
    $$ = $1;
  };

SelectStmt:
  IF LP Cond RP Stmt %prec LOWER_THEN_ELSE {
    $$ = new IfNode(NULL, pNode($3), pNode($5));
  }|
  IF LP Cond RP Stmt ELSE Stmt {
    $$ = new IfNode(NULL, pNode($3), pNode($5), pNode($7));
  };

IterationStmt:
  WHILE LP Cond RP Stmt {
    $$ = new WhileNode(NULL, pNode($3), pNode($5));
  };

ReturnStmt:
  RETURN Exp SEMICOLON {
    $$ = new ReturnNode(NULL, pNode($2));
  }|
  RETURN SEMICOLON {
    $$ = new ReturnNode(NULL);
  };

Exp:
  AddExp {
    $$ = $1;
  };

Cond:
  LOrExp {
    $$ = $1;
  };

LVal:
  ID {
    $$ = new SymNode(NULL, to_symbol(*$1));
    delete $1;
  }|
  ID ArrayIndexes {
    $$ = new ItemNode(NULL, new_sym_node(NULL, to_symbol(*$1)), *$2);
    delete $1;
    delete $2;
  };

PrimaryExp:
  LP Exp RP {
    $$ = $2;
  }|
  LVal {
    $$ = $1;
  }|
  Number {
    $$ = $1;
  };


Number:
  INT {
    $$ = new ImmNode(NULL, ImmValue($1));
  }|
  FLOAT {
    $$ = new ImmNode(NULL, ImmValue($1));
  };

UnaryExp:
  PrimaryExp {
    $$ = $1;
  }|
  Call {
    $$ = $1;
  }|
  UnaryOp UnaryExp {
    $$ = new UnaryNode(NULL, $1, pNode($2));
  };


Call:
  ID LP RP {
    $$ = new CallNode(NULL, to_symbol(*$1), {});
    delete $1;
  }|
  ID LP Args RP {
    $$ = new CallNode(NULL, to_symbol(*$1), *$3);
    delete $1;
    delete $3;
  };

UnaryOp:
  ADD {
    $$ = OPR_POS;
  }|
  MINUS {
    $$ = OPR_NEG;
  }|
  NOT {
    $$ = OPR_NOT;
  };

Args:
  Exp {
    $$ = new Vector<pNode>();
    $$->push_back(pNode($1));
  }|
  Args COMMA Exp {
    $$ = $1;
    $$->push_back(pNode($3));
  };

MulExp:
  UnaryExp {
    $$ = $1;
  }|
  MulExp MUL UnaryExp {
    $$ = new BinaryNode(NULL, OPR_MUL, pNode($1), pNode($3));
  }|
  MulExp DIV UnaryExp {
    $$ = new BinaryNode(NULL, OPR_DIV, pNode($1), pNode($3));
  }|
  MulExp MOD UnaryExp {
    $$ = new BinaryNode(NULL, OPR_REM, pNode($1), pNode($3));
  };

AddExp:
  MulExp {
    $$ = $1;
  }|
  AddExp ADD MulExp {
    $$ = new BinaryNode(NULL, OPR_ADD, pNode($1), pNode($3));
  }|
  AddExp MINUS MulExp {
    $$ = new BinaryNode(NULL, OPR_SUB, pNode($1), pNode($3));
  };

RelExp:
  AddExp {
    $$ = $1;
  }|
  RelExp GTE AddExp {
    $$ = new BinaryNode(NULL, OPR_GE, pNode($1), pNode($3));
  }|
  RelExp LTE AddExp {
    $$ = new BinaryNode(NULL, OPR_LE, pNode($1), pNode($3));
  }|
  RelExp GT AddExp {
    $$ = new BinaryNode(NULL, OPR_GT, pNode($1), pNode($3));
  }|
  RelExp LT AddExp {
    $$ = new BinaryNode(NULL, OPR_LT, pNode($1), pNode($3));
  };

EqExp:
  RelExp {
    $$ = $1;
  }|
  EqExp EQ RelExp {
    $$ = new BinaryNode(NULL, OPR_EQ, pNode($1), pNode($3));
  }|
  EqExp NEQ RelExp {
    $$ = new BinaryNode(NULL, OPR_NE, pNode($1), pNode($3));
  };

LAndExp:
  EqExp {
    $$ = $1;
  }|
  LAndExp AND EqExp {
    $$ = new BinaryNode(NULL, OPR_AND, pNode($1), pNode($3));
  };

LOrExp:
  LAndExp {
    $$ = $1;
  }|
  LOrExp OR LAndExp {
    $$ = new BinaryNode(NULL, OPR_OR, pNode($1), pNode($3));
  };
%%

void setFileName(const char *name) {
  strcpy(filename, name);
  freopen(filename, "r", stdin);
}

void yyerror(const char* fmt) {
  printf("%s:%d ", filename, yylineno);
  printf("%s\n", fmt);
}
