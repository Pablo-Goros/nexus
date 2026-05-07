%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"
#include <stdlib.h>

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
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

	/**
	 * Non-terminals.
	 *
	 * Stage II Step 1: introduce the Stage I Nexus grammar first.
	 * The Nexus AST and semantic actions are wired in on Step 2.
	 */
	void * node;
}

/** Destructors. */
%destructor { free($$); } <string>

/** Terminals. */
%token <string> IDENTIFIER
%token <integer> INTEGER

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
%type <node> program
%type <node> top_level_decls
%type <node> top_level_decl

%type <node> graph_decl
%type <node> kind_value
%type <node> traits_section_opt
%type <node> traits_list
%type <node> trait
%type <node> nodes_section
%type <node> node_decls_opt
%type <node> node_decls_opt_nonempty
%type <node> node_decl
%type <node> node_attr_opt
%type <node> edges_section
%type <node> edge_decls_opt
%type <node> edge_decls_opt_nonempty
%type <node> edge_decl
%type <node> edge_operator
%type <node> edge_attr_opt
%type <node> weight_attr_opt
%type <node> capacity_attr_opt
%type <node> groups_section_opt
%type <node> group_decls_opt
%type <node> group_decls_opt_nonempty
%type <node> group_decl
%type <node> id_list
%type <node> id_list_tail
%type <node> constraints_section_opt
%type <node> constraint_decls_opt
%type <node> constraint_decls_opt_nonempty
%type <node> constraint_decl
%type <node> simple_constraint
%type <node> reachable_constraint
%type <node> tree_constraint
%type <node> binary_tree_constraint
%type <node> forall_constraint
%type <node> predicate
%type <node> degree_fn
%type <node> comparator

%type <node> derive_decl
%type <node> transformation

%type <node> analysis_decl
%type <node> analysis_stmts_opt
%type <node> analysis_stmts_opt_nonempty
%type <node> analysis_stmt
%type <node> run_stmt
%type <node> algorithm
%type <node> export_stmt
%type <node> export_target
%type <node> export_format

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program
	: top_level_decls											{ $$ = $1; }
	;

top_level_decls
	: top_level_decl											{ $$ = $1; }
	| top_level_decls top_level_decl							{ $$ = $1; }
	;

top_level_decl
	: graph_decl												{ $$ = $1; }
	| derive_decl												{ $$ = $1; }
	| analysis_decl											{ $$ = $1; }
	;

// Graph declarations (fixed section order: kind, optional traits, nodes, edges, optional groups, optional constraints)

graph_decl
	: GRAPH IDENTIFIER COLON KIND kind_value traits_section_opt nodes_section edges_section groups_section_opt constraints_section_opt
																	{ $$ = NULL; }
	;

kind_value
	: DIRECTED												{ $$ = NULL; }
	| UNDIRECTED											{ $$ = NULL; }
	;

traits_section_opt
	: %empty												{ $$ = NULL; }
	| TRAITS traits_list										{ $$ = NULL; }
	;

traits_list
	: trait													{ $$ = NULL; }
	| traits_list trait										{ $$ = NULL; }
	;

trait
	: WEIGHTED												{ $$ = NULL; }
	| CAPACITATED											{ $$ = NULL; }
	;

nodes_section
	: NODES COLON node_decls_opt									{ $$ = NULL; }
	;

node_decls_opt
	: %empty												{ $$ = NULL; }
	| node_decls_opt_nonempty									{ $$ = NULL; }
	;

node_decls_opt_nonempty
	: node_decl												{ $$ = NULL; }
	| node_decls_opt_nonempty node_decl								{ $$ = NULL; }
	;

node_decl
	: IDENTIFIER node_attr_opt									{ $$ = NULL; }
	;

node_attr_opt
	: %empty												{ $$ = NULL; }
	| ROOT													{ $$ = NULL; }
	| SOURCE												{ $$ = NULL; }
	| SINK													{ $$ = NULL; }
	| TERMINAL											{ $$ = NULL; }
	;

edges_section
	: EDGES COLON edge_decls_opt									{ $$ = NULL; }
	;

edge_decls_opt
	: %empty												{ $$ = NULL; }
	| edge_decls_opt_nonempty									{ $$ = NULL; }
	;

edge_decls_opt_nonempty
	: edge_decl												{ $$ = NULL; }
	| edge_decls_opt_nonempty edge_decl								{ $$ = NULL; }
	;

edge_decl
	: IDENTIFIER edge_operator IDENTIFIER edge_attr_opt					{ $$ = NULL; }
	;

edge_operator
	: DIRECTED_EDGE											{ $$ = NULL; }
	| UNDIRECTED_EDGE										{ $$ = NULL; }
	;

// Fixed order: optional weight first, then optional capacity.
edge_attr_opt
	: weight_attr_opt capacity_attr_opt							{ $$ = NULL; }
	;

weight_attr_opt
	: %empty												{ $$ = NULL; }
	| WEIGHT INTEGER										{ $$ = NULL; }
	;

capacity_attr_opt
	: %empty												{ $$ = NULL; }
	| CAPACITY INTEGER										{ $$ = NULL; }
	;

