#include "SemanticAnalyzer.h"
#include "../../support/symbol-table/IdSet.h"
#include <stdlib.h>
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownSemanticAnalyzerModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: SemanticAnalyzer...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSemanticAnalyzerModule() {
	_logger = createLogger("SemanticAnalyzer");
	return _shutdownSemanticAnalyzerModule;
}

/* ANALYSIS STATE (per run) */

typedef struct GraphInfo {
	char * id;             /* not owned: points into the AST */
	GraphKind kind;
	GraphTraits traits;
	IdSet * nodes;         /* owned */
	IdSet * groups;        /* owned */
	struct GraphInfo * next;
} GraphInfo;

typedef struct {
	IdSet * graphIds;      /* global: declared + derived graph ids */
	IdSet * analysisIds;   /* global: analysis ids */
	GraphInfo * graphs;
	int errors;
} Analyzer;

static GraphInfo * _findGraph(Analyzer * a, const char * id) {
	for (GraphInfo * g = a->graphs; g != NULL; g = g->next) {
		if (strcmp(g->id, id) == 0) {
			return g;
		}
	}
	return NULL;
}

static GraphInfo * _registerGraph(Analyzer * a, char * id, GraphKind kind, GraphTraits traits) {
	GraphInfo * info = calloc(1, sizeof(GraphInfo));
	info->id = id;
	info->kind = kind;
	info->traits = traits;
	info->nodes = createIdSet();
	info->groups = createIdSet();
	info->next = a->graphs;
	a->graphs = info;
	return info;
}

static void _destroyGraphs(Analyzer * a) {
	for (GraphInfo * g = a->graphs; g != NULL; ) {
		GraphInfo * next = g->next;
		destroyIdSet(g->nodes);
		destroyIdSet(g->groups);
		free(g);
		g = next;
	}
	a->graphs = NULL;
}

static void _addIdToSet(const char * id, void * context) {
	idSetAdd((IdSet *) context, id);
}

static void _copyNodeIds(GraphInfo * dst, GraphInfo * src) {
	idSetForEach(src->nodes, _addIdToSet, dst->nodes);
}

static void _copyGroupIds(GraphInfo * dst, GraphInfo * src) {
	idSetForEach(src->groups, _addIdToSet, dst->groups);
}

static void _error(Analyzer * a, const char * format, const char * arg) {
	logError(_logger, format, arg);
	a->errors++;
}

static void _checkConstraints(Analyzer * a, GraphDecl * g, GraphInfo * info) {
	for (ConstraintList * it = g->constraints; it != NULL; it = it->next) {
		Constraint * c = it->value;
		switch (c->type) {
			case CONSTRAINT_SIMPLE:
				if (c->simple == SIMPLE_STRONGLY_CONNECTED
						&& g->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'strongly_connected' requires a directed graph: '%s'.", g->id);
				}
				if (c->simple == SIMPLE_CONNECTED
						&& g->kind != GRAPH_KIND_UNDIRECTED) {
					_error(a, "'connected' requires an undirected graph: '%s'.", g->id);
				}
				break;
			case CONSTRAINT_REACHABLE:
				if (!idSetContains(info->nodes, c->reachable.from)) {
					_error(a, "Constraint references undeclared node: '%s'.", c->reachable.from);
				}
				if (!idSetContains(info->nodes, c->reachable.to)) {
					_error(a, "Constraint references undeclared node: '%s'.", c->reachable.to);
				}
				break;
			case CONSTRAINT_TREE:
				if (g->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'tree' requires a directed graph: '%s'.", g->id);
				}
				if (!idSetContains(info->nodes, c->tree.root)) {
					_error(a, "tree 'rooted_at' references undeclared node: '%s'.", c->tree.root);
				}
				break;
			case CONSTRAINT_BINARY_TREE:
				if (g->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'binary_tree' requires a directed graph: '%s'.", g->id);
				}
				if (!idSetContains(info->nodes, c->binaryTree.root)) {
					_error(a, "binary_tree 'rooted_at' references undeclared node: '%s'.",
						c->binaryTree.root);
				}
				break;
			case CONSTRAINT_FORALL:
				if (!idSetContains(info->groups, c->forall.group)) {
					_error(a, "forall references undeclared group: '%s'.", c->forall.group);
				}
				if (strcmp(c->forall.predicate->var, c->forall.var) != 0) {
					_error(a, "forall predicate variable must be the bound variable: '%s'.",
						c->forall.var);
				}
				break;
		}
	}
}

