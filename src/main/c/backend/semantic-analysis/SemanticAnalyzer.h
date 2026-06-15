#ifndef SEMANTIC_ANALYZER_HEADER
#define SEMANTIC_ANALYZER_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>

/** Initialize module's internal state. */
ModuleDestructor initializeSemanticAnalyzerModule();

/** The result of the semantic phase. Valid only if "succeeded" is true. */
typedef struct {
	bool succeeded;
} SemanticResult;

/**
 * Validates the AST stored in the compiler state against the Nexus domain
 * rules. Logs every violation found and reports overall success/failure.
 */
SemanticResult executeSemanticAnalysis(CompilerState * compilerState);

#endif