groups_section_opt
	: %empty												{ $$ = NULL; }
	| GROUPS COLON group_decls_opt								{ $$ = NULL; }
	;

group_decls_opt
	: %empty												{ $$ = NULL; }
	| group_decls_opt_nonempty									{ $$ = NULL; }
	;

group_decls_opt_nonempty
	: group_decl												{ $$ = NULL; }
	| group_decls_opt_nonempty group_decl								{ $$ = NULL; }
	;

group_decl
	: IDENTIFIER ASSIGN OPEN_BRACE id_list CLOSE_BRACE					{ $$ = NULL; }
	;

id_list
	: IDENTIFIER id_list_tail									{ $$ = NULL; }
	;

id_list_tail
	: %empty												{ $$ = NULL; }
	| id_list_tail COMMA IDENTIFIER								{ $$ = NULL; }
	;

constraints_section_opt
	: %empty												{ $$ = NULL; }
	| CONSTRAINTS COLON constraint_decls_opt							{ $$ = NULL; }
	;

constraint_decls_opt
	: %empty												{ $$ = NULL; }
	| constraint_decls_opt_nonempty									{ $$ = NULL; }
	;

constraint_decls_opt_nonempty
	: constraint_decl											{ $$ = NULL; }
	| constraint_decls_opt_nonempty constraint_decl							{ $$ = NULL; }
	;

constraint_decl
	: ASSERT simple_constraint									{ $$ = NULL; }
	| ASSERT reachable_constraint								{ $$ = NULL; }
	| ASSERT tree_constraint									{ $$ = NULL; }
	| ASSERT binary_tree_constraint							{ $$ = NULL; }
	| ASSERT forall_constraint								{ $$ = NULL; }
	;

simple_constraint
	: CONNECTED												{ $$ = NULL; }
	| STRONGLY_CONNECTED										{ $$ = NULL; }
	| ACYCLIC												{ $$ = NULL; }
	;

reachable_constraint
	: REACHABLE IDENTIFIER DIRECTED_EDGE IDENTIFIER					{ $$ = NULL; }
	;

tree_constraint
	: TREE ROOTED_AT IDENTIFIER								{ $$ = NULL; }
	;

binary_tree_constraint
	: BINARY_TREE ROOTED_AT IDENTIFIER							{ $$ = NULL; }
	;

forall_constraint
	: FORALL IDENTIFIER IN IDENTIFIER COLON predicate					{ $$ = NULL; }
	;

predicate
	: degree_fn OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS comparator INTEGER
																	{ $$ = NULL; }
	;

degree_fn
	: INDEGREE												{ $$ = NULL; }
	| OUTDEGREE											{ $$ = NULL; }
	| DEGREE												{ $$ = NULL; }
	;

comparator
	: ASSIGN													{ $$ = NULL; }
	| NOT_EQUAL												{ $$ = NULL; }
	| GREATER_EQUAL											{ $$ = NULL; }
	| LESS_EQUAL											{ $$ = NULL; }
	| GREATER_THAN											{ $$ = NULL; }
	| LESS_THAN												{ $$ = NULL; }
	;

// Derivations

derive_decl
	: DERIVE IDENTIFIER FROM IDENTIFIER USING transformation					{ $$ = NULL; }
	;

transformation
	: TRANSPOSE												{ $$ = NULL; }
	| UNDERLYING											{ $$ = NULL; }
	| REMOVE_SELF_LOOPS										{ $$ = NULL; }
	| INDUCED_SUBGRAPH OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS		{ $$ = NULL; }
	;

// Analysis blocks

analysis_decl
	: ANALYSIS IDENTIFIER ON IDENTIFIER COLON analysis_stmts_opt				{ $$ = NULL; }
	;

analysis_stmts_opt
	: %empty												{ $$ = NULL; }
	| analysis_stmts_opt_nonempty									{ $$ = NULL; }
	;

analysis_stmts_opt_nonempty
	: analysis_stmt											{ $$ = NULL; }
	| analysis_stmts_opt_nonempty analysis_stmt							{ $$ = NULL; }
	;

analysis_stmt
	: run_stmt												{ $$ = NULL; }
	| export_stmt											{ $$ = NULL; }
	;

run_stmt
	: RUN algorithm AS IDENTIFIER								{ $$ = NULL; }
	;

algorithm
	: SHORTEST_PATH FROM IDENTIFIER TO IDENTIFIER					{ $$ = NULL; }
	| TOPOLOGICAL_SORT										{ $$ = NULL; }
	| COMPONENTS											{ $$ = NULL; }
	| SCC													{ $$ = NULL; }
	| MST													{ $$ = NULL; }
	| MAX_FLOW FROM IDENTIFIER TO IDENTIFIER						{ $$ = NULL; }
	;

export_stmt
	: EXPORT export_target TO export_format							{ $$ = NULL; }
	;

export_target
	: GRAPH													{ $$ = NULL; }
	| RESULT IDENTIFIER										{ $$ = NULL; }
	;

export_format
	: DOT													{ $$ = NULL; }
	| JSON													{ $$ = NULL; }
	;

%%