static void _checkGraphScope(Analyzer * a, GraphDecl * g) {
	GraphInfo * info = _registerGraph(a, g->id, g->kind, g->traits);
	if (g->nodes == NULL) {
		_error(a, "Graph '%s' declares no nodes.", g->id);
	}
	int rootCount = 0, sourceCount = 0, sinkCount = 0;
	for (NodeDeclList * it = g->nodes; it != NULL; it = it->next) {
		if (!idSetAdd(info->nodes, it->value->id)) {
			_error(a, "Duplicate node identifier: '%s'.", it->value->id);
		}
		switch (it->value->attr) {
			case NODE_ATTR_ROOT: rootCount++; break;
			case NODE_ATTR_SOURCE:
				sourceCount++;
				if (g->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "Node attribute 'source' requires a directed graph: '%s'.", g->id);
				}
				break;
			case NODE_ATTR_SINK:
				sinkCount++;
				if (g->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "Node attribute 'sink' requires a directed graph: '%s'.", g->id);
				}
				break;
			default: break;
		}
	}
	if (rootCount > 1) {
		_error(a, "Graph '%s' declares multiple 'root' nodes.", g->id);
	}
	if (sourceCount > 1) {
		_error(a, "Graph '%s' declares multiple 'source' nodes.", g->id);
	}
	if (sinkCount > 1) {
		_error(a, "Graph '%s' declares multiple 'sink' nodes.", g->id);
	}
	for (GroupDeclList * it = g->groups; it != NULL; it = it->next) {
		if (!idSetAdd(info->groups, it->value->name)) {
			_error(a, "Duplicate group identifier: '%s'.", it->value->name);
		}
	}
	for (EdgeDeclList * it = g->edges; it != NULL; it = it->next) {
		EdgeDecl * e = it->value;
		if (!idSetContains(info->nodes, e->from)) {
			_error(a, "Edge references undeclared node: '%s'.", e->from);
		}
		if (!idSetContains(info->nodes, e->to)) {
			_error(a, "Edge references undeclared node: '%s'.", e->to);
		}
		if (e->hasWeight && !g->traits.weighted) {
			_error(a, "Edge uses 'weight' but graph '%s' is not weighted.", g->id);
		}
		if (e->hasCapacity && !g->traits.capacitated) {
			_error(a, "Edge uses 'capacity' but graph '%s' is not capacitated.", g->id);
		}
		if (e->op == EDGE_OP_DIRECTED && g->kind != GRAPH_KIND_DIRECTED) {
			_error(a, "Directed edge '->' in undirected graph '%s'.", g->id);
		}
		if (e->op == EDGE_OP_UNDIRECTED && g->kind != GRAPH_KIND_UNDIRECTED) {
			_error(a, "Undirected edge '--' in directed graph '%s'.", g->id);
		}
	}
	for (GroupDeclList * it = g->groups; it != NULL; it = it->next) {
		for (IdList * m = it->value->members; m != NULL; m = m->next) {
			if (!idSetContains(info->nodes, m->value)) {
				_error(a, "Group references undeclared node: '%s'.", m->value);
			}
		}
	}
	_checkConstraints(a, g, info);
}

static void _checkAlgorithm(Analyzer * a, GraphInfo * target, Algorithm * algo) {
	bool directed = target->kind == GRAPH_KIND_DIRECTED;
	switch (algo->type) {
		case ALGO_TOPOLOGICAL_SORT:
			if (!directed) {
				_error(a, "'topological_sort' requires a directed graph: '%s'.", target->id);
			}
			break;
		case ALGO_SCC:
			if (!directed) {
				_error(a, "'scc' requires a directed graph: '%s'.", target->id);
			}
			break;
		case ALGO_COMPONENTS:
			if (directed) {
				_error(a, "'components' requires an undirected graph: '%s'.", target->id);
			}
			break;
		case ALGO_MST:
			if (directed) {
				_error(a, "'mst' requires an undirected graph: '%s'.", target->id);
			}
			if (!target->traits.weighted) {
				_error(a, "'mst' requires a weighted graph: '%s'.", target->id);
			}
			break;
		case ALGO_MAX_FLOW:
			if (!directed) {
				_error(a, "'max_flow' requires a directed graph: '%s'.", target->id);
			}
			if (!target->traits.capacitated) {
				_error(a, "'max_flow' requires a capacitated graph: '%s'.", target->id);
			}
			break;
		case ALGO_SHORTEST_PATH:
			if (!target->traits.weighted) {
				_error(a, "'shortest_path' requires a weighted graph: '%s'.", target->id);
			}
			break;
	}
}

