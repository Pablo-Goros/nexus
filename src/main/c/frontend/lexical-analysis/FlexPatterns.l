%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

/**
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus IntegerLexemeAction();
extern CompilationStatus TokenLexemeAction(TokenLabel label);
extern CompilationStatus UnknownLexemeAction();

%}

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
whitespace			[[:space:]]+
line_comment		"//"[^\n]*
identifier			[A-Za-z_][A-Za-z0-9_]*
integer				[[:digit:]]+

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

"graph"								{ return TokenLexemeAction(GRAPH); }
"kind"								{ return TokenLexemeAction(KIND); }
"traits"							{ return TokenLexemeAction(TRAITS); }
"nodes"								{ return TokenLexemeAction(NODES); }
"edges"								{ return TokenLexemeAction(EDGES); }
"groups"							{ return TokenLexemeAction(GROUPS); }
"constraints"						{ return TokenLexemeAction(CONSTRAINTS); }
"assert"							{ return TokenLexemeAction(ASSERT); }
"derive"							{ return TokenLexemeAction(DERIVE); }
"from"								{ return TokenLexemeAction(FROM); }
"using"								{ return TokenLexemeAction(USING); }
"analysis"							{ return TokenLexemeAction(ANALYSIS); }
"on"								{ return TokenLexemeAction(ON); }
"run"								{ return TokenLexemeAction(RUN); }
"as"								{ return TokenLexemeAction(AS); }
"export"							{ return TokenLexemeAction(EXPORT); }
"to"								{ return TokenLexemeAction(TO); }
"result"							{ return TokenLexemeAction(RESULT); }
"directed"							{ return TokenLexemeAction(DIRECTED); }
"undirected"						{ return TokenLexemeAction(UNDIRECTED); }
"weighted"							{ return TokenLexemeAction(WEIGHTED); }
"capacitated"						{ return TokenLexemeAction(CAPACITATED); }
"connected"						{ return TokenLexemeAction(CONNECTED); }
"strongly_connected"				{ return TokenLexemeAction(STRONGLY_CONNECTED); }
"acyclic"							{ return TokenLexemeAction(ACYCLIC); }
"reachable"						{ return TokenLexemeAction(REACHABLE); }
"tree"								{ return TokenLexemeAction(TREE); }
"rooted_at"							{ return TokenLexemeAction(ROOTED_AT); }
"binary_tree"						{ return TokenLexemeAction(BINARY_TREE); }
"forall"							{ return TokenLexemeAction(FORALL); }
"in"								{ return TokenLexemeAction(IN); }
"transpose"							{ return TokenLexemeAction(TRANSPOSE); }
"induced_subgraph"					{ return TokenLexemeAction(INDUCED_SUBGRAPH); }
"remove_self_loops"					{ return TokenLexemeAction(REMOVE_SELF_LOOPS); }
"underlying"						{ return TokenLexemeAction(UNDERLYING); }
"shortest_path"						{ return TokenLexemeAction(SHORTEST_PATH); }
"topological_sort"					{ return TokenLexemeAction(TOPOLOGICAL_SORT); }
"components"						{ return TokenLexemeAction(COMPONENTS); }
"scc"								{ return TokenLexemeAction(SCC); }
"mst"								{ return TokenLexemeAction(MST); }
"max_flow"							{ return TokenLexemeAction(MAX_FLOW); }
"root"								{ return TokenLexemeAction(ROOT); }
"source"							{ return TokenLexemeAction(SOURCE); }
"sink"								{ return TokenLexemeAction(SINK); }
"terminal"							{ return TokenLexemeAction(TERMINAL); }
"dot"								{ return TokenLexemeAction(DOT); }
"json"								{ return TokenLexemeAction(JSON); }
"weight"							{ return TokenLexemeAction(WEIGHT); }
"capacity"							{ return TokenLexemeAction(CAPACITY); }
"indegree"							{ return TokenLexemeAction(INDEGREE); }
"outdegree"							{ return TokenLexemeAction(OUTDEGREE); }
"degree"							{ return TokenLexemeAction(DEGREE); }

"->"								{ return TokenLexemeAction(DIRECTED_EDGE); }
"--"								{ return TokenLexemeAction(UNDIRECTED_EDGE); }
"!="								{ return TokenLexemeAction(NOT_EQUAL); }
">="								{ return TokenLexemeAction(GREATER_EQUAL); }
"<="								{ return TokenLexemeAction(LESS_EQUAL); }
":"									{ return TokenLexemeAction(COLON); }
"{"									{ return TokenLexemeAction(OPEN_BRACE); }
"}"									{ return TokenLexemeAction(CLOSE_BRACE); }
"("									{ return TokenLexemeAction(OPEN_PARENTHESIS); }
")"									{ return TokenLexemeAction(CLOSE_PARENTHESIS); }
","									{ return TokenLexemeAction(COMMA); }
"="									{ return TokenLexemeAction(ASSIGN); }
"<"									{ return TokenLexemeAction(LESS_THAN); }
">"									{ return TokenLexemeAction(GREATER_THAN); }

{integer}							{ return IntegerLexemeAction(); }
{identifier}						{ return IdentifierLexemeAction(); }

{line_comment}						{ return IgnoredLexemeAction(); }
{whitespace}						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
