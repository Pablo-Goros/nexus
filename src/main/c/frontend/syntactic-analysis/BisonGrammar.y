%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"
#include <stdbool.h>
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

	/** Non-terminals. */
	Program * program;
	TopLevelDecl * topLevelDecl;
	TopLevelDeclList * topLevelDeclList;

	GraphDecl * graphDecl;
	DeriveDecl * deriveDecl;
	AnalysisDecl * analysisDecl;

	GraphKind graphKind;
	GraphTraits graphTraits;

	NodeAttr nodeAttr;
	NodeDecl * nodeDecl;
	NodeDeclList * nodeDeclList;

	EdgeOp edgeOp;
	EdgeDecl * edgeDecl;
	EdgeDeclList * edgeDeclList;
	bool boolean;
	struct {
		bool present;
		int value;
	} intOpt;

	IdList * idList;

	GroupDecl * groupDecl;
	GroupDeclList * groupDeclList;

	Constraint * constraint;
	ConstraintList * constraintList;

	DegreeFn degreeFn;
	Comparator comparator;
	Predicate * predicate;

	Transformation * transformation;

	Algorithm * algorithm;
	RunStmt * runStmt;
	ExportStmt * exportStmt;
	AnalysisStmt * analysisStmt;
	AnalysisStmtList * analysisStmtList;
	ExportFormat exportFormat;
}

/** Destructors. */
%destructor { free($$); } <string>

%destructor { destroyProgram($$); } <program>
%destructor { destroyTopLevelDecl($$); } <topLevelDecl>
%destructor { destroyTopLevelDeclList($$); } <topLevelDeclList>

%destructor { destroyGraphDecl($$); } <graphDecl>
%destructor { destroyDeriveDecl($$); } <deriveDecl>
%destructor { destroyAnalysisDecl($$); } <analysisDecl>

%destructor { destroyNodeDecl($$); } <nodeDecl>
%destructor { destroyNodeDeclList($$); } <nodeDeclList>

%destructor { destroyEdgeDecl($$); } <edgeDecl>
%destructor { destroyEdgeDeclList($$); } <edgeDeclList>

%destructor { destroyIdList($$); } <idList>

%destructor { destroyGroupDecl($$); } <groupDecl>
%destructor { destroyGroupDeclList($$); } <groupDeclList>

%destructor { destroyConstraint($$); } <constraint>
%destructor { destroyConstraintList($$); } <constraintList>

%destructor { destroyPredicate($$); } <predicate>

%destructor { destroyTransformation($$); } <transformation>

%destructor { destroyAlgorithm($$); } <algorithm>
%destructor { destroyRunStmt($$); } <runStmt>
%destructor { destroyExportStmt($$); } <exportStmt>
%destructor { destroyAnalysisStmt($$); } <analysisStmt>
%destructor { destroyAnalysisStmtList($$); } <analysisStmtList>

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
%type <program> program
%type <topLevelDeclList> top_level_decls
%type <topLevelDecl> top_level_decl

%type <graphDecl> graph_decl
%type <graphKind> kind_value
%type <graphTraits> traits_section_opt
%type <graphTraits> traits_list

%type <nodeDeclList> nodes_section
%type <nodeDeclList> node_decls_opt
%type <nodeDeclList> node_decls_opt_nonempty
%type <nodeDecl> node_decl
%type <nodeAttr> node_attr_opt

%type <edgeDeclList> edges_section
%type <edgeDeclList> edge_decls_opt
%type <edgeDeclList> edge_decls_opt_nonempty
%type <edgeDecl> edge_decl
%type <edgeOp> edge_operator
%type <intOpt> weight_attr_opt
%type <intOpt> capacity_attr_opt

%type <groupDeclList> groups_section_opt
%type <groupDeclList> group_decls_opt
%type <groupDeclList> group_decls_opt_nonempty
%type <groupDecl> group_decl
%type <idList> id_list
%type <idList> id_list_tail