static void _checkAnalysisScope(Analyzer * a, AnalysisDecl * an) {
	GraphInfo * target = _findGraph(a, an->onGraphId);
	if (target == NULL) {
		_error(a, "Analysis references undeclared graph: '%s'.", an->onGraphId);
	}
	if (an->statements == NULL) {
		_error(a, "Analysis '%s' has no statements.", an->id);
	}
	IdSet * results = createIdSet();
	for (AnalysisStmtList * it = an->statements; it != NULL; it = it->next) {
		AnalysisStmt * stmt = it->value;
		if (stmt->type == ANALYSIS_STMT_RUN) {
			RunStmt * run = stmt->run;
			if (!idSetAdd(results, run->resultId)) {
				_error(a, "Duplicate result identifier: '%s'.", run->resultId);
			}
			if (target != NULL) {
				Algorithm * algo = run->algorithm;
				if (algo->from != NULL && !idSetContains(target->nodes, algo->from)) {
					_error(a, "Run references undeclared node: '%s'.", algo->from);
				}
				if (algo->to != NULL && !idSetContains(target->nodes, algo->to)) {
					_error(a, "Run references undeclared node: '%s'.", algo->to);
				}
				_checkAlgorithm(a, target, run->algorithm);
			}
		}
		else {
			ExportStmt * ex = stmt->exportStmt;
			if (ex->targetType == EXPORT_TARGET_RESULT && !idSetContains(results, ex->resultId)) {
				_error(a, "Export references undeclared result: '%s'.", ex->resultId);
			}
		}
	}
	destroyIdSet(results);
}

static void _checkTopLevel(Analyzer * a, TopLevelDecl * decl) {
	switch (decl->type) {
		case TOP_LEVEL_GRAPH: {
			GraphDecl * g = decl->graphDecl;
			if (!idSetAdd(a->graphIds, g->id)) {
				_error(a, "Duplicate graph identifier: '%s'.", g->id);
			}
			_checkGraphScope(a, g);
			break;
		}
		case TOP_LEVEL_DERIVE: {
			DeriveDecl * d = decl->deriveDecl;
			if (!idSetAdd(a->graphIds, d->newId)) {
				_error(a, "Duplicate graph identifier: '%s'.", d->newId);
			}
			GraphInfo * src = _findGraph(a, d->fromId);
			if (src == NULL) {
				_error(a, "Derive references undeclared graph: '%s'.", d->fromId);
			}
			else {
				GraphKind kind = (d->transformation->type == TRANSFORMATION_UNDERLYING)
					? GRAPH_KIND_UNDIRECTED : src->kind;
				GraphInfo * derived = _registerGraph(a, d->newId, kind, src->traits);
				_copyNodeIds(derived, src);
				_copyGroupIds(derived, src);
				if (d->transformation->type == TRANSFORMATION_TRANSPOSE
						&& src->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'transpose' requires a directed source graph: '%s'.", d->fromId);
				}
				if (d->transformation->type == TRANSFORMATION_UNDERLYING
						&& src->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'underlying' requires a directed source graph: '%s'.", d->fromId);
				}
				if (d->transformation->type == TRANSFORMATION_INDUCED_SUBGRAPH
						&& !idSetContains(src->groups, d->transformation->group)) {
					_error(a, "induced_subgraph references undeclared group: '%s'.",
						d->transformation->group);
				}
			}
			break;
		}
		case TOP_LEVEL_ANALYSIS: {
			AnalysisDecl * an = decl->analysisDecl;
			if (!idSetAdd(a->analysisIds, an->id)) {
				_error(a, "Duplicate analysis identifier: '%s'.", an->id);
			}
			_checkAnalysisScope(a, an);
			break;
		}
	}
}

/* PUBLIC FUNCTIONS */

SemanticResult executeSemanticAnalysis(CompilerState * compilerState) {
	Program * program = (Program *) compilerState->abstractSyntaxtTree;
	SemanticResult result = { .succeeded = false };
	if (program == NULL) {
		logError(_logger, "Semantic analysis received a null AST.");
		return result;
	}
	Analyzer a = {
		.graphIds = createIdSet(),
		.analysisIds = createIdSet(),
		.graphs = NULL,
		.errors = 0
	};
	for (TopLevelDeclList * it = program->decls; it != NULL; it = it->next) {
		_checkTopLevel(&a, it->value);
	}
	_destroyGraphs(&a);
	destroyIdSet(a.graphIds);
	destroyIdSet(a.analysisIds);
	if (a.errors == 0) {
		logDebugging(_logger, "Semantic analysis passed.");
		result.succeeded = true;
	}
	else {
		logError(_logger, "Semantic analysis rejected the program.");
	}
	return result;
}
