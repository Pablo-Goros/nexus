#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
static void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* LIST HELPERS */

#define DEFINE_REVERSE_LIST(FN_NAME, LIST_TYPE) \
	LIST_TYPE * FN_NAME(LIST_TYPE * list) { \
		LIST_TYPE * prev = NULL; \
		LIST_TYPE * cur = list; \
		while (cur != NULL) { \
			LIST_TYPE * next = cur->next; \
			cur->next = prev; \
			prev = cur; \
			cur = next; \
		} \
		return prev; \
	}

DEFINE_REVERSE_LIST(reverseTopLevelDeclList, TopLevelDeclList)
DEFINE_REVERSE_LIST(reverseNodeDeclList, NodeDeclList)
DEFINE_REVERSE_LIST(reverseEdgeDeclList, EdgeDeclList)
DEFINE_REVERSE_LIST(reverseGroupDeclList, GroupDeclList)
DEFINE_REVERSE_LIST(reverseConstraintList, ConstraintList)
DEFINE_REVERSE_LIST(reverseAnalysisStmtList, AnalysisStmtList)
DEFINE_REVERSE_LIST(reverseIdList, IdList)

/* CONSTRUCTORS */

Program * createProgram(TopLevelDeclList * decls) {
	Program * program = calloc(1, sizeof(Program));
	program->decls = decls;
	return program;
}

TopLevelDecl * createTopLevelGraphDecl(GraphDecl * graphDecl) {
	TopLevelDecl * decl = calloc(1, sizeof(TopLevelDecl));
	decl->type = TOP_LEVEL_GRAPH;
	decl->graphDecl = graphDecl;
	return decl;
}

TopLevelDecl * createTopLevelDeriveDecl(DeriveDecl * deriveDecl) {
	TopLevelDecl * decl = calloc(1, sizeof(TopLevelDecl));
	decl->type = TOP_LEVEL_DERIVE;
	decl->deriveDecl = deriveDecl;
	return decl;
}

TopLevelDecl * createTopLevelAnalysisDecl(AnalysisDecl * analysisDecl) {
	TopLevelDecl * decl = calloc(1, sizeof(TopLevelDecl));
	decl->type = TOP_LEVEL_ANALYSIS;
	decl->analysisDecl = analysisDecl;
	return decl;
}

TopLevelDeclList * prependTopLevelDecl(TopLevelDeclList * list, TopLevelDecl * value) {
	TopLevelDeclList * cell = calloc(1, sizeof(TopLevelDeclList));
	cell->value = value;
	cell->next = list;
	return cell;
}

GraphTraits createGraphTraits(bool weighted, bool capacitated) {
	GraphTraits traits = { .weighted = weighted, .capacitated = capacitated };
	return traits;
}

GraphDecl * createGraphDecl(char * id, GraphKind kind, GraphTraits traits, NodeDeclList * nodes, EdgeDeclList * edges, GroupDeclList * groups, ConstraintList * constraints) {
	GraphDecl * graphDecl = calloc(1, sizeof(GraphDecl));
	graphDecl->id = id;
	graphDecl->kind = kind;
	graphDecl->traits = traits;
	graphDecl->nodes = nodes;
	graphDecl->edges = edges;
	graphDecl->groups = groups;
	graphDecl->constraints = constraints;
	return graphDecl;
}

NodeDecl * createNodeDecl(char * id, NodeAttr attr) {
	NodeDecl * nodeDecl = calloc(1, sizeof(NodeDecl));
	nodeDecl->id = id;
	nodeDecl->attr = attr;
	return nodeDecl;
}

NodeDeclList * prependNodeDecl(NodeDeclList * list, NodeDecl * value) {
	NodeDeclList * cell = calloc(1, sizeof(NodeDeclList));
	cell->value = value;
	cell->next = list;
	return cell;
}