%type <constraintList> constraints_section_opt
%type <constraintList> constraint_decls_opt
%type <constraintList> constraint_decls_opt_nonempty
%type <constraint> constraint_decl
%type <constraint> simple_constraint
%type <constraint> reachable_constraint
%type <constraint> tree_constraint
%type <constraint> binary_tree_constraint
%type <constraint> forall_constraint
%type <predicate> predicate
%type <degreeFn> degree_fn
%type <comparator> comparator

%type <deriveDecl> derive_decl
%type <transformation> transformation

%type <analysisDecl> analysis_decl
%type <analysisStmtList> analysis_stmts_opt
%type <analysisStmtList> analysis_stmts_opt_nonempty
%type <analysisStmt> analysis_stmt
%type <runStmt> run_stmt
%type <algorithm> algorithm
%type <exportStmt> export_stmt
%type <exportFormat> export_format

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program
	: top_level_decls
																{ $$ = ProgramSemanticAction($1); }
	;

top_level_decls
	: %empty
																{ $$ = EmptyTopLevelDeclListSemanticAction(); }
	| top_level_decls top_level_decl
																{ $$ = AppendTopLevelDeclSemanticAction($1, $2); }
	;

top_level_decl
	: graph_decl
																{ $$ = GraphTopLevelDeclSemanticAction($1); }
	| derive_decl
																{ $$ = DeriveTopLevelDeclSemanticAction($1); }
	| analysis_decl
																{ $$ = AnalysisTopLevelDeclSemanticAction($1); }
	;

// Graph declarations (fixed section order: kind, optional traits, nodes, edges, optional groups, optional constraints)

graph_decl
	: GRAPH IDENTIFIER COLON KIND kind_value traits_section_opt nodes_section edges_section groups_section_opt constraints_section_opt
																	{ $$ = GraphDeclSemanticAction($2, $5, $6, $7, $8, $9, $10); }
	;

kind_value
	: DIRECTED												{ $$ = DirectedKindSemanticAction(); }
	| UNDIRECTED											{ $$ = UndirectedKindSemanticAction(); }
	;

traits_section_opt
	: %empty												{ $$ = EmptyTraitsSemanticAction(); }
	| TRAITS traits_list										{ $$ = $2; }
	;

traits_list
	: WEIGHTED												{ $$ = AddWeightedTraitSemanticAction(EmptyTraitsSemanticAction()); }
	| CAPACITATED											{ $$ = AddCapacitatedTraitSemanticAction(EmptyTraitsSemanticAction()); }
	| traits_list WEIGHTED									{ $$ = AddWeightedTraitSemanticAction($1); }
	| traits_list CAPACITATED								{ $$ = AddCapacitatedTraitSemanticAction($1); }
	;

nodes_section
	: NODES COLON node_decls_opt									{ $$ = $3; }
	;

node_decls_opt
	: %empty												{ $$ = EmptyNodeDeclListSemanticAction(); }
	| node_decls_opt_nonempty									{ $$ = $1; }
	;

node_decls_opt_nonempty
	: node_decl												{ $$ = AppendNodeDeclSemanticAction(EmptyNodeDeclListSemanticAction(), $1); }
	| node_decls_opt_nonempty node_decl								{ $$ = AppendNodeDeclSemanticAction($1, $2); }
	;

node_decl
	: IDENTIFIER node_attr_opt									{ $$ = NodeDeclSemanticAction($1, $2); }
	;

node_attr_opt
	: %empty												{ $$ = EmptyNodeAttrSemanticAction(); }
	| ROOT													{ $$ = RootNodeAttrSemanticAction(); }
	| SOURCE												{ $$ = SourceNodeAttrSemanticAction(); }
	| SINK													{ $$ = SinkNodeAttrSemanticAction(); }
	| TERMINAL											{ $$ = TerminalNodeAttrSemanticAction(); }
	;

edges_section
	: EDGES COLON edge_decls_opt									{ $$ = $3; }
	;

edge_decls_opt
	: %empty												{ $$ = EmptyEdgeDeclListSemanticAction(); }
	| edge_decls_opt_nonempty									{ $$ = $1; }
	;

