#include "SemanticAnalyzer.h"
#include "../../support/symbol-table/IdSet.h"

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

typedef struct {
	IdSet * graphIds;      /* global: declared + derived graph ids */
	IdSet * analysisIds;   /* global: analysis ids */
	int errors;
} Analyzer;

static void _error(Analyzer * a, const char * format, const char * arg) {
	logError(_logger, format, arg);
	a->errors++;
}

static void _checkGraphScope(Analyzer * a, GraphDecl * g) {
	IdSet * nodes = createIdSet();
	IdSet * groups = createIdSet();
	for (NodeDeclList * it = g->nodes; it != NULL; it = it->next) {
		if (!idSetAdd(nodes, it->value->id)) {
			_error(a, "Duplicate node identifier: '%s'.", it->value->id);
		}
	}
	for (GroupDeclList * it = g->groups; it != NULL; it = it->next) {
		if (!idSetAdd(groups, it->value->name)) {
			_error(a, "Duplicate group identifier: '%s'.", it->value->name);
		}
	}
	for (EdgeDeclList * it = g->edges; it != NULL; it = it->next) {
		EdgeDecl * e = it->value;
		if (!idSetContains(nodes, e->from)) {
			_error(a, "Edge references undeclared node: '%s'.", e->from);
		}
		if (!idSetContains(nodes, e->to)) {
			_error(a, "Edge references undeclared node: '%s'.", e->to);
		}
	}
	for (GroupDeclList * it = g->groups; it != NULL; it = it->next) {
		for (IdList * m = it->value->members; m != NULL; m = m->next) {
			if (!idSetContains(nodes, m->value)) {
				_error(a, "Group references undeclared node: '%s'.", m->value);
			}
		}
	}
	destroyIdSet(nodes);
	destroyIdSet(groups);
}

static void _checkAnalysisScope(Analyzer * a, AnalysisDecl * an) {
	IdSet * results = createIdSet();
	for (AnalysisStmtList * it = an->statements; it != NULL; it = it->next) {
		if (it->value->type == ANALYSIS_STMT_RUN) {
			char * resultId = it->value->run->resultId;
			if (!idSetAdd(results, resultId)) {
				_error(a, "Duplicate result identifier: '%s'.", resultId);
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
		.errors = 0
	};
	for (TopLevelDeclList * it = program->decls; it != NULL; it = it->next) {
		_checkTopLevel(&a, it->value);
	}
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