EdgeDecl * createEdgeDecl(char * from, EdgeOp op, char * to, bool hasWeight, int weight, bool hasCapacity, int capacity) {
	EdgeDecl * edgeDecl = calloc(1, sizeof(EdgeDecl));
	edgeDecl->from = from;
	edgeDecl->to = to;
	edgeDecl->op = op;
	edgeDecl->hasWeight = hasWeight;
	edgeDecl->weight = weight;
	edgeDecl->hasCapacity = hasCapacity;
	edgeDecl->capacity = capacity;
	return edgeDecl;
}

EdgeDeclList * prependEdgeDecl(EdgeDeclList * list, EdgeDecl * value) {
	EdgeDeclList * cell = calloc(1, sizeof(EdgeDeclList));
	cell->value = value;
	cell->next = list;
	return cell;
}

IdList * prependId(IdList * list, char * value) {
	IdList * cell = calloc(1, sizeof(IdList));
	cell->value = value;
	cell->next = list;
	return cell;
}

GroupDecl * createGroupDecl(char * name, IdList * members) {
	GroupDecl * groupDecl = calloc(1, sizeof(GroupDecl));
	groupDecl->name = name;
	groupDecl->members = members;
	return groupDecl;
}

GroupDeclList * prependGroupDecl(GroupDeclList * list, GroupDecl * value) {
	GroupDeclList * cell = calloc(1, sizeof(GroupDeclList));
	cell->value = value;
	cell->next = list;
	return cell;
}

Predicate * createPredicate(DegreeFn fn, char * var, Comparator cmp, int rhs) {
	Predicate * predicate = calloc(1, sizeof(Predicate));
	predicate->fn = fn;
	predicate->var = var;
	predicate->cmp = cmp;
	predicate->rhs = rhs;
	return predicate;
}

Constraint * createSimpleConstraint(SimpleConstraintType type) {
	Constraint * constraint = calloc(1, sizeof(Constraint));
	constraint->type = CONSTRAINT_SIMPLE;
	constraint->simple = type;
	return constraint;
}

Constraint * createReachableConstraint(char * from, char * to) {
	Constraint * constraint = calloc(1, sizeof(Constraint));
	constraint->type = CONSTRAINT_REACHABLE;
	constraint->reachable.from = from;
	constraint->reachable.to = to;
	return constraint;
}

Constraint * createTreeConstraint(char * root) {
	Constraint * constraint = calloc(1, sizeof(Constraint));
	constraint->type = CONSTRAINT_TREE;
	constraint->tree.root = root;
	return constraint;
}

Constraint * createBinaryTreeConstraint(char * root) {
	Constraint * constraint = calloc(1, sizeof(Constraint));
	constraint->type = CONSTRAINT_BINARY_TREE;
	constraint->binaryTree.root = root;
	return constraint;
}

Constraint * createForallConstraint(char * var, char * group, Predicate * predicate) {
	Constraint * constraint = calloc(1, sizeof(Constraint));
	constraint->type = CONSTRAINT_FORALL;
	constraint->forall.var = var;
	constraint->forall.group = group;
	constraint->forall.predicate = predicate;
	return constraint;
}

ConstraintList * prependConstraint(ConstraintList * list, Constraint * value) {
	ConstraintList * cell = calloc(1, sizeof(ConstraintList));
	cell->value = value;
	cell->next = list;
	return cell;
}

Transformation * createTransformation(TransformationType type, char * group) {
	Transformation * transformation = calloc(1, sizeof(Transformation));
	transformation->type = type;
	transformation->group = group;
	return transformation;
}

DeriveDecl * createDeriveDecl(char * newId, char * fromId, Transformation * transformation) {
	DeriveDecl * deriveDecl = calloc(1, sizeof(DeriveDecl));
	deriveDecl->newId = newId;
	deriveDecl->fromId = fromId;
	deriveDecl->transformation = transformation;
	return deriveDecl;
}

Algorithm * createAlgorithm(AlgorithmType type, char * from, char * to) {
	Algorithm * algorithm = calloc(1, sizeof(Algorithm));
	algorithm->type = type;
	algorithm->from = from;
	algorithm->to = to;
	return algorithm;
}

