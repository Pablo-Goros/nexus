%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"
#include <stdlib.h>

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */

	signed int integer;
	char * string;
	TokenLabel token;

	/** Non-terminals. */

	Constant * constant;
	Expression * expression;
	Factor * factor;
	Program * program;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { destroyConstant($$); } <constant>
%destructor { destroyExpression($$); } <expression>
%destructor { destroyFactor($$); } <factor>
%destructor { free($$); } <string>

/** Terminals. */
%token <string> IDENTIFIER
%token <integer> INTEGER
%token <token> ADD
%token <token> ANALYSIS
%token <token> AS
%token <token> ASSERT
%token <token> ASSIGN
%token <token> ACYCLIC
%token <token> BINARY_TREE
%token <token> CAPACITATED
%token <token> CAPACITY
%token <token> CLOSE_BRACE
%token <token> CLOSE_PARENTHESIS
%token <token> COLON
%token <token> COMMA
%token <token> CONNECTED
%token <token> CONSTRAINTS
%token <token> DEGREE
%token <token> DERIVE
%token <token> DIRECTED
%token <token> DIRECTED_EDGE
%token <token> DIV
%token <token> DOT
%token <token> EDGES
%token <token> EXPORT
%token <token> COMPONENTS
%token <token> FORALL
%token <token> FROM
%token <token> GRAPH
%token <token> GREATER_EQUAL
%token <token> GREATER_THAN
%token <token> GROUPS
%token <token> IN
%token <token> INDEGREE
%token <token> INDUCED_SUBGRAPH
%token <token> JSON
%token <token> KIND
%token <token> LESS_EQUAL
%token <token> LESS_THAN
%token <token> MAX_FLOW
%token <token> MUL
%token <token> MST
%token <token> NODES
%token <token> NOT_EQUAL
%token <token> ON
%token <token> OPEN_BRACE
%token <token> OPEN_PARENTHESIS
%token <token> OUTDEGREE
%token <token> REACHABLE
%token <token> REMOVE_SELF_LOOPS
%token <token> RESULT
%token <token> ROOT
%token <token> ROOTED_AT
%token <token> RUN
%token <token> SCC
%token <token> SHORTEST_PATH
%token <token> SINK
%token <token> SOURCE
%token <token> STRONGLY_CONNECTED
%token <token> SUB
%token <token> TERMINAL
%token <token> TO
%token <token> TOPOLOGICAL_SORT
%token <token> TRAITS
%token <token> TRANSPOSE
%token <token> TREE
%token <token> UNDIRECTED
%token <token> UNDIRECTED_EDGE
%token <token> UNDERLYING
%token <token> USING
%token <token> WEIGHT
%token <token> WEIGHTED

%token <token> IGNORED
%token <token> UNKNOWN

/** Non-terminals. */
%type <constant> constant
%type <expression> expression
%type <factor> factor
%type <program> program

/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */
%left ADD SUB
%left MUL DIV

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program: expression											{ $$ = ExpressionProgramSemanticAction($1); }
	;

expression: expression[left] ADD expression[right]			{ $$ = ArithmeticExpressionSemanticAction($left, $right, ADDITION); }
	| expression[left] DIV expression[right]				{ $$ = ArithmeticExpressionSemanticAction($left, $right, DIVISION); }
	| expression[left] MUL expression[right]				{ $$ = ArithmeticExpressionSemanticAction($left, $right, MULTIPLICATION); }
	| expression[left] SUB expression[right]				{ $$ = ArithmeticExpressionSemanticAction($left, $right, SUBTRACTION); }
	| factor												{ $$ = FactorExpressionSemanticAction($1); }
	;

factor: OPEN_PARENTHESIS expression CLOSE_PARENTHESIS		{ $$ = ExpressionFactorSemanticAction($2); }
	| constant												{ $$ = ConstantFactorSemanticAction($1); }
	;

constant: INTEGER											{ $$ = IntegerConstantSemanticAction($1); }
	;

%%