edge_decls_opt_nonempty
	: edge_decl												{ $$ = AppendEdgeDeclSemanticAction(EmptyEdgeDeclListSemanticAction(), $1); }
	| edge_decls_opt_nonempty edge_decl								{ $$ = AppendEdgeDeclSemanticAction($1, $2); }
	;

edge_decl
	: IDENTIFIER edge_operator IDENTIFIER weight_attr_opt capacity_attr_opt
																{ $$ = EdgeDeclSemanticAction($1, $2, $3, $4.present, $4.value, $5.present, $5.value); }
	;

edge_operator
	: DIRECTED_EDGE											{ $$ = DirectedEdgeOpSemanticAction(); }
	| UNDIRECTED_EDGE										{ $$ = UndirectedEdgeOpSemanticAction(); }
	;

// Fixed order: optional weight first, then optional capacity.
weight_attr_opt
	: %empty												{ $$.present = false; $$.value = 0; }
	| WEIGHT INTEGER										{ $$.present = true; $$.value = $2; }
	;

capacity_attr_opt
	: %empty												{ $$.present = false; $$.value = 0; }
	| CAPACITY INTEGER										{ $$.present = true; $$.value = $2; }
	;

groups_section_opt
	: %empty												{ $$ = EmptyGroupDeclListSemanticAction(); }
	| GROUPS COLON group_decls_opt								{ $$ = $3; }
	;

group_decls_opt
	: %empty												{ $$ = EmptyGroupDeclListSemanticAction(); }
	| group_decls_opt_nonempty									{ $$ = $1; }
	;

group_decls_opt_nonempty
	: group_decl												{ $$ = AppendGroupDeclSemanticAction(EmptyGroupDeclListSemanticAction(), $1); }
	| group_decls_opt_nonempty group_decl								{ $$ = AppendGroupDeclSemanticAction($1, $2); }
	;

group_decl
	: IDENTIFIER ASSIGN OPEN_BRACE id_list CLOSE_BRACE					{ $$ = GroupDeclSemanticAction($1, $4); }
	;

id_list
	: IDENTIFIER id_list_tail									{ $$ = AppendIdListSemanticAction($2, $1); }
	;

id_list_tail
	: %empty												{ $$ = NULL; }
	| id_list_tail COMMA IDENTIFIER								{ $$ = AppendIdListSemanticAction($1, $3); }
	;

constraints_section_opt
	: %empty												{ $$ = EmptyConstraintListSemanticAction(); }
	| CONSTRAINTS COLON constraint_decls_opt							{ $$ = $3; }
	;

constraint_decls_opt
	: %empty												{ $$ = EmptyConstraintListSemanticAction(); }
	| constraint_decls_opt_nonempty									{ $$ = $1; }
	;

constraint_decls_opt_nonempty
	: constraint_decl											{ $$ = AppendConstraintSemanticAction(EmptyConstraintListSemanticAction(), $1); }
	| constraint_decls_opt_nonempty constraint_decl							{ $$ = AppendConstraintSemanticAction($1, $2); }
	;

constraint_decl
	: ASSERT simple_constraint									{ $$ = $2; }
	| ASSERT reachable_constraint								{ $$ = $2; }
	| ASSERT tree_constraint									{ $$ = $2; }
	| ASSERT binary_tree_constraint							{ $$ = $2; }
	| ASSERT forall_constraint								{ $$ = $2; }
	;

simple_constraint
	: CONNECTED												{ $$ = SimpleConnectedConstraintSemanticAction(); }
	| STRONGLY_CONNECTED										{ $$ = SimpleStronglyConnectedConstraintSemanticAction(); }
	| ACYCLIC												{ $$ = SimpleAcyclicConstraintSemanticAction(); }
	;

reachable_constraint
	: REACHABLE IDENTIFIER DIRECTED_EDGE IDENTIFIER					{ $$ = ReachableConstraintSemanticAction($2, $4); }
	;

tree_constraint
	: TREE ROOTED_AT IDENTIFIER								{ $$ = TreeConstraintSemanticAction($3); }
	;

binary_tree_constraint
	: BINARY_TREE ROOTED_AT IDENTIFIER							{ $$ = BinaryTreeConstraintSemanticAction($3); }
	;