RunStmt * createRunStmt(Algorithm * algorithm, char * resultId) {
	RunStmt * runStmt = calloc(1, sizeof(RunStmt));
	runStmt->algorithm = algorithm;
	runStmt->resultId = resultId;
	return runStmt;
}

ExportStmt * createExportGraphStmt(ExportFormat format) {
	ExportStmt * exportStmt = calloc(1, sizeof(ExportStmt));
	exportStmt->targetType = EXPORT_TARGET_GRAPH;
	exportStmt->format = format;
	return exportStmt;
}

ExportStmt * createExportResultStmt(char * resultId, ExportFormat format) {
	ExportStmt * exportStmt = calloc(1, sizeof(ExportStmt));
	exportStmt->targetType = EXPORT_TARGET_RESULT;
	exportStmt->resultId = resultId;
	exportStmt->format = format;
	return exportStmt;
}

AnalysisStmt * createRunAnalysisStmt(RunStmt * runStmt) {
	AnalysisStmt * stmt = calloc(1, sizeof(AnalysisStmt));
	stmt->type = ANALYSIS_STMT_RUN;
	stmt->run = runStmt;
	return stmt;
}

AnalysisStmt * createExportAnalysisStmt(ExportStmt * exportStmt) {
	AnalysisStmt * stmt = calloc(1, sizeof(AnalysisStmt));
	stmt->type = ANALYSIS_STMT_EXPORT;
	stmt->exportStmt = exportStmt;
	return stmt;
}

AnalysisStmtList * prependAnalysisStmt(AnalysisStmtList * list, AnalysisStmt * value) {
	AnalysisStmtList * cell = calloc(1, sizeof(AnalysisStmtList));
	cell->value = value;
	cell->next = list;
	return cell;
}

AnalysisDecl * createAnalysisDecl(char * id, char * onGraphId, AnalysisStmtList * statements) {
	AnalysisDecl * analysisDecl = calloc(1, sizeof(AnalysisDecl));
	analysisDecl->id = id;
	analysisDecl->onGraphId = onGraphId;
	analysisDecl->statements = statements;
	return analysisDecl;
}

/* DESTRUCTORS */

void destroyNodeDecl(NodeDecl * nodeDecl) {
	if (nodeDecl == NULL) {
		return;
	}
	free(nodeDecl->id);
	free(nodeDecl);
}

void destroyNodeDeclList(NodeDeclList * list) {
	while (list != NULL) {
		NodeDeclList * next = list->next;
		destroyNodeDecl(list->value);
		free(list);
		list = next;
	}
}

void destroyEdgeDecl(EdgeDecl * edgeDecl) {
	if (edgeDecl == NULL) {
		return;
	}
	free(edgeDecl->from);
	free(edgeDecl->to);
	free(edgeDecl);
}

void destroyEdgeDeclList(EdgeDeclList * list) {
	while (list != NULL) {
		EdgeDeclList * next = list->next;
		destroyEdgeDecl(list->value);
		free(list);
		list = next;
	}
}

void destroyIdList(IdList * list) {
	while (list != NULL) {
		IdList * next = list->next;
		free(list->value);
		free(list);
		list = next;
	}
}

void destroyGroupDecl(GroupDecl * groupDecl) {
	if (groupDecl == NULL) {
		return;
	}
	free(groupDecl->name);
	destroyIdList(groupDecl->members);
	free(groupDecl);
}

void destroyGroupDeclList(GroupDeclList * list) {
	while (list != NULL) {
		GroupDeclList * next = list->next;
		destroyGroupDecl(list->value);
		free(list);
		list = next;
	}
}

void destroyPredicate(Predicate * predicate) {
	if (predicate == NULL) {
		return;
	}
	free(predicate->var);
	free(predicate);
}

