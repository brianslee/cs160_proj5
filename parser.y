%{
    #include <cstdlib>
    #include <cstdio>
    #include <iostream>
    #include "ast.hpp"
    
    #define YYDEBUG 1
    #define YYINITDEPTH 10000
    
    int yylex(void);
    void yyerror(const char *);
    
    extern ASTNode* astRoot;
%}

%error-verbose
// %glr-parser
/* NOTE: You may use the %glr-parser directive, which may allow your parser to
         work even with some shift/reduce conflicts remianing. */

/* WRITEME: Copy your token and precedence specifiers from Project 4 here. */
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE
%token T_GREATER_THAN T_GREATER_THAN_EQUALS T_EQUALS
%token T_AND T_OR T_NOT //T_UNARY_MINUS
%token T_OPEN_BRACKET T_CLOSE_BRACKET
%token T_OPEN_PAREN T_CLOSE_PAREN
%token T_RETURN_TYPE T_ASSIGNMENT
%token T_DOT T_SEMICOLON T_COMMA
%token T_IF T_ELSE T_WHILE T_DO
%token T_PRINT T_RETURN T_INTEGER T_BOOLEAN
%token T_NONE T_TRUE T_FALSE
%token T_NEW T_EXTENDS
%token T_INT T_IDENTIFIER

/* WRITEME: Specify precedence here */
%left T_OR
%left T_AND
%left T_GREATER_THAN T_GREATER_THAN_EQUALS T_EQUALS
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE
%precedence T_NOT T_UNARY_MINUS

/* WRITEME: Copy your type specifiers from Project 4 here. */
%type <program_ptr> Start
%type <class_list_ptr> ClassList
%type <class_ptr> Class
%type <identifier_ptr> Identifier
%type <declaration_list_ptr> DeclarList
%type <method_list_ptr> MemMethList
%type <method_ptr> MethodDec
%type <expression_list_ptr> Arguments
%type <type_ptr> Type
%type <methodbody_ptr> MethodBod
%type <statement_list_ptr> StateList
%type <returnstatement_ptr> Return
%type <parameter_ptr> Param
%type <parameter_list_ptr> ParaList
%type <identifier_list_ptr> VariableDec
%type <expression_ptr> Expression
%type <assignment_ptr> Assignment
%type <memberaccess_ptr> Call
%type <methodcall_ptr> MethodCall
%type <ifelse_ptr> IfElse
%type <while_ptr> While
%type <dowhile_ptr> DoWhile
%type <print_ptr> Print

%type <statement_ptr> Statement



%type <negation_ptr> T_UNARY_MINUS
%type <memberaccess_ptr> T_DOT

%type <booleanliteral_ptr> T_TRUE T_FALSE
%type <new_ptr> T_NEW
%type <integertype_ptr> T_INTEGER
%type <booleantype_ptr> T_BOOLEAN
%type <none_ptr> T_NONE

%%

/* WRITEME: This rule is a placeholder. Replace it with your grammar
            rules and actions from Project 4. */
            
Start       :   ClassList { $$ = new ProgramNode($1);   astRoot = $$; }
            ;

/* WRITME: Write your Bison grammar specification here */
ClassList   :   ClassList Class { $$ = $1; $$->push_back($2); }
            |   Class { $$ = new std::list<ClassNode*>(); $$->push_back($1); }
            ;
Class       :   Identifier T_OPEN_BRACKET DeclarList MemMethList T_CLOSE_BRACKET { $$ = new ClassNode($1, NULL, $3, $4); }
            |   Identifier T_EXTENDS Identifier T_OPEN_BRACKET DeclarList MemMethList T_CLOSE_BRACKET { $$ = new ClassNode($1, $3, $5, $6);}
            ;

Type        :   T_INTEGER { $$ = new IntegerTypeNode(); }
            |   T_BOOLEAN { $$ = new BooleanTypeNode(); }
            |   Identifier { $$ = new ObjectTypeNode($1); }
            |   T_NONE { $$ = new NoneNode(); }
            ;

DeclarList  :   DeclarList Type VariableDec T_SEMICOLON { $$ = $1; $$->push_back(new DeclarationNode($2,$3)); }
            |   %empty { $$ = new std::list<DeclarationNode*>(); }
            ;

MemMethList :   MethodDec MemMethList  { $$ = $2, $$->push_front($1); }
            |   %empty { $$ = new std::list<MethodNode*>(); }
            ;

MethodDec   :   Identifier T_OPEN_PAREN ParaList T_CLOSE_PAREN T_RETURN_TYPE Type T_OPEN_BRACKET MethodBod T_CLOSE_BRACKET { $$ = new MethodNode($1, $3, $6, $8);}
            |   Identifier T_OPEN_PAREN T_CLOSE_PAREN T_RETURN_TYPE Type T_OPEN_BRACKET MethodBod T_CLOSE_BRACKET { $$ = new MethodNode($1, NULL, $5, $7); }
            ;
            
VariableDec :   Identifier { $$ = new std::list<IdentifierNode*>(); $$->push_back($1); }
            |   VariableDec T_COMMA Identifier { $$ = $1; $$->push_back($3); }
            ;

Identifier  :   T_IDENTIFIER { $$ = new IdentifierNode(yylval.base_char_ptr); }
            ;
            
Call        :   Identifier T_DOT Identifier { $$ = new MemberAccessNode($1,$3); }
            ;
            