forall_constraint
	: FORALL IDENTIFIER IN IDENTIFIER COLON predicate					{ $$ = ForallConstraintSemanticAction($2, $4, $6); }
	;

predicate
	: degree_fn OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS comparator INTEGER
																	{ $$ = PredicateSemanticAction($1, $3, $5, $6); }
	;

degree_fn
	: INDEGREE												{ $$ = IndegreeFnSemanticAction(); }
	| OUTDEGREE											{ $$ = OutdegreeFnSemanticAction(); }
	| DEGREE												{ $$ = DegreeFnSemanticAction(); }
	;

comparator
	: ASSIGN													{ $$ = EqComparatorSemanticAction(); }
	| NOT_EQUAL												{ $$ = NeqComparatorSemanticAction(); }
	| GREATER_EQUAL											{ $$ = GeqComparatorSemanticAction(); }
	| LESS_EQUAL											{ $$ = LeqComparatorSemanticAction(); }
	| GREATER_THAN											{ $$ = GtComparatorSemanticAction(); }
	| LESS_THAN												{ $$ = LtComparatorSemanticAction(); }
	;

// Derivations

derive_decl
	: DERIVE IDENTIFIER FROM IDENTIFIER USING transformation					{ $$ = DeriveDeclSemanticAction($2, $4, $6); }
	;

transformation
	: TRANSPOSE												{ $$ = TransposeTransformationSemanticAction(); }
	| UNDERLYING											{ $$ = UnderlyingTransformationSemanticAction(); }
	| REMOVE_SELF_LOOPS										{ $$ = RemoveSelfLoopsTransformationSemanticAction(); }
	| INDUCED_SUBGRAPH OPEN_PARENTHESIS IDENTIFIER CLOSE_PARENTHESIS		{ $$ = InducedSubgraphTransformationSemanticAction($3); }
	;

// Analysis blocks

analysis_decl
	: ANALYSIS IDENTIFIER ON IDENTIFIER COLON analysis_stmts_opt				{ $$ = AnalysisDeclSemanticAction($2, $4, $6); }
	;

analysis_stmts_opt
	: %empty												{ $$ = EmptyAnalysisStmtListSemanticAction(); }
	| analysis_stmts_opt_nonempty									{ $$ = $1; }
	;

analysis_stmts_opt_nonempty
	: analysis_stmt											{ $$ = AppendAnalysisStmtSemanticAction(EmptyAnalysisStmtListSemanticAction(), $1); }
	| analysis_stmts_opt_nonempty analysis_stmt							{ $$ = AppendAnalysisStmtSemanticAction($1, $2); }
	;

analysis_stmt
	: run_stmt												{ $$ = RunAnalysisStmtSemanticAction($1); }
	| export_stmt											{ $$ = ExportAnalysisStmtSemanticAction($1); }
	;

run_stmt
	: RUN algorithm AS IDENTIFIER								{ $$ = RunStmtSemanticAction($2, $4); }
	;

algorithm
	: SHORTEST_PATH FROM IDENTIFIER TO IDENTIFIER					{ $$ = ShortestPathAlgorithmSemanticAction($3, $5); }
	| TOPOLOGICAL_SORT										{ $$ = TopologicalSortAlgorithmSemanticAction(); }
	| COMPONENTS											{ $$ = ComponentsAlgorithmSemanticAction(); }
	| SCC													{ $$ = SccAlgorithmSemanticAction(); }
	| MST													{ $$ = MstAlgorithmSemanticAction(); }
	| MAX_FLOW FROM IDENTIFIER TO IDENTIFIER						{ $$ = MaxFlowAlgorithmSemanticAction($3, $5); }
	;

export_stmt
	: EXPORT GRAPH TO export_format								{ $$ = ExportGraphStmtSemanticAction($4); }
	| EXPORT RESULT IDENTIFIER TO export_format						{ $$ = ExportResultStmtSemanticAction($3, $5); }
	;

export_format
	: DOT													{ $$ = DotFormatSemanticAction(); }
	| JSON													{ $$ = JsonFormatSemanticAction(); }
	;

%%
