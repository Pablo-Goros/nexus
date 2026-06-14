#include "SemanticAnalyzer.h"

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

/* PUBLIC FUNCTIONS */

SemanticResult executeSemanticAnalysis(CompilerState * compilerState) {
	Program * program = (Program *) compilerState->abstractSyntaxtTree;
	if (program == NULL) {
		logError(_logger, "Semantic analysis received a null AST.");
		SemanticResult result = { .succeeded = false };
		return result;
	}
	logDebugging(_logger, "Semantic analysis passed (no rules implemented yet).");
	SemanticResult result = { .succeeded = true };
	return result;
}
