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

static void _checkTopLevel(Analyzer * a, TopLevelDecl * decl) {
	switch (decl->type) {
		case TOP_LEVEL_GRAPH: {
			GraphDecl * g = decl->graphDecl;
			if (!idSetAdd(a->graphIds, g->id)) {
				_error(a, "Duplicate graph identifier: '%s'.", g->id);
			}
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