Expression  :   Expression T_PLUS Expression  { $$ = new PlusNode($1,$3); }
            |   Expression T_MINUS Expression  { $$ = new MinusNode($1,$3); }
            |   Expression T_MULTIPLY Expression { $$ = new TimesNode($1, $3); }
            |   Expression T_DIVIDE Expression { $$ = new DivideNode($1, $3); }
            |   Expression T_GREATER_THAN Expression { $$ = new GreaterNode($1, $3); }
            |   Expression T_GREATER_THAN_EQUALS Expression { $$ = new GreaterEqualNode($1, $3); }
            |   Expression T_EQUALS Expression { $$ = new EqualNode($1, $3); }
            |   Expression T_AND Expression { $$ = new AndNode($1, $3); }
            |   Expression T_OR Expression { $$ = new OrNode($1, $3); }
            |   T_NOT Expression { $$ = new NotNode($2); }
            |   T_MINUS Expression %prec T_UNARY_MINUS { $$ = new NegationNode($2); }
            |   Identifier { $$ = new VariableNode($1); }
            |   Call { $$ = $1; }
            |   MethodCall { $$ = $1; }
            |   T_OPEN_PAREN Expression T_CLOSE_PAREN { $$ = $2; }
            |   T_INT { $$ = new IntegerLiteralNode(new IntegerNode(yylval.base_int)); }
            |   T_TRUE { $$ = new BooleanLiteralNode(new IntegerNode(1)); }
            |   T_FALSE { $$ = new BooleanLiteralNode(new IntegerNode(0)); }
            |   T_NEW Identifier { $$ = new NewNode($2, NULL); }
            |   T_NEW Identifier T_OPEN_PAREN Arguments T_CLOSE_PAREN { $$ = new NewNode($2, $4); }
            |   T_NEW Identifier T_OPEN_PAREN T_CLOSE_PAREN { $$ = new NewNode($2, NULL); }
            ;

MethodCall  :   Identifier T_OPEN_PAREN Arguments T_CLOSE_PAREN { $$ = new MethodCallNode($1, NULL, $3); }
            |   Identifier T_DOT Identifier T_OPEN_PAREN Arguments T_CLOSE_PAREN { $$ = new MethodCallNode($1, $3, $5); }
            |   Identifier T_OPEN_PAREN T_CLOSE_PAREN { $$ = new MethodCallNode($1, NULL, NULL); }
            |   Identifier T_DOT Identifier T_OPEN_PAREN T_CLOSE_PAREN { $$ = new MethodCallNode($1, $3, NULL); }
            ;

Arguments   :   Arguments T_COMMA Expression { $$ = $1; $$->push_back($3); }
            |   Expression { $$ = new std::list<ExpressionNode*>(); $$->push_back($1); }
            ;

            
Return      :   T_RETURN Expression T_SEMICOLON { $$ = new ReturnStatementNode($2); }
            ;

ParaList    :   ParaList T_COMMA Param { $$ = $1; $$->push_back($3); }
            |   Param { $$ = new std::list<ParameterNode*>(); $$->push_back($1); }
            ;
            
Param       :   Type Identifier { $$ = new ParameterNode($1, $2);  }
            ;

MethodBod   :   DeclarList StateList Return { $$ = new MethodBodyNode($1, $2, $3); }
            |   DeclarList Return { $$ = new MethodBodyNode($1, NULL, $2); }
            |   DeclarList StateList { $$ = new MethodBodyNode($1, $2, NULL); }
            |   DeclarList { $$ = new MethodBodyNode($1, NULL, NULL); }
            ;
            
StateList   :   StateList Statement  { $$ = $1; $$->push_back($2); }
            |   Statement { $$ = new std::list<StatementNode*>(); $$->push_back($1);}
            ;

Statement   :   Assignment { $$ = $1; }
            |   MethodCall T_SEMICOLON  {$$ = new CallNode($1); }
            |   IfElse { $$ = $1; }
            |   While { $$ = $1; }
            |   DoWhile { $$ = $1; }
            |   Print {$$ = $1; }
            ;

Assignment  :   Identifier T_ASSIGNMENT Expression T_SEMICOLON { $$ = new AssignmentNode($1, NULL, $3); }
            |   Identifier T_DOT Identifier T_ASSIGNMENT Expression T_SEMICOLON { $$ = new AssignmentNode($1, $3, $5); }
            ;

IfElse      :   T_IF Expression T_OPEN_BRACKET StateList T_CLOSE_BRACKET { $$ = new IfElseNode($2, $4, NULL); }
            |   T_IF Expression T_OPEN_BRACKET StateList T_CLOSE_BRACKET T_ELSE T_OPEN_BRACKET StateList T_CLOSE_BRACKET { $$ = new IfElseNode($2, $4, $8); }
            ;
            
While       :   T_WHILE Expression T_OPEN_BRACKET StateList T_CLOSE_BRACKET { $$ = new WhileNode($2, $4); }
            ;

DoWhile     :   T_DO T_OPEN_BRACKET StateList T_CLOSE_BRACKET T_WHILE T_OPEN_PAREN Expression T_CLOSE_PAREN T_SEMICOLON { $$ = new DoWhileNode($3, $7); }
            ;

Print       :   T_PRINT Expression T_SEMICOLON { $$ = new PrintNode($2); }
            ;

%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(1);
}