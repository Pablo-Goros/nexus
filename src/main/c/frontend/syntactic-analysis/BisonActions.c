#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
static void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PROGRAM */

Program * ProgramSemanticAction(TopLevelDeclList * decls) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelDeclList * ordered = reverseTopLevelDeclList(decls);
	Program * program = createProgram(ordered);
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

TopLevelDeclList * EmptyTopLevelDeclListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

TopLevelDeclList * AppendTopLevelDeclSemanticAction(TopLevelDeclList * list, TopLevelDecl * decl) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependTopLevelDecl(list, decl);
}

TopLevelDecl * GraphTopLevelDeclSemanticAction(GraphDecl * graphDecl) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTopLevelGraphDecl(graphDecl);
}

TopLevelDecl * DeriveTopLevelDeclSemanticAction(DeriveDecl * deriveDecl) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTopLevelDeriveDecl(deriveDecl);
}

TopLevelDecl * AnalysisTopLevelDeclSemanticAction(AnalysisDecl * analysisDecl) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTopLevelAnalysisDecl(analysisDecl);
}

/* GRAPH */

GraphKind DirectedKindSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return GRAPH_KIND_DIRECTED;
}

GraphKind UndirectedKindSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return GRAPH_KIND_UNDIRECTED;
}

GraphTraits EmptyTraitsSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createGraphTraits(false, false);
}

GraphTraits TraitsSemanticAction(bool weighted, bool capacitated) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createGraphTraits(weighted, capacitated);
}

GraphTraits AddWeightedTraitSemanticAction(GraphTraits traits) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	traits.weighted = true;
	return traits;
}

GraphTraits AddCapacitatedTraitSemanticAction(GraphTraits traits) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	traits.capacitated = true;
	return traits;
}

NodeAttr EmptyNodeAttrSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NODE_ATTR_NONE;
}

NodeAttr RootNodeAttrSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NODE_ATTR_ROOT;
}

NodeAttr SourceNodeAttrSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NODE_ATTR_SOURCE;
}

NodeAttr SinkNodeAttrSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NODE_ATTR_SINK;
}

NodeAttr TerminalNodeAttrSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NODE_ATTR_TERMINAL;
}

NodeDeclList * EmptyNodeDeclListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

NodeDeclList * AppendNodeDeclSemanticAction(NodeDeclList * list, NodeDecl * decl) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependNodeDecl(list, decl);
}

NodeDecl * NodeDeclSemanticAction(char * id, NodeAttr attr) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createNodeDecl(id, attr);
}

EdgeOp DirectedEdgeOpSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return EDGE_OP_DIRECTED;
}

EdgeOp UndirectedEdgeOpSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return EDGE_OP_UNDIRECTED;
}

EdgeDeclList * EmptyEdgeDeclListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

EdgeDeclList * AppendEdgeDeclSemanticAction(EdgeDeclList * list, EdgeDecl * decl) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependEdgeDecl(list, decl);
}

EdgeDecl * EdgeDeclSemanticAction(char * from, EdgeOp op, char * to, bool hasWeight, int weight, bool hasCapacity, int capacity) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createEdgeDecl(from, op, to, hasWeight, weight, hasCapacity, capacity);
}

IdList * SingleIdListSemanticAction(char * id) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependId(NULL, id);
}

IdList * AppendIdListSemanticAction(IdList * list, char * id) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependId(list, id);
}

GroupDeclList * EmptyGroupDeclListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

GroupDeclList * AppendGroupDeclSemanticAction(GroupDeclList * list, GroupDecl * decl) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependGroupDecl(list, decl);
}

GroupDecl * GroupDeclSemanticAction(char * name, IdList * members) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IdList * ordered = reverseIdList(members);
	return createGroupDecl(name, ordered);
}

ConstraintList * EmptyConstraintListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

ConstraintList * AppendConstraintSemanticAction(ConstraintList * list, Constraint * constraint) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependConstraint(list, constraint);
}

Constraint * SimpleConnectedConstraintSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createSimpleConstraint(SIMPLE_CONNECTED);
}

Constraint * SimpleStronglyConnectedConstraintSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createSimpleConstraint(SIMPLE_STRONGLY_CONNECTED);
}

Constraint * SimpleAcyclicConstraintSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createSimpleConstraint(SIMPLE_ACYCLIC);
}

Constraint * ReachableConstraintSemanticAction(char * from, char * to) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createReachableConstraint(from, to);
}

Constraint * TreeConstraintSemanticAction(char * root) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTreeConstraint(root);
}

Constraint * BinaryTreeConstraintSemanticAction(char * root) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createBinaryTreeConstraint(root);
}