void destroyConstraint(Constraint * constraint) {
	if (constraint == NULL) {
		return;
	}
	switch (constraint->type) {
		case CONSTRAINT_SIMPLE:
			break;
		case CONSTRAINT_REACHABLE:
			free(constraint->reachable.from);
			free(constraint->reachable.to);
			break;
		case CONSTRAINT_TREE:
			free(constraint->tree.root);
			break;
		case CONSTRAINT_BINARY_TREE:
			free(constraint->binaryTree.root);
			break;
		case CONSTRAINT_FORALL:
			free(constraint->forall.var);
			free(constraint->forall.group);
			destroyPredicate(constraint->forall.predicate);
			break;
	}
	free(constraint);
}

void destroyConstraintList(ConstraintList * list) {
	while (list != NULL) {
		ConstraintList * next = list->next;
		destroyConstraint(list->value);
		free(list);
		list = next;
	}
}

void destroyTransformation(Transformation * transformation) {
	if (transformation == NULL) {
		return;
	}
	free(transformation->group);
	free(transformation);
}

void destroyDeriveDecl(DeriveDecl * deriveDecl) {
	if (deriveDecl == NULL) {
		return;
	}
	free(deriveDecl->newId);
	free(deriveDecl->fromId);
	destroyTransformation(deriveDecl->transformation);
	free(deriveDecl);
}

void destroyAlgorithm(Algorithm * algorithm) {
	if (algorithm == NULL) {
		return;
	}
	free(algorithm->from);
	free(algorithm->to);
	free(algorithm);
}

void destroyRunStmt(RunStmt * runStmt) {
	if (runStmt == NULL) {
		return;
	}
	destroyAlgorithm(runStmt->algorithm);
	free(runStmt->resultId);
	free(runStmt);
}

void destroyExportStmt(ExportStmt * exportStmt) {
	if (exportStmt == NULL) {
		return;
	}
	free(exportStmt->resultId);
	free(exportStmt);
}

void destroyAnalysisStmt(AnalysisStmt * stmt) {
	if (stmt == NULL) {
		return;
	}
	switch (stmt->type) {
		case ANALYSIS_STMT_RUN:
			destroyRunStmt(stmt->run);
			break;
		case ANALYSIS_STMT_EXPORT:
			destroyExportStmt(stmt->exportStmt);
			break;
	}
	free(stmt);
}

void destroyAnalysisStmtList(AnalysisStmtList * list) {
	while (list != NULL) {
		AnalysisStmtList * next = list->next;
		destroyAnalysisStmt(list->value);
		free(list);
		list = next;
	}
}

void destroyAnalysisDecl(AnalysisDecl * analysisDecl) {
	if (analysisDecl == NULL) {
		return;
	}
	free(analysisDecl->id);
	free(analysisDecl->onGraphId);
	destroyAnalysisStmtList(analysisDecl->statements);
	free(analysisDecl);
}

void destroyGraphDecl(GraphDecl * graphDecl) {
	if (graphDecl == NULL) {
		return;
	}
	free(graphDecl->id);
	destroyNodeDeclList(graphDecl->nodes);
	destroyEdgeDeclList(graphDecl->edges);
	destroyGroupDeclList(graphDecl->groups);
	destroyConstraintList(graphDecl->constraints);
	free(graphDecl);
}

void destroyTopLevelDecl(TopLevelDecl * decl) {
	if (decl == NULL) {
		return;
	}
	switch (decl->type) {
		case TOP_LEVEL_GRAPH:
			destroyGraphDecl(decl->graphDecl);
			break;
		case TOP_LEVEL_DERIVE:
			destroyDeriveDecl(decl->deriveDecl);
			break;
		case TOP_LEVEL_ANALYSIS:
			destroyAnalysisDecl(decl->analysisDecl);
			break;
	}
	free(decl);
}

void destroyTopLevelDeclList(TopLevelDeclList * list) {
	while (list != NULL) {
		TopLevelDeclList * next = list->next;
		destroyTopLevelDecl(list->value);
		free(list);
		list = next;
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program == NULL) {
		return;
	}
	destroyTopLevelDeclList(program->decls);
	free(program);
}
