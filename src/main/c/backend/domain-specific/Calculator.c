#include "Calculator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownCalculatorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Calculator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCalculatorModule() {
	_logger = createLogger("Calculator");
	return _shutdownCalculatorModule;
}

ComputationResult executeCalculator(CompilerState * compilerState) {
	(void) compilerState;
	// Stage II frontend-only: leave backend as a stub.
	ComputationResult computationResult = {
		.succeeded = true,
		.value = 0
	};
	return computationResult;
}