Constraint * ForallConstraintSemanticAction(char * var, char * group, Predicate * predicate) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createForallConstraint(var, group, predicate);
}

DegreeFn IndegreeFnSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return DEGREE_FN_INDEGREE;
}

DegreeFn OutdegreeFnSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return DEGREE_FN_OUTDEGREE;
}

DegreeFn DegreeFnSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return DEGREE_FN_DEGREE;
}

Comparator EqComparatorSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return CMP_EQ;
}

Comparator NeqComparatorSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return CMP_NEQ;
}

Comparator GeqComparatorSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return CMP_GEQ;
}

Comparator LeqComparatorSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return CMP_LEQ;
}

Comparator GtComparatorSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return CMP_GT;
}

Comparator LtComparatorSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return CMP_LT;
}

Predicate * PredicateSemanticAction(DegreeFn fn, char * var, Comparator cmp, int rhs) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createPredicate(fn, var, cmp, rhs);
}

Transformation * TransposeTransformationSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTransformation(TRANSFORMATION_TRANSPOSE, NULL);
}

Transformation * UnderlyingTransformationSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTransformation(TRANSFORMATION_UNDERLYING, NULL);
}

Transformation * RemoveSelfLoopsTransformationSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTransformation(TRANSFORMATION_REMOVE_SELF_LOOPS, NULL);
}

Transformation * InducedSubgraphTransformationSemanticAction(char * group) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createTransformation(TRANSFORMATION_INDUCED_SUBGRAPH, group);
}

DeriveDecl * DeriveDeclSemanticAction(char * newId, char * fromId, Transformation * transformation) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createDeriveDecl(newId, fromId, transformation);
}

Algorithm * ShortestPathAlgorithmSemanticAction(char * from, char * to) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createAlgorithm(ALGO_SHORTEST_PATH, from, to);
}

Algorithm * TopologicalSortAlgorithmSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createAlgorithm(ALGO_TOPOLOGICAL_SORT, NULL, NULL);
}

Algorithm * ComponentsAlgorithmSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createAlgorithm(ALGO_COMPONENTS, NULL, NULL);
}

Algorithm * SccAlgorithmSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createAlgorithm(ALGO_SCC, NULL, NULL);
}

Algorithm * MstAlgorithmSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createAlgorithm(ALGO_MST, NULL, NULL);
}

Algorithm * MaxFlowAlgorithmSemanticAction(char * from, char * to) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createAlgorithm(ALGO_MAX_FLOW, from, to);
}

AnalysisStmtList * EmptyAnalysisStmtListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL;
}

AnalysisStmtList * AppendAnalysisStmtSemanticAction(AnalysisStmtList * list, AnalysisStmt * stmt) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return prependAnalysisStmt(list, stmt);
}

RunStmt * RunStmtSemanticAction(Algorithm * algorithm, char * resultId) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createRunStmt(algorithm, resultId);
}

AnalysisStmt * RunAnalysisStmtSemanticAction(RunStmt * runStmt) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createRunAnalysisStmt(runStmt);
}

ExportStmt * ExportGraphStmtSemanticAction(ExportFormat format) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createExportGraphStmt(format);
}

ExportStmt * ExportResultStmtSemanticAction(char * resultId, ExportFormat format) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createExportResultStmt(resultId, format);
}

AnalysisStmt * ExportAnalysisStmtSemanticAction(ExportStmt * exportStmt) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return createExportAnalysisStmt(exportStmt);
}

ExportFormat DotFormatSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return EXPORT_FORMAT_DOT;
}

ExportFormat JsonFormatSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return EXPORT_FORMAT_JSON;
}

AnalysisDecl * AnalysisDeclSemanticAction(char * id, char * onGraphId, AnalysisStmtList * statements) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	AnalysisStmtList * ordered = reverseAnalysisStmtList(statements);
	return createAnalysisDecl(id, onGraphId, ordered);
}

GraphDecl * GraphDeclSemanticAction(char * id, GraphKind kind, GraphTraits traits, NodeDeclList * nodes, EdgeDeclList * edges, GroupDeclList * groups, ConstraintList * constraints) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	NodeDeclList * orderedNodes = reverseNodeDeclList(nodes);
	EdgeDeclList * orderedEdges = reverseEdgeDeclList(edges);
	GroupDeclList * orderedGroups = reverseGroupDeclList(groups);
	ConstraintList * orderedConstraints = reverseConstraintList(constraints);
	return createGraphDecl(id, kind, traits, orderedNodes, orderedEdges, orderedGroups, orderedConstraints);
}
