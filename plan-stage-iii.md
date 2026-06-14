# Nexus Stage III (Backend) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. **Each top-level Task is one commit**, in order.

**Goal:** Complete the Nexus compiler with a semantic-analysis phase (symbol table + scopes + domain type-checking over the AST) and a code-generation phase that emits a runnable Python program backed by a bundled, stdlib-only runtime library.

**Architecture:** `EntryPoint` runs **parse → semantic analysis → code generation**. Semantic analysis walks the AST building per-scope identifier sets (global graphs/analyses; per-graph nodes/groups; per-analysis results) plus a registry of resolved graph signatures (kind/traits, including derived graphs), rejecting programs that violate domain rules. Code generation is a pure syntax-directed translation: it emits a Python program (to stdout) that imports `nexus_runtime.py` (a copied asset) to build each graph, run algorithms, evaluate constraints as runtime assertions, and write `.dot`/`.json` artifacts.

**Tech Stack:** C (GNU99, ASan), Flex/Bison (already done), CMake, Bash test harness, Python 3 (generated-program runtime, stdlib only — no external libraries).

---

## File Structure

Created:
- `src/main/c/support/symbol-table/IdSet.h` / `IdSet.c` — string hash-set primitive (insert-if-absent, contains). Reused for every duplicate/membership check.
- `src/main/c/backend/semantic-analysis/SemanticAnalyzer.h` / `SemanticAnalyzer.c` — the semantic phase: AST walk, scope/registry construction, all domain validations.
- `src/main/c/backend/code-generation/runtime/nexus_runtime.py` — stdlib-only Python runtime asset (Graph type, 6 algorithms, constraint asserts, dot/json writers).
- `src/main/bash/test-codegen.sh` — generates Python from accept programs, runs it, asserts artifacts.
- `src/test/c/reject-semantic/*` — programs that parse but must be rejected by semantic analysis.
- `docs/report/Stage-III-Report.md` — draft of the required final report.

Modified:
- `src/main/c/EntryPoint.c` — swap the `Calculator` call for `SemanticAnalyzer`; drop the `value` plumbing.
- `src/main/c/backend/code-generation/Generator.c` — real `generate(...)` tree-walk emitting Python.
- `src/main/c/support/type/CompilerState.h` — remove the leftover `value` field.
- `CMakeLists.txt` — add `IdSet.c`, `SemanticAnalyzer.c`; remove `Calculator.c`.
- `src/main/bash/test.sh` — also iterate `src/test/c/reject-semantic/` (must reject).

Deleted:
- `src/main/c/backend/domain-specific/Calculator.h` / `Calculator.c`.

Conventions (from the base project): every module exposes `initializeXModule()` returning a `ModuleDestructor`, owns a static `Logger *`, and frees everything it allocates. Build/run/test commands run inside the Docker dev container (`docker compose run --rm compiler`) or an equivalent Linux env.

---

## Task 1: Scaffold the semantic-analysis module and wire it into the pipeline

Replaces the leftover `Calculator` stub's slot with a permissive `SemanticAnalyzer` that always succeeds. No checks yet — this only proves the wiring builds and the existing suite stays green.

**Files:**
- Create: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.h`
- Create: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Modify: `src/main/c/EntryPoint.c`
- Modify: `src/main/c/support/type/CompilerState.h`
- Modify: `CMakeLists.txt`
- Delete: `src/main/c/backend/domain-specific/Calculator.h`, `src/main/c/backend/domain-specific/Calculator.c`

- [x] **Step 1: Create the header**

`src/main/c/backend/semantic-analysis/SemanticAnalyzer.h`:

```c
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
```

- [x] **Step 2: Create the permissive implementation**

`src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`:

```c
#include "SemanticAnalyzer.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

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
```

- [x] **Step 3: Remove the leftover `value` field from compiler state**

In `src/main/c/support/type/CompilerState.h`, delete the `value` field and its comment block so the struct is:

```c
typedef struct {
	/** The root node of the AST. */
	void * abstractSyntaxtTree;
} CompilerState;
```

- [x] **Step 4: Rewire `EntryPoint.c`**

In `src/main/c/EntryPoint.c`: replace `#include "backend/domain-specific/Calculator.h"` with `#include "backend/semantic-analysis/SemanticAnalyzer.h"`. In the module destructor array, replace `initializeCalculatorModule(),` with `initializeSemanticAnalyzerModule(),`. Change the `CompilerState` initializer to `CompilerState compilerState = { .abstractSyntaxtTree = NULL };`. Replace the backend block with:

```c
		// ----------------------------------------------------------------------------------------
		// Beginning of the Backend... ------------------------------------------------------------
		logDebugging(logger, "Running semantic analysis...");
		SemanticResult semanticResult = executeSemanticAnalysis(&compilerState);
		if (semanticResult.succeeded) {
			executeGenerator(&compilerState);
		}
		else {
			logError(logger, "The semantic-analysis phase rejects the input program.");
			compilationStatus = FAILED;
		}
		// ...end of the Backend. -----------------------------------------------------------------
		// ----------------------------------------------------------------------------------------
```

- [x] **Step 5: Update CMake; delete Calculator**

In `CMakeLists.txt`, inside `add_executable(Nexus ...)` replace the line
`src/main/c/backend/domain-specific/Calculator.c` with
`src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`. Then delete the files:

```bash
git rm src/main/c/backend/domain-specific/Calculator.c src/main/c/backend/domain-specific/Calculator.h
```

- [x] **Step 6: Build and run the full suite**

```bash
src/main/bash/build.sh
src/main/bash/test.sh
```
Expected: build succeeds with no Bison conflicts; all 31 accept tests exit 0; all 12 reject tests exit non-zero (they still fail at parse time). The pipeline now goes through `SemanticAnalyzer` instead of `Calculator`.

- [x] **Step 7: Commit**

```bash
git add -A
git commit -m "feat(semantic): scaffold semantic analyzer and wire into pipeline"
```

---

## Task 2: Add the `IdSet` symbol-table primitive

A string hash-set with insert-if-absent semantics — the building block for every duplicate and membership check. Built now so later tasks consume a stable API.

**Files:**
- Create: `src/main/c/support/symbol-table/IdSet.h`
- Create: `src/main/c/support/symbol-table/IdSet.c`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Create the header**

`src/main/c/support/symbol-table/IdSet.h`:

```c
#ifndef ID_SET_HEADER
#define ID_SET_HEADER

#include <stdbool.h>
#include <stddef.h>

/** A hash-set of identifier strings (owns copies of the keys). */
typedef struct IdSet IdSet;

IdSet * createIdSet(void);
void destroyIdSet(IdSet * set);

/** Inserts "id". Returns true if inserted, false if it was already present. */
bool idSetAdd(IdSet * set, const char * id);

/** Returns true if "id" is present. */
bool idSetContains(const IdSet * set, const char * id);

#endif
```

- [ ] **Step 2: Create the implementation**

`src/main/c/support/symbol-table/IdSet.c`:

```c
#include "IdSet.h"
#include <stdlib.h>
#include <string.h>

#define ID_SET_BUCKETS 97

typedef struct Entry {
	char * key;
	struct Entry * next;
} Entry;

struct IdSet {
	Entry * buckets[ID_SET_BUCKETS];
};

static size_t _hash(const char * s) {
	size_t h = 5381;
	for (; *s; ++s) {
		h = ((h << 5) + h) + (unsigned char) *s; /* djb2 */
	}
	return h % ID_SET_BUCKETS;
}

IdSet * createIdSet(void) {
	IdSet * set = calloc(1, sizeof(IdSet));
	return set;
}

void destroyIdSet(IdSet * set) {
	if (set == NULL) {
		return;
	}
	for (size_t i = 0; i < ID_SET_BUCKETS; ++i) {
		Entry * e = set->buckets[i];
		while (e != NULL) {
			Entry * next = e->next;
			free(e->key);
			free(e);
			e = next;
		}
	}
	free(set);
}

bool idSetContains(const IdSet * set, const char * id) {
	const Entry * e = set->buckets[_hash(id)];
	while (e != NULL) {
		if (strcmp(e->key, id) == 0) {
			return true;
		}
		e = e->next;
	}
	return false;
}

bool idSetAdd(IdSet * set, const char * id) {
	if (idSetContains(set, id)) {
		return false;
	}
	size_t b = _hash(id);
	Entry * e = malloc(sizeof(Entry));
	e->key = malloc(strlen(id) + 1);
	strcpy(e->key, id);
	e->next = set->buckets[b];
	set->buckets[b] = e;
	return true;
}
```

- [ ] **Step 3: Add to CMake**

In `CMakeLists.txt`, add to `add_executable(Nexus ...)`:
`src/main/c/support/symbol-table/IdSet.c`

- [ ] **Step 4: Build and verify the suite stays green**

```bash
src/main/bash/build.sh
src/main/bash/test.sh
```
Expected: build succeeds; 31 accept exit 0; 12 reject exit non-zero. `IdSet` compiles and links but is not exercised yet.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(semantic): add IdSet string hash-set primitive"
```

---

## Task 3: Detect duplicate global identifiers (graphs and analyses) + reject-semantic harness

Introduces the AST traversal skeleton and the first real rule. Also wires a dedicated `reject-semantic/` test folder into `test.sh`.

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Modify: `src/main/bash/test.sh`
- Create: `src/test/c/reject-semantic/01-duplicate-graph`
- Create: `src/test/c/reject-semantic/02-duplicate-analysis`

- [ ] **Step 1: Write the failing tests**

`src/test/c/reject-semantic/01-duplicate-graph`:

```
graph G:
    kind directed
    nodes:
        a
graph G:
    kind directed
    nodes:
        a
```

`src/test/c/reject-semantic/02-duplicate-analysis`:

```
graph G:
    kind directed
    nodes:
        a
analysis M on G:
    export graph to dot
analysis M on G:
    export graph to dot
```

- [ ] **Step 2: Extend `test.sh` to check the new folder**

In `src/main/bash/test.sh`, immediately after the existing `reject` loop's closing `done` and its `echo ""`, add:

```bash
echo "Compiler should reject (semantic)..."
echo ""

for test in $(ls src/test/c/reject-semantic/); do
	cat "src/test/c/reject-semantic/$test" | ".build/Nexus" >/dev/null 2>&1
	RESULT="$?"
	if [ "$RESULT" != "0" ]; then
		echo -e "    $test, ${GREEN}and it does${OFF} (status $RESULT)"
	else
		STATUS=1
		echo -e "    $test, ${RED}but it accepts${OFF} (status $RESULT)"
	fi
done
echo ""
```

- [ ] **Step 3: Run to verify they currently fail (are wrongly accepted)**

```bash
src/main/bash/build.sh
src/main/bash/test.sh
```
Expected: both `reject-semantic` programs report `but it accepts (status 0)` and the suite exits non-zero — the RED state (no duplicate rule yet).

- [ ] **Step 4: Implement global-duplicate detection**

Rewrite `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c` so the walk records errors and registers global ids. Replace the file body (keep the module boilerplate) with:

```c
#include "SemanticAnalyzer.h"
#include "../../support/symbol-table/IdSet.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

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
		logError(_logger, "Semantic analysis found %d error(s).", (const char *) (long) 0);
	}
	return result;
}
```

Note: the final summary log uses the project's `logError(logger, format, ...)` signature; pass the count via a small local instead of a cast. Replace the summary `else` block with:

```c
	else {
		logError(_logger, "Semantic analysis rejected the program.");
	}
```

- [ ] **Step 5: Run to verify GREEN**

```bash
src/main/bash/build.sh
src/main/bash/test.sh
```
Expected: `01-duplicate-graph` and `02-duplicate-analysis` now report `and it does`; all 31 accept still exit 0; all 12 original reject still exit non-zero; suite exits 0.

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "feat(semantic): detect duplicate graph and analysis identifiers"
```

---

## Task 4: Detect duplicate node, group, and result identifiers

Per-graph scopes for node/group ids and a per-analysis scope for result ids.

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Create: `src/test/c/reject-semantic/03-duplicate-node`
- Create: `src/test/c/reject-semantic/04-duplicate-group`
- Create: `src/test/c/reject-semantic/05-duplicate-result`

- [ ] **Step 1: Write the failing tests**

`src/test/c/reject-semantic/03-duplicate-node`:

```
graph G:
    kind directed
    nodes:
        a
        a
```

`src/test/c/reject-semantic/04-duplicate-group`:

```
graph G:
    kind directed
    nodes:
        a
    groups:
        x = {a}
        x = {a}
```

`src/test/c/reject-semantic/05-duplicate-result`:

```
graph G:
    kind directed
    nodes:
        a
        b
    edges:
        a -> b
analysis M on G:
    run topological_sort as r
    run topological_sort as r
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: the three new programs report `but it accepts (status 0)`.

- [ ] **Step 3: Implement per-graph and per-analysis duplicate checks**

In `SemanticAnalyzer.c`, add two helpers and call them from `_checkTopLevel`. Add before `_checkTopLevel`:

```c
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
```

In `_checkTopLevel`, after the duplicate-graph `idSetAdd` for `TOP_LEVEL_GRAPH`, add `_checkGraphScope(a, g);`. In the `TOP_LEVEL_ANALYSIS` case, after the duplicate-analysis `idSetAdd`, add `_checkAnalysisScope(a, an);`.

- [ ] **Step 4: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 03–05 now report `and it does`; full suite exits 0.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(semantic): detect duplicate node, group, and result identifiers"
```

---

## Task 5: Resolve graph-internal references (edge endpoints, group members)

Edges and group members must reference nodes declared in the same graph. Reuse the per-graph node set built in Task 4 — promote it so the reference checks can see it.

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Create: `src/test/c/reject-semantic/06-undeclared-edge-endpoint`
- Create: `src/test/c/reject-semantic/07-undeclared-group-member`

- [ ] **Step 1: Write the failing tests**

`src/test/c/reject-semantic/06-undeclared-edge-endpoint`:

```
graph G:
    kind directed
    nodes:
        a
    edges:
        a -> b
```

`src/test/c/reject-semantic/07-undeclared-group-member`:

```
graph G:
    kind directed
    nodes:
        a
    groups:
        x = {a, b}
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 06 and 07 report `but it accepts (status 0)`.

- [ ] **Step 3: Implement reference checks inside the graph scope**

In `_checkGraphScope`, after the node/group sets are populated and before they are destroyed, add edge-endpoint and group-member resolution:

```c
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
```

(Place this block after the existing group-duplicate loop, while `nodes` and `groups` are still alive.)

- [ ] **Step 4: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 06 and 07 now report `and it does`; full suite exits 0.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(semantic): resolve edge endpoints and group members"
```

---

## Task 6: Resolve cross-declaration references + register derived-graph signatures

`derive ... from G`, `analysis ... on G`, `run ... from/to`, and `export result <id>` must resolve against the right scope. To check analyses against the *target* graph's nodes (including derived graphs), build a small registry mapping each graph id to a `GraphInfo` (kind, traits, node set, group set). Derived graphs get a synthesized `GraphInfo` from their source + transformation.

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Create: `src/test/c/reject-semantic/08-undeclared-derive-source`
- Create: `src/test/c/reject-semantic/09-undeclared-analysis-graph`
- Create: `src/test/c/reject-semantic/10-undeclared-run-endpoint`
- Create: `src/test/c/reject-semantic/11-undeclared-export-result`

- [ ] **Step 1: Write the failing tests**

`src/test/c/reject-semantic/08-undeclared-derive-source`:

```
graph G:
    kind directed
    nodes:
        a
derive R from H using transpose
```

`src/test/c/reject-semantic/09-undeclared-analysis-graph`:

```
graph G:
    kind directed
    nodes:
        a
analysis M on H:
    export graph to dot
```

`src/test/c/reject-semantic/10-undeclared-run-endpoint`:

```
graph G:
    kind directed
    traits weighted
    nodes:
        a
        b
    edges:
        a -> b weight 1
analysis M on G:
    run shortest_path from a to z as p
```

`src/test/c/reject-semantic/11-undeclared-export-result`:

```
graph G:
    kind directed
    nodes:
        a
        b
    edges:
        a -> b
analysis M on G:
    run topological_sort as r
    export result nope to json
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 08–11 report `but it accepts (status 0)`.

- [ ] **Step 3: Add the graph registry types**

In `SemanticAnalyzer.c`, above the `Analyzer` struct, add:

```c
typedef struct GraphInfo {
	char * id;             /* not owned: points into the AST */
	GraphKind kind;
	GraphTraits traits;
	IdSet * nodes;         /* owned */
	IdSet * groups;        /* owned */
	struct GraphInfo * next;
} GraphInfo;
```

Extend `Analyzer` with a registry head: add `GraphInfo * graphs;` to the struct and initialize `.graphs = NULL` in `executeSemanticAnalysis`.

Add registry helpers (after the `GraphInfo` typedef):

```c
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
```

Add `#include <string.h>` at the top.

- [ ] **Step 4: Populate the registry during graph/derive checks**

Change `_checkGraphScope` to build and return the registered `GraphInfo` so its node/group sets persist in the registry instead of being freed. Replace `_checkGraphScope`'s body so it registers the graph, fills `info->nodes` / `info->groups`, and uses them for the reference checks from Task 5:

```c
static void _checkGraphScope(Analyzer * a, GraphDecl * g) {
	GraphInfo * info = _registerGraph(a, g->id, g->kind, g->traits);
	for (NodeDeclList * it = g->nodes; it != NULL; it = it->next) {
		if (!idSetAdd(info->nodes, it->value->id)) {
			_error(a, "Duplicate node identifier: '%s'.", it->value->id);
		}
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
	}
	for (GroupDeclList * it = g->groups; it != NULL; it = it->next) {
		for (IdList * m = it->value->members; m != NULL; m = m->next) {
			if (!idSetContains(info->nodes, m->value)) {
				_error(a, "Group references undeclared node: '%s'.", m->value);
			}
		}
	}
}
```

In `executeSemanticAnalysis`, after the traversal loop, free the registry:

```c
	for (GraphInfo * g = a.graphs; g != NULL; ) {
		GraphInfo * next = g->next;
		destroyIdSet(g->nodes);
		destroyIdSet(g->groups);
		free(g);
		g = next;
	}
```

- [ ] **Step 5: Add derive-source resolution and derived-graph synthesis**

In `_checkTopLevel`'s `TOP_LEVEL_DERIVE` case, after the duplicate `idSetAdd`, resolve the source and register the derived graph:

```c
			GraphInfo * src = _findGraph(a, d->fromId);
			if (src == NULL) {
				_error(a, "Derive references undeclared graph: '%s'.", d->fromId);
			}
			else {
				GraphKind kind = src->kind;
				if (d->transformation->type == TRANSFORMATION_UNDERLYING) {
					kind = GRAPH_KIND_UNDIRECTED;
				}
				GraphInfo * derived = _registerGraph(a, d->newId, kind, src->traits);
				if (d->transformation->type == TRANSFORMATION_INDUCED_SUBGRAPH) {
					if (!idSetContains(src->groups, d->transformation->group)) {
						_error(a, "induced_subgraph references undeclared group: '%s'.",
							d->transformation->group);
					}
				}
				/* For reference checks, a derived graph exposes the same node ids
				   as its source (induced_subgraph narrows at runtime). */
				for (GraphInfo * s = a->graphs; s != NULL; s = s->next) {
					if (s == src) { break; }
				}
				/* Copy source node ids into the derived graph's node set. */
				_copyNodeIds(derived, src);
```

This requires a node-id copy helper. Because `IdSet` has no iterator, add a re-derivation instead: change `_registerGraph` calls for derived graphs to copy by walking the AST is unavailable here, so add this helper near the registry helpers:

```c
/* IdSet has no iterator; expose the keys so derived graphs can inherit them. */
```

To keep `IdSet` simple, add a tiny callback-based visitor to `IdSet` now:

In `IdSet.h` add:

```c
/** Visits every id in the set. */
void idSetForEach(const IdSet * set, void (*visit)(const char * id, void * context), void * context);
```

In `IdSet.c` add:

```c
void idSetForEach(const IdSet * set, void (*visit)(const char * id, void * context), void * context) {
	for (size_t i = 0; i < ID_SET_BUCKETS; ++i) {
		for (const Entry * e = set->buckets[i]; e != NULL; e = e->next) {
			visit(e->key, context);
		}
	}
}
```

Then in `SemanticAnalyzer.c` add the copy helper above `_checkTopLevel`:

```c
static void _addIdToSet(const char * id, void * context) {
	idSetAdd((IdSet *) context, id);
}

static void _copyNodeIds(GraphInfo * dst, GraphInfo * src) {
	idSetForEach(src->nodes, _addIdToSet, dst->nodes);
}
```

Replace the loose loop fragment in the derive case (the `for (GraphInfo * s ...)` stub) so the final derive case reads exactly:

```c
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
				if (d->transformation->type == TRANSFORMATION_INDUCED_SUBGRAPH
						&& !idSetContains(src->groups, d->transformation->group)) {
					_error(a, "induced_subgraph references undeclared group: '%s'.",
						d->transformation->group);
				}
			}
			break;
		}
```

- [ ] **Step 6: Add analysis cross-references (on-graph, run endpoints, export result)**

Replace `_checkAnalysisScope` so it resolves the target graph and validates run/export references:

```c
static void _checkAnalysisScope(Analyzer * a, AnalysisDecl * an) {
	GraphInfo * target = _findGraph(a, an->onGraphId);
	if (target == NULL) {
		_error(a, "Analysis references undeclared graph: '%s'.", an->onGraphId);
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
			}
		}
		else { /* ANALYSIS_STMT_EXPORT */
			ExportStmt * ex = stmt->exportStmt;
			if (ex->targetType == EXPORT_TARGET_RESULT && !idSetContains(results, ex->resultId)) {
				_error(a, "Export references undeclared result: '%s'.", ex->resultId);
			}
		}
	}
	destroyIdSet(results);
}
```

- [ ] **Step 7: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 08–11 now report `and it does`; all accept exit 0; all rejects (lexical/syntactic + semantic) non-zero. Run under ASan; no leaks reported on exit.

- [ ] **Step 8: Commit**

```bash
git add -A
git commit -m "feat(semantic): resolve cross-declaration references and derived graphs"
```

---

## Task 7: Trait and operator compatibility

`weight` requires the `weighted` trait; `capacity` requires `capacitated`; `->` requires a directed graph; `--` requires undirected.

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Create: `src/test/c/reject-semantic/12-weight-without-trait`
- Create: `src/test/c/reject-semantic/13-capacity-without-trait`
- Create: `src/test/c/reject-semantic/14-directed-op-on-undirected`
- Create: `src/test/c/reject-semantic/15-undirected-op-on-directed`

- [ ] **Step 1: Write the failing tests**

`12-weight-without-trait`:

```
graph G:
    kind directed
    nodes:
        a
        b
    edges:
        a -> b weight 2
```

`13-capacity-without-trait`:

```
graph G:
    kind directed
    nodes:
        a
        b
    edges:
        a -> b capacity 5
```

`14-directed-op-on-undirected`:

```
graph G:
    kind undirected
    nodes:
        a
        b
    edges:
        a -> b
```

`15-undirected-op-on-directed`:

```
graph G:
    kind directed
    nodes:
        a
        b
    edges:
        a -- b
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 12–15 report `but it accepts (status 0)`.

- [ ] **Step 3: Implement the edge-vs-graph checks**

In `_checkGraphScope`, extend the existing edge loop body (the one that checks endpoints) to also validate traits and operator:

```c
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
```

- [ ] **Step 4: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 12–15 now report `and it does`; full suite exits 0.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(semantic): check edge trait and operator compatibility"
```

---

## Task 8: Constraint-vs-kind and transformation-vs-source-kind checks

`strongly_connected` requires a directed graph; `transpose` requires a directed source. (Other constraints/transformations are valid on both kinds in the Nexus domain.) `forall` predicate variables must equal the bound loop variable, and the named group must exist.

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Create: `src/test/c/reject-semantic/16-strongly-connected-undirected`
- Create: `src/test/c/reject-semantic/17-transpose-undirected`
- Create: `src/test/c/reject-semantic/18-forall-unknown-group`

- [ ] **Step 1: Write the failing tests**

`16-strongly-connected-undirected`:

```
graph G:
    kind undirected
    nodes:
        a
    constraints:
        assert strongly_connected
```

`17-transpose-undirected`:

```
graph G:
    kind undirected
    nodes:
        a
derive R from G using transpose
```

`18-forall-unknown-group`:

```
graph G:
    kind directed
    nodes:
        a
    constraints:
        assert forall n in missing: outdegree(n) = 0
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 16–18 report `but it accepts (status 0)`.

- [ ] **Step 3: Implement constraint checks**

Add a constraint checker and call it from `_checkGraphScope` after the group-member loop:

```c
static void _checkConstraints(Analyzer * a, GraphDecl * g, GraphInfo * info) {
	for (ConstraintList * it = g->constraints; it != NULL; it = it->next) {
		Constraint * c = it->value;
		switch (c->type) {
			case CONSTRAINT_SIMPLE:
				if (c->simple == SIMPLE_STRONGLY_CONNECTED
						&& g->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'strongly_connected' requires a directed graph: '%s'.", g->id);
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
				if (!idSetContains(info->nodes, c->tree.root)) {
					_error(a, "tree 'rooted_at' references undeclared node: '%s'.", c->tree.root);
				}
				break;
			case CONSTRAINT_BINARY_TREE:
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
```

Call `_checkConstraints(a, g, info);` at the end of `_checkGraphScope`.

- [ ] **Step 4: Implement the transpose-vs-kind check**

In the `TOP_LEVEL_DERIVE` case, inside the `else` branch (where `src != NULL`), after computing `kind`, add:

```c
				if (d->transformation->type == TRANSFORMATION_TRANSPOSE
						&& src->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'transpose' requires a directed source graph: '%s'.", d->fromId);
				}
				if (d->transformation->type == TRANSFORMATION_UNDERLYING
						&& src->kind != GRAPH_KIND_DIRECTED) {
					_error(a, "'underlying' requires a directed source graph: '%s'.", d->fromId);
				}
```

- [ ] **Step 5: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 16–18 now report `and it does`; full suite exits 0. (Existing accept test `17-constraint-forall` still passes — its predicate var `n` matches its bound var `n` and group `leaves` exists.)

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "feat(semantic): check constraint and transformation kind compatibility"
```

---

## Task 9: Algorithm-vs-kind/traits compatibility

`topological_sort`/`scc` require directed; `components` requires undirected; `mst` requires undirected + weighted; `max_flow` requires directed + capacitated; `shortest_path` requires weighted. Checked against the target graph's resolved `GraphInfo` (so it also covers analyses on derived graphs).

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Create: `src/test/c/reject-semantic/19-mst-on-directed`
- Create: `src/test/c/reject-semantic/20-maxflow-without-capacity`
- Create: `src/test/c/reject-semantic/21-toposort-on-undirected`
- Create: `src/test/c/reject-semantic/22-shortest-path-unweighted`

- [ ] **Step 1: Write the failing tests**

`19-mst-on-directed`:

```
graph G:
    kind directed
    traits weighted
    nodes:
        a
        b
    edges:
        a -> b weight 1
analysis M on G:
    run mst as t
```

`20-maxflow-without-capacity`:

```
graph G:
    kind directed
    nodes:
        a
        b
    edges:
        a -> b
analysis M on G:
    run max_flow from a to b as f
```

`21-toposort-on-undirected`:

```
graph G:
    kind undirected
    nodes:
        a
        b
    edges:
        a -- b
analysis M on G:
    run topological_sort as r
```

`22-shortest-path-unweighted`:

```
graph G:
    kind directed
    nodes:
        a
        b
    edges:
        a -> b
analysis M on G:
    run shortest_path from a to b as p
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 19–22 report `but it accepts (status 0)`.

- [ ] **Step 3: Implement the algorithm matrix**

Add a helper and call it from `_checkAnalysisScope` for each run statement (where `target != NULL`):

```c
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
```

In `_checkAnalysisScope`, inside the `if (target != NULL)` block of the run case, add `_checkAlgorithm(a, target, run->algorithm);`.

- [ ] **Step 4: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 19–22 now report `and it does`; full suite exits 0. Confirm existing accept tests 22–27 still pass (their graphs carry the traits/kinds each algorithm needs; if any do not, update that accept program to declare the required trait/kind — e.g. `22-analysis-shortest-path` must be `weighted`).

- [ ] **Step 5: Reconcile existing accept tests with the algorithm rules**

```bash
for t in 22 23 24 25 26 27; do
  echo "== $t =="; cat src/test/c/accept/$t-*; echo
done
```
For each, ensure the graph satisfies its algorithm's rule (shortest_path→weighted, mst→undirected+weighted, max_flow→directed+capacitated, components→undirected, scc/toposort→directed). Edit the accept program minimally where needed, then re-run `src/main/bash/test.sh` until green.

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "feat(semantic): check algorithm kind and trait compatibility"
```

---

## Task 10: Reject empty analyses and node-less graphs

A "meaningful construct" rule in the spirit of a reference group's mandatory-clause check: the grammar permits a graph with an empty `nodes:` section and an analysis with zero statements, but neither is semantically useful. Reject both.

**Files:**
- Modify: `src/main/c/backend/semantic-analysis/SemanticAnalyzer.c`
- Create: `src/test/c/reject-semantic/23-empty-analysis`
- Create: `src/test/c/reject-semantic/24-nodeless-graph`

- [ ] **Step 1: Write the failing tests**

`src/test/c/reject-semantic/23-empty-analysis`:

```
graph G:
    kind directed
    nodes:
        a
analysis M on G:
```

`src/test/c/reject-semantic/24-nodeless-graph`:

```
graph G:
    kind directed
    nodes:
    edges:
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: both new programs report `but it accepts (status 0)` — they parse (empty node list and empty statement list are grammar-legal) but are not yet rejected.

- [ ] **Step 3: Implement the non-empty checks**

In `_checkGraphScope`, immediately after `GraphInfo * info = _registerGraph(...)`, add:

```c
	if (g->nodes == NULL) {
		_error(a, "Graph '%s' declares no nodes.", g->id);
	}
```

In `_checkAnalysisScope`, immediately after resolving `target` (before the result loop), add:

```c
	if (an->statements == NULL) {
		_error(a, "Analysis '%s' has no statements.", an->id);
	}
```

(Empty lists are represented as `NULL` by the `Empty*SemanticAction` constructors, so a `NULL` head means the section is empty.)

- [ ] **Step 4: Verify GREEN and that no accept test regresses**

```bash
src/main/bash/build.sh && src/main/bash/test.sh
```
Expected: tests 23–24 now report `and it does`. Confirm every `src/test/c/accept/*` program still exits 0 — each declares at least one node and every analysis has at least one statement. If any accept program has an empty `nodes:` section or a statement-less analysis, that program is itself degenerate; add a minimal node/statement to it rather than weakening the rule.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(semantic): reject empty analyses and node-less graphs"
```

---

## Task 11: The `nexus_runtime.py` runtime asset

A stdlib-only Python module providing the graph type, the six algorithms, constraint assertions, and the dot/json writers. The generated program imports it. No external libraries (keeps the compiler dependency-free and avoids the QRF approval requirement).

**Files:**
- Create: `src/main/c/backend/code-generation/runtime/nexus_runtime.py`

- [ ] **Step 1: Write the runtime library**

`src/main/c/backend/code-generation/runtime/nexus_runtime.py`:

```python
"""Nexus runtime: graph model, algorithms, constraints, and exporters.

Bundled as a compiler asset and copied next to each generated program.
Standard library only.
"""

import heapq
import json
from collections import deque


class Graph:
    def __init__(self, directed=False, weighted=False, capacitated=False):
        self.directed = directed
        self.weighted = weighted
        self.capacitated = capacitated
        self.nodes = {}          # id -> attr (str or None)
        self.adj = {}            # id -> list of (to, weight, capacity)
        self.groups = {}         # name -> list of node ids

    def add_node(self, node_id, attr=None):
        self.nodes.setdefault(node_id, attr)
        self.adj.setdefault(node_id, [])

    def add_edge(self, u, v, weight=None, capacity=None):
        self.adj[u].append((v, weight, capacity))
        if not self.directed:
            self.adj[v].append((u, weight, capacity))

    def add_group(self, name, members):
        self.groups[name] = list(members)

    def neighbors(self, u):
        return self.adj.get(u, [])


# ---- Derivations -------------------------------------------------------------

def transpose(g):
    t = Graph(directed=g.directed, weighted=g.weighted, capacitated=g.capacitated)
    for n, attr in g.nodes.items():
        t.add_node(n, attr)
    for u in g.adj:
        for (v, w, c) in g.adj[u]:
            t.add_edge(v, u, w, c)
    t.groups = dict(g.groups)
    return t


def induced_subgraph(g, group_name):
    keep = set(g.groups.get(group_name, []))
    s = Graph(directed=g.directed, weighted=g.weighted, capacitated=g.capacitated)
    for n in keep:
        s.add_node(n, g.nodes.get(n))
    for u in keep:
        for (v, w, c) in g.adj[u]:
            if v in keep:
                s.add_edge(u, v, w, c)
    return s


def remove_self_loops(g):
    s = Graph(directed=g.directed, weighted=g.weighted, capacitated=g.capacitated)
    for n, attr in g.nodes.items():
        s.add_node(n, attr)
    for u in g.adj:
        for (v, w, c) in g.adj[u]:
            if u != v:
                s.add_edge(u, v, w, c)
    s.groups = dict(g.groups)
    return s


def underlying(g):
    u = Graph(directed=False, weighted=g.weighted, capacitated=g.capacitated)
    for n, attr in g.nodes.items():
        u.add_node(n, attr)
    seen = set()
    for a in g.adj:
        for (b, w, c) in g.adj[a]:
            key = frozenset((a, b))
            if key not in seen:
                seen.add(key)
                u.add_edge(a, b, w, c)
    u.groups = dict(g.groups)
    return u


# ---- Algorithms --------------------------------------------------------------

def shortest_path(g, source, target):
    dist = {n: float("inf") for n in g.nodes}
    prev = {n: None for n in g.nodes}
    dist[source] = 0
    pq = [(0, source)]
    while pq:
        d, u = heapq.heappop(pq)
        if d > dist[u]:
            continue
        for (v, w, _c) in g.neighbors(u):
            weight = 1 if w is None else w
            if dist[u] + weight < dist[v]:
                dist[v] = dist[u] + weight
                prev[v] = u
                heapq.heappush(pq, (dist[v], v))
    path = []
    node = target
    if dist.get(target, float("inf")) == float("inf"):
        return {"source": source, "target": target, "distance": None, "path": []}
    while node is not None:
        path.append(node)
        node = prev[node]
    path.reverse()
    return {"source": source, "target": target, "distance": dist[target], "path": path}


def topological_sort(g):
    indeg = {n: 0 for n in g.nodes}
    for u in g.adj:
        for (v, _w, _c) in g.adj[u]:
            indeg[v] += 1
    q = deque([n for n in g.nodes if indeg[n] == 0])
    order = []
    while q:
        u = q.popleft()
        order.append(u)
        for (v, _w, _c) in g.adj[u]:
            indeg[v] -= 1
            if indeg[v] == 0:
                q.append(v)
    if len(order) != len(g.nodes):
        return {"order": None, "acyclic": False}
    return {"order": order, "acyclic": True}


def components(g):
    seen = set()
    result = []
    for start in g.nodes:
        if start in seen:
            continue
        comp = []
        q = deque([start])
        seen.add(start)
        while q:
            u = q.popleft()
            comp.append(u)
            for (v, _w, _c) in g.neighbors(u):
                if v not in seen:
                    seen.add(v)
                    q.append(v)
        result.append(sorted(comp))
    return {"components": result, "count": len(result)}


def scc(g):
    index = {}
    low = {}
    on_stack = {}
    stack = []
    result = []
    counter = [0]

    def strongconnect(v):
        index[v] = counter[0]
        low[v] = counter[0]
        counter[0] += 1
        stack.append(v)
        on_stack[v] = True
        for (w, _wt, _c) in g.neighbors(v):
            if w not in index:
                strongconnect(w)
                low[v] = min(low[v], low[w])
            elif on_stack.get(w):
                low[v] = min(low[v], index[w])
        if low[v] == index[v]:
            comp = []
            while True:
                w = stack.pop()
                on_stack[w] = False
                comp.append(w)
                if w == v:
                    break
            result.append(sorted(comp))

    for v in g.nodes:
        if v not in index:
            strongconnect(v)
    return {"components": result, "count": len(result)}


def mst(g):
    parent = {n: n for n in g.nodes}

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    edges = []
    seen = set()
    for u in g.adj:
        for (v, w, _c) in g.adj[u]:
            key = frozenset((u, v))
            if key in seen:
                continue
            seen.add(key)
            edges.append((1 if w is None else w, u, v))
    edges.sort()
    chosen = []
    total = 0
    for (w, u, v) in edges:
        ru, rv = find(u), find(v)
        if ru != rv:
            parent[ru] = rv
            chosen.append({"from": u, "to": v, "weight": w})
            total += w
    return {"edges": chosen, "total_weight": total}


def max_flow(g, source, target):
    cap = {}
    for u in g.adj:
        for (v, _w, c) in g.adj[u]:
            value = 0 if c is None else c
            cap[(u, v)] = cap.get((u, v), 0) + value
            cap.setdefault((v, u), 0)
    flow = 0
    while True:
        parent = {source: source}
        q = deque([source])
        while q and target not in parent:
            u = q.popleft()
            for (a, b), cuv in list(cap.items()):
                if a == u and b not in parent and cuv > 0:
                    parent[b] = u
                    q.append(b)
        if target not in parent:
            break
        bottleneck = float("inf")
        v = target
        while v != source:
            u = parent[v]
            bottleneck = min(bottleneck, cap[(u, v)])
            v = u
        v = target
        while v != source:
            u = parent[v]
            cap[(u, v)] -= bottleneck
            cap[(v, u)] += bottleneck
            v = u
        flow += bottleneck
    return {"source": source, "target": target, "max_flow": flow}


# ---- Constraint assertions ---------------------------------------------------

def _reachable(g, source, target):
    seen = {source}
    q = deque([source])
    while q:
        u = q.popleft()
        if u == target:
            return True
        for (v, _w, _c) in g.neighbors(u):
            if v not in seen:
                seen.add(v)
                q.append(v)
    return target in seen


def assert_connected(g):
    if not g.nodes:
        return
    start = next(iter(g.nodes))
    reach = components(underlying(g) if g.directed else g)
    assert reach["count"] == 1, "constraint failed: graph is not connected"


def assert_strongly_connected(g):
    assert scc(g)["count"] == 1, "constraint failed: graph is not strongly connected"


def assert_acyclic(g):
    if g.directed:
        assert topological_sort(g)["acyclic"], "constraint failed: graph is not acyclic"
    else:
        assert len([e for u in g.adj for e in g.adj[u]]) // 2 < len(g.nodes), \
            "constraint failed: graph is not acyclic"


def assert_reachable(g, source, target):
    assert _reachable(g, source, target), \
        "constraint failed: '%s' does not reach '%s'" % (source, target)


def assert_tree(g, root):
    edge_count = sum(len(g.adj[u]) for u in g.adj)
    edge_count = edge_count if g.directed else edge_count // 2
    reach = components(underlying(g) if g.directed else g)
    assert reach["count"] == 1 and edge_count == len(g.nodes) - 1, \
        "constraint failed: graph rooted at '%s' is not a tree" % root


def assert_binary_tree(g, root):
    assert_tree(g, root)
    for u in g.nodes:
        children = len(g.adj[u]) if g.directed else max(0, len(g.adj[u]) - 1)
        assert children <= 2, \
            "constraint failed: node '%s' has more than two children" % u


def _degree(g, node, kind):
    out = len(g.adj.get(node, []))
    inc = sum(1 for u in g.adj for (v, _w, _c) in g.adj[u] if v == node)
    if kind == "indegree":
        return inc
    if kind == "outdegree":
        return out
    return inc + out if g.directed else out


def assert_forall(g, group, fn, cmp, rhs):
    ops = {
        "=": lambda x, y: x == y, "!=": lambda x, y: x != y,
        ">=": lambda x, y: x >= y, "<=": lambda x, y: x <= y,
        ">": lambda x, y: x > y, "<": lambda x, y: x < y,
    }
    for node in g.groups.get(group, []):
        value = _degree(g, node, fn)
        assert ops[cmp](value, rhs), \
            "constraint failed: %s(%s) %s %d is false" % (fn, node, cmp, rhs)


# ---- Exporters ---------------------------------------------------------------

def write_dot(g, filename):
    arrow = "->" if g.directed else "--"
    keyword = "digraph" if g.directed else "graph"
    lines = ["%s Nexus {" % keyword]
    for n in g.nodes:
        lines.append('    "%s";' % n)
    seen = set()
    for u in g.adj:
        for (v, w, c) in g.adj[u]:
            if not g.directed:
                key = frozenset((u, v))
                if key in seen:
                    continue
                seen.add(key)
            labels = []
            if w is not None:
                labels.append("w=%s" % w)
            if c is not None:
                labels.append("c=%s" % c)
            label = ' [label="%s"]' % ",".join(labels) if labels else ""
            lines.append('    "%s" %s "%s"%s;' % (u, arrow, v, label))
    lines.append("}")
    with open(filename, "w") as handle:
        handle.write("\n".join(lines) + "\n")


def write_json(value, filename):
    with open(filename, "w") as handle:
        json.dump(value, handle, indent=2)
```

- [ ] **Step 2: Smoke-test the runtime in isolation**

```bash
cd src/main/c/backend/code-generation/runtime
python3 -c "
import nexus_runtime as nx
g = nx.Graph(directed=True, weighted=True, capacitated=True)
for n in ['a','b','c']: g.add_node(n)
g.add_edge('a','b',weight=2); g.add_edge('b','c',capacity=5,weight=1)
print(nx.shortest_path(g,'a','c'))
print(nx.topological_sort(g))
nx.write_dot(g,'/tmp/g.dot'); nx.write_json(nx.shortest_path(g,'a','c'),'/tmp/p.json')
print(open('/tmp/g.dot').read())
"
cd -
```
Expected: prints a path dict with `distance: 3` and `path: ['a','b','c']`, a topological order, and valid DOT text; no exceptions.

- [ ] **Step 3: Commit**

```bash
git add -A
git commit -m "feat(codegen): add stdlib-only Python runtime library"
```

---

## Task 12: Generator — emit graph construction + codegen test harness

The generator becomes a real tree-walk that prints a Python program to stdout. This task covers graph construction (nodes, attrs, edges, groups) and the import header, plus the `test-codegen.sh` harness that compiles and runs a program end-to-end.

**Files:**
- Modify: `src/main/c/backend/code-generation/Generator.c`
- Create: `src/main/bash/test-codegen.sh`

- [ ] **Step 1: Write the codegen harness**

`src/main/bash/test-codegen.sh`:

```bash
#! /bin/bash
set -u
BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

GREEN='\033[0;32m'; RED='\033[0;31m'; OFF='\033[0m'; STATUS=0
RUNTIME="src/main/c/backend/code-generation/runtime/nexus_runtime.py"
WORK="$(mktemp -d)"
cp "$RUNTIME" "$WORK/"

echo "Code generation should produce runnable Python..."
echo ""
for test in 30-stage-i-integration; do
	cat "src/test/c/accept/$test" | ".build/Nexus" 2>/dev/null > "$WORK/program.py"
	if [ ! -s "$WORK/program.py" ]; then
		STATUS=1; echo -e "    $test, ${RED}emitted no Python${OFF}"; continue
	fi
	( cd "$WORK" && python3 program.py ) >/dev/null 2>&1
	if [ "$?" == "0" ]; then
		echo -e "    $test, ${GREEN}runs cleanly${OFF}"
	else
		STATUS=1; echo -e "    $test, ${RED}runtime error${OFF}"
	fi
done
echo ""
rm -rf "$WORK"
echo "All done."
exit $STATUS
```

Make it executable: `chmod +x src/main/bash/test-codegen.sh`.

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh
src/main/bash/test-codegen.sh
```
Expected: `emitted no Python` — the generator is still the stub.

- [ ] **Step 3: Drop the leftover calculator indentation constants**

In `src/main/c/backend/code-generation/Generator.c`, delete the two unused constants in the `MODULE INTERNAL STATE` block (carried over from the base calculator) so only the logger remains:

```c
/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;
```

(Remove the `const char _indentationCharacter = ' ';` and `const char _indentationSize = 4;` lines — the Python output to stdout needs no indentation bookkeeping.)

- [ ] **Step 4: Implement the generator header emission + graph construction**

Replace the body of `executeGenerator` in `src/main/c/backend/code-generation/Generator.c` and add emit helpers. Keep the module boilerplate; replace from `/** PUBLIC FUNCTIONS */` down with:

```c
/* OUTPUT SINK */

static void _out(const char * text) {
	fputs(text, stdout);
}

static const char * _boolLiteral(bool value) {
	return value ? "True" : "False";
}

static const char * _nodeAttr(NodeAttr attr) {
	switch (attr) {
		case NODE_ATTR_ROOT: return "root";
		case NODE_ATTR_SOURCE: return "source";
		case NODE_ATTR_SINK: return "sink";
		case NODE_ATTR_TERMINAL: return "terminal";
		default: return NULL;
	}
}

static void _generateGraph(const char * varName, GraphDecl * g) {
	printf("%s = Graph(directed=%s, weighted=%s, capacitated=%s)\n",
		varName,
		_boolLiteral(g->kind == GRAPH_KIND_DIRECTED),
		_boolLiteral(g->traits.weighted),
		_boolLiteral(g->traits.capacitated));
	for (NodeDeclList * it = g->nodes; it != NULL; it = it->next) {
		const char * attr = _nodeAttr(it->value->attr);
		if (attr != NULL) {
			printf("%s.add_node(\"%s\", attr=\"%s\")\n", varName, it->value->id, attr);
		}
		else {
			printf("%s.add_node(\"%s\")\n", varName, it->value->id);
		}
	}
	for (EdgeDeclList * it = g->edges; it != NULL; it = it->next) {
		EdgeDecl * e = it->value;
		printf("%s.add_edge(\"%s\", \"%s\"", varName, e->from, e->to);
		if (e->hasWeight) { printf(", weight=%d", e->weight); }
		if (e->hasCapacity) { printf(", capacity=%d", e->capacity); }
		printf(")\n");
	}
	for (GroupDeclList * it = g->groups; it != NULL; it = it->next) {
		printf("%s.add_group(\"%s\", [", varName, it->value->name);
		bool first = true;
		for (IdList * m = it->value->members; m != NULL; m = m->next) {
			printf("%s\"%s\"", first ? "" : ", ", m->value);
			first = false;
		}
		printf("])\n");
	}
}

static void _generateHeader(void) {
	_out("from nexus_runtime import Graph\n");
	_out("from nexus_runtime import transpose, induced_subgraph, remove_self_loops, underlying\n");
	_out("from nexus_runtime import shortest_path, topological_sort, components, scc, mst, max_flow\n");
	_out("from nexus_runtime import assert_connected, assert_strongly_connected, assert_acyclic\n");
	_out("from nexus_runtime import assert_reachable, assert_tree, assert_binary_tree, assert_forall\n");
	_out("from nexus_runtime import write_dot, write_json\n\n");
}

void executeGenerator(CompilerState * compilerState) {
	Program * program = (Program *) compilerState->abstractSyntaxtTree;
	if (program == NULL) {
		logError(_logger, "Code generation received a null AST.");
		return;
	}
	_generateHeader();
	for (TopLevelDeclList * it = program->decls; it != NULL; it = it->next) {
		TopLevelDecl * decl = it->value;
		if (decl->type == TOP_LEVEL_GRAPH) {
			_generateGraph(decl->graphDecl->id, decl->graphDecl);
			_out("\n");
		}
	}
	logDebugging(_logger, "Code generation complete.");
}
```

(`printf` writes to stdout, matching `_out`. `<stdio.h>` is already included via `Generator.h`.)

- [ ] **Step 5: Verify GREEN**

```bash
src/main/bash/build.sh
src/main/bash/test-codegen.sh
```
Expected: `30-stage-i-integration, runs cleanly` — the emitted Python builds graph `G` and imports the runtime without error. (Derive/analysis lines come in Task 13; the program currently builds only the graphs.)

- [ ] **Step 6: Confirm the existing suite is unaffected**

```bash
src/main/bash/test.sh
```
Expected: accept tests still exit 0 (generator now prints Python to stdout, which `test.sh` discards). Rejects unchanged.

- [ ] **Step 7: Commit**

```bash
git add -A
git commit -m "feat(codegen): emit Python graph construction and add codegen harness"
```

---

## Task 13: Generator — emit derivations and analysis statements

Emit `R = transpose(G)` style derivations and, per analysis, `run`/`export` lines using the runtime functions. Artifact filenames: `<analysisId>_graph.<fmt>` for graph exports and `<analysisId>_<resultId>.<fmt>` for result exports.

**Files:**
- Modify: `src/main/c/backend/code-generation/Generator.c`
- Modify: `src/main/bash/test-codegen.sh`

- [ ] **Step 1: Strengthen the harness to assert artifacts exist**

In `src/main/bash/test-codegen.sh`, replace the success branch of the loop body so it also checks that the integration program's exports were produced:

```bash
	( cd "$WORK" && python3 program.py ) >/dev/null 2>&1
	if [ "$?" == "0" ] && [ -f "$WORK/Main_graph.dot" ] && [ -f "$WORK/Main_path.json" ]; then
		echo -e "    $test, ${GREEN}runs and exports${OFF}"
	else
		STATUS=1; echo -e "    $test, ${RED}runtime error or missing artifacts${OFF}"
	fi
```

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test-codegen.sh
```
Expected: `runtime error or missing artifacts` — analyses/derivations are not emitted yet, so no `Main_graph.dot` / `Main_path.json`.

- [ ] **Step 3: Emit derivations and analyses**

Add emit helpers and call them in `executeGenerator`. Add above `executeGenerator`:

```c
static const char * _transformCall(Transformation * t, const char * src) {
	static char buffer[256];
	switch (t->type) {
		case TRANSFORMATION_TRANSPOSE:
			snprintf(buffer, sizeof(buffer), "transpose(%s)", src);
			break;
		case TRANSFORMATION_INDUCED_SUBGRAPH:
			snprintf(buffer, sizeof(buffer), "induced_subgraph(%s, \"%s\")", src, t->group);
			break;
		case TRANSFORMATION_REMOVE_SELF_LOOPS:
			snprintf(buffer, sizeof(buffer), "remove_self_loops(%s)", src);
			break;
		case TRANSFORMATION_UNDERLYING:
			snprintf(buffer, sizeof(buffer), "underlying(%s)", src);
			break;
	}
	return buffer;
}

static void _generateDerive(DeriveDecl * d) {
	printf("%s = %s\n\n", d->newId, _transformCall(d->transformation, d->fromId));
}

static const char * _algorithmCall(Algorithm * a, const char * graphVar) {
	static char buffer[256];
	const char * name = NULL;
	switch (a->type) {
		case ALGO_SHORTEST_PATH: name = "shortest_path"; break;
		case ALGO_TOPOLOGICAL_SORT: name = "topological_sort"; break;
		case ALGO_COMPONENTS: name = "components"; break;
		case ALGO_SCC: name = "scc"; break;
		case ALGO_MST: name = "mst"; break;
		case ALGO_MAX_FLOW: name = "max_flow"; break;
	}
	if (a->from != NULL && a->to != NULL) {
		snprintf(buffer, sizeof(buffer), "%s(%s, \"%s\", \"%s\")", name, graphVar, a->from, a->to);
	}
	else {
		snprintf(buffer, sizeof(buffer), "%s(%s)", name, graphVar);
	}
	return buffer;
}

static void _generateAnalysis(AnalysisDecl * an) {
	const char * graphVar = an->onGraphId;
	for (AnalysisStmtList * it = an->statements; it != NULL; it = it->next) {
		AnalysisStmt * stmt = it->value;
		if (stmt->type == ANALYSIS_STMT_RUN) {
			RunStmt * run = stmt->run;
			printf("%s = %s\n", run->resultId, _algorithmCall(run->algorithm, graphVar));
		}
		else {
			ExportStmt * ex = stmt->exportStmt;
			const char * fmt = (ex->format == EXPORT_FORMAT_DOT) ? "dot" : "json";
			if (ex->targetType == EXPORT_TARGET_GRAPH) {
				if (ex->format == EXPORT_FORMAT_DOT) {
					printf("write_dot(%s, \"%s_graph.dot\")\n", graphVar, an->id);
				}
				else {
					printf("write_json({\"nodes\": list(%s.nodes), "
						"\"edges\": [(u, v) for u in %s.adj for (v, _w, _c) in %s.adj[u]]}, "
						"\"%s_graph.json\")\n", graphVar, graphVar, graphVar, an->id);
				}
			}
			else {
				printf("write_%s(%s, \"%s_%s.%s\")\n",
					(ex->format == EXPORT_FORMAT_DOT) ? "dot" : "json",
					ex->resultId, an->id, ex->resultId, fmt);
			}
		}
	}
	printf("\n");
}
```

In `executeGenerator`'s loop, handle the other two declaration types:

```c
		if (decl->type == TOP_LEVEL_GRAPH) {
			_generateGraph(decl->graphDecl->id, decl->graphDecl);
			_out("\n");
		}
		else if (decl->type == TOP_LEVEL_DERIVE) {
			_generateDerive(decl->deriveDecl);
		}
		else { /* TOP_LEVEL_ANALYSIS */
			_generateAnalysis(decl->analysisDecl);
		}
```

Add `#include <stdio.h>` if not already present (it is, via `Generator.h`).

Note on `write_dot` for a result export: results are dicts, so DOT export of a result is uncommon; the `write_dot` path above is only emitted for `EXPORT_TARGET_RESULT` + `EXPORT_FORMAT_DOT`. Since `write_dot` expects a Graph, restrict result exports to JSON in the generator by emitting `write_json` for result+dot too:

```c
			else {
				/* Results are data, not graphs: always serialize as JSON. */
				printf("write_json(%s, \"%s_%s.%s\")\n",
					ex->resultId, an->id, ex->resultId, fmt);
			}
```

Use this simpler `else` branch instead of the `write_%s` version above.

- [ ] **Step 4: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test-codegen.sh
```
Expected: `30-stage-i-integration, runs and exports` — `Main_graph.dot` and `Main_path.json` are produced.

- [ ] **Step 5: Eyeball the generated program**

```bash
cat src/test/c/accept/30-stage-i-integration | .build/Nexus 2>/dev/null
```
Expected: a complete Python program — header imports, `G = Graph(directed=True, weighted=True, capacitated=True)` with nodes/edges/group, `R = transpose(G)`, then `path = shortest_path(G, "a", "c")`, `write_dot(G, "Main_graph.dot")`, `write_json(path, "Main_path.json")`.

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "feat(codegen): emit derivations and analysis run/export statements"
```

---

## Task 14: Generator — emit constraints as runtime assertions

Constraints describe instance properties the compiler cannot verify statically (is the graph actually connected, a tree, reachable). Emit them as `assert_*` calls right after the graph they belong to, so the generated program checks them at runtime.

**Files:**
- Modify: `src/main/c/backend/code-generation/Generator.c`
- Modify: `src/main/bash/test-codegen.sh`

- [ ] **Step 1: Add a constraint program to the codegen harness**

In `test-codegen.sh`, change the loop list to cover a constraint-bearing program:

```bash
for test in 30-stage-i-integration 17-constraint-forall; do
```
And relax the artifact assertion so it only applies to the integration program:

```bash
	rc=$?
	ok=1
	[ "$rc" != "0" ] && ok=0
	if [ "$test" == "30-stage-i-integration" ]; then
		{ [ -f "$WORK/Main_graph.dot" ] && [ -f "$WORK/Main_path.json" ]; } || ok=0
	fi
	if [ "$ok" == "1" ]; then
		echo -e "    $test, ${GREEN}runs cleanly${OFF}"
	else
		STATUS=1; echo -e "    $test, ${RED}runtime error or missing artifacts${OFF}"
	fi
```
(Replace the previous success/failure `if` block with this; remove the old artifact-only check. Also reset `cp "$RUNTIME" "$WORK/"` stays; and clear stale artifacts between iterations with `rm -f "$WORK"/Main_*.dot "$WORK"/Main_*.json` at the top of the loop.)

- [ ] **Step 2: Verify RED**

```bash
src/main/bash/build.sh && src/main/bash/test-codegen.sh
```
Expected: `17-constraint-forall` runs but its `forall` constraint is not emitted, so nothing asserts it — it passes vacuously (still "runs cleanly"). To force a real RED, temporarily change `17-constraint-forall`'s predicate to `outdegree(n) = 5` (false for node `b`), rebuild, and confirm it still `runs cleanly` (proving constraints are NOT yet enforced). Revert the edit after observing RED.

- [ ] **Step 3: Emit constraint assertions**

Add a constraint emitter and call it from `_generateGraph`. Add above `_generateGraph`:

```c
static const char * _degreeFnName(DegreeFn fn) {
	switch (fn) {
		case DEGREE_FN_INDEGREE: return "indegree";
		case DEGREE_FN_OUTDEGREE: return "outdegree";
		default: return "degree";
	}
}

static const char * _comparatorLiteral(Comparator cmp) {
	switch (cmp) {
		case CMP_EQ: return "=";
		case CMP_NEQ: return "!=";
		case CMP_GEQ: return ">=";
		case CMP_LEQ: return "<=";
		case CMP_GT: return ">";
		default: return "<";
	}
}

static void _generateConstraints(const char * varName, GraphDecl * g) {
	for (ConstraintList * it = g->constraints; it != NULL; it = it->next) {
		Constraint * c = it->value;
		switch (c->type) {
			case CONSTRAINT_SIMPLE:
				if (c->simple == SIMPLE_CONNECTED) {
					printf("assert_connected(%s)\n", varName);
				}
				else if (c->simple == SIMPLE_STRONGLY_CONNECTED) {
					printf("assert_strongly_connected(%s)\n", varName);
				}
				else {
					printf("assert_acyclic(%s)\n", varName);
				}
				break;
			case CONSTRAINT_REACHABLE:
				printf("assert_reachable(%s, \"%s\", \"%s\")\n",
					varName, c->reachable.from, c->reachable.to);
				break;
			case CONSTRAINT_TREE:
				printf("assert_tree(%s, \"%s\")\n", varName, c->tree.root);
				break;
			case CONSTRAINT_BINARY_TREE:
				printf("assert_binary_tree(%s, \"%s\")\n", varName, c->binaryTree.root);
				break;
			case CONSTRAINT_FORALL:
				printf("assert_forall(%s, \"%s\", \"%s\", \"%s\", %d)\n",
					varName, c->forall.group,
					_degreeFnName(c->forall.predicate->fn),
					_comparatorLiteral(c->forall.predicate->cmp),
					c->forall.predicate->rhs);
				break;
		}
	}
}
```

At the end of `_generateGraph` (after the groups loop), add `_generateConstraints(varName, g);`.

- [ ] **Step 4: Verify GREEN**

```bash
src/main/bash/build.sh && src/main/bash/test-codegen.sh
```
Expected: both programs `run cleanly`. Now re-apply the temporary false predicate from Step 2 (`outdegree(n) = 5`), rebuild, run the generated Python by hand, and confirm it now **raises AssertionError** (proving enforcement); revert afterward:

```bash
cat src/test/c/accept/17-constraint-forall | .build/Nexus 2>/dev/null > /tmp/c.py
cp src/main/c/backend/code-generation/runtime/nexus_runtime.py /tmp/
( cd /tmp && python3 c.py ); echo "exit=$?"
```
Expected with the false predicate: non-zero exit + `AssertionError: constraint failed: outdegree(b) = 5 is false`. With the original `= 0`: exit 0.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(codegen): emit constraints as runtime assertions"
```

---

## Task 15: Integration — broaden codegen coverage and wire into the test flow

Generate and run Python for a representative spread of accept programs (each algorithm + each derivation + dot/json exports), and document the two-command verification.

**Files:**
- Modify: `src/main/bash/test-codegen.sh`
- Modify: `README.md`

- [ ] **Step 1: Expand the codegen test list**

In `test-codegen.sh`, set the loop to a spread that exercises every algorithm and transformation that produces a runnable program:

```bash
for test in \
	18-derive-transpose 19-derive-induced 20-derive-remove-self-loops 21-derive-underlying \
	22-analysis-shortest-path 23-analysis-topological-sort 24-analysis-components \
	25-analysis-scc 26-analysis-mst 27-analysis-max-flow \
	28-export-graph 29-export-result 30-stage-i-integration; do
```
Keep the per-test artifact check only for `30-stage-i-integration` (as in Task 14). All others just need exit 0.

- [ ] **Step 2: Run the full codegen sweep**

```bash
src/main/bash/build.sh && src/main/bash/test-codegen.sh
```
Expected: every listed program reports `runs cleanly`. If any fails, inspect the emitted Python (`cat src/test/c/accept/<name> | .build/Nexus`) and fix the corresponding emit helper. Each algorithm's accept program must satisfy its semantic rule from Task 9 (adjust the accept program's traits/kind if a mismatch surfaces).

- [ ] **Step 3: Document the backend verification in the README**

In `README.md`, under `## Commands`, add a `### Generate` subsection after `### Run`:

```markdown
### Generate

The compiler emits a Python program to standard output. Pair it with the bundled
runtime library to produce the `.dot` / `.json` artifacts:

```bash
src/main/bash/run.sh <program> > program.py
cp src/main/c/backend/code-generation/runtime/nexus_runtime.py .
python3 program.py    # writes <Analysis>_graph.dot, <Analysis>_<result>.json
```

### Test (code generation)

```bash
src/main/bash/test-codegen.sh
```
```

Also update the README intro line: change "Stage II targets the compiler frontend only" to note Stage III adds semantic analysis and Python code generation.

- [ ] **Step 4: Run both test suites**

```bash
src/main/bash/test.sh && src/main/bash/test-codegen.sh
```
Expected: accept (31) exit 0; reject lexical/syntactic (12) + reject-semantic (24) exit non-zero; every codegen program runs cleanly. Both scripts exit 0.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "test(codegen): cover all algorithms and derivations end-to-end"
```

---

## Task 16: Draft the Stage III report

Write the required report as Markdown (export to PDF for delivery). Pull content from `CLAUDE.md` and the implemented behavior.

**Files:**
- Create: `docs/report/Stage-III-Report.md`

- [ ] **Step 1: Write the report draft**

`docs/report/Stage-III-Report.md` with the mandated sections, each filled from the implementation:

```markdown
# Nexus — Diseño e Implementación de un Lenguaje (Stage III)

## 1. Introducción
Purpose of Nexus and what Stage III adds (semantic analysis + code generation).

## 2. Modelo Computacional
### 2.1. Dominio
Graphs: declaration, traits, nodes/attributes, edges, groups, constraints, derivations, analyses.
### 2.2. Lenguaje
The Nexus surface syntax (the example program) and what each construct means.

## 3. Implementación
### 3.1. Frontend
Flex lexer, Bison grammar, AST (reference AbstractSyntaxTree.h). Carried over from Stage II.
### 3.2. Backend
- Tabla de Símbolos y Scopes: IdSet primitive; global graph/analysis ids; per-graph node/group
  sets; per-analysis result ids; the GraphInfo registry; derived-graph signature synthesis.
  Design principle (worth stating explicitly): the symbol table holds only information the
  compiler can actually validate in this domain — identifiers and their kind/trait signature —
  not deeper structure the language never models. Identifier checking is framed as two sets per
  scope, *declared* vs *referenced*: an undeclared reference and a redefinition are the two failure
  modes, mirroring a "defined/used" split.
- Sistema de Tipos (domain reinterpretation): the validation matrix — duplicates, undeclared
  references, trait/operator compatibility, constraint/transformation/algorithm vs kind & traits,
  plus non-empty-construct checks. Note which validations Nexus enforces *syntactically* rather
  than semantically — single fixed sections, mandatory `kind`/`nodes`/`edges`/`on`, algorithm
  argument arity (`shortest_path`/`max_flow` require `from … to …`; the rest forbid it), and no
  nested declarations — a deliberate grammar design choice that shrinks the semantic phase.
- Generación de Código: syntax-directed translation to Python; the generate()/output() structure.
  Output goes to standard output with no indentation bookkeeping (the calculator's indentation
  constants were removed), chosen as the most convenient artifact for a user to capture and run.
- Runtime: nexus_runtime.py bundled as an asset (copied, not inlined); client provides Python 3;
  compile-time validity vs. runtime-asserted instance properties (constraints).
### 3.3. Adicionales (opcional)
DOT and JSON exporters; constraint assertions executable at runtime.
### 3.4. Dificultades Encontradas
Cross-graph node resolution (analyses on derived graphs), the IdSet iterator for derived-node
inheritance, mapping degree predicates to runtime checks, memory ownership under ASan. Deciding
which rules belong in the grammar vs. the semantic phase (Nexus pushes more into the grammar than
a looser language like SQL would, so several checks a peer SQL-targeting group did semantically are
structural here).

## 4. Futuras Extensiones
Type coercion/inference for weighted/capacitated interplay; more algorithms; multi-file output;
richer constraint language; alternative code-gen targets (C, Graphviz-direct).

## 5. Conclusiones
What worked, what the architecture buys, lessons.

## 6. Referencias
Stage I/II docs; Flex-Bison-Compiler v2.0.0 base; course material.

## 7. Bibliografía
Dragon Book and any sources actually consulted.
```

Expand each section into prose during delivery; this is the skeleton to fill, not a placeholder for code.

- [ ] **Step 2: Commit**

```bash
git add -A
git commit -m "docs: draft Stage III backend report"
```

---

## Self-Review Notes

- **Spec coverage:** semantic analysis (symbol table/scopes/type-system) → Tasks 2–10; code generation → Tasks 12–14; runtime → Task 11; report → Task 16. Every validation listed in the design maps to a Task (duplicates 3–4, undeclared refs 5–6, traits/operator 7, constraint/transform 8, algorithm 9, non-empty constructs 10). The compile-time-vs-runtime constraint split is realized in Tasks 8 (static structural checks) and 14 (runtime asserts).
- **Type consistency:** `IdSet` API (`createIdSet`/`idSetAdd`/`idSetContains`/`idSetForEach`/`destroyIdSet`) is used identically across Tasks 2–10. `GraphInfo`/`Analyzer` fields introduced in Task 6 are consumed unchanged in 7–10. Generator helper names (`_generateGraph`, `_generateDerive`, `_generateAnalysis`, `_generateConstraints`) are stable across Tasks 12–14.
- **Reference-group cross-check (applied):** a peer group's backend notes were reviewed; their "defined-vs-used" identifier sets and "symbol table holds only validatable info" framings already match this design (Tasks 3–6), several of their *semantic* checks (single fixed clauses, mandatory clauses, algorithm-argument arity, no nested declarations) are enforced *syntactically* by the Nexus grammar, their "remove unnecessary indentation / emit to stdout" codegen decision is mirrored in Task 12, and their mandatory-clause stance motivated the non-empty checks in Task 10.
- **Known follow-ups to confirm during execution:** (1) verify the project's `logError` variadic signature accepts the `%s`/`%d` format usage as written (it follows the existing `logDebugging(_logger, "...", arg)` pattern); (2) confirm accept tests 22–27 already satisfy the Task 9 algorithm rules, editing traits/kind minimally if not (Task 9 Step 5); (3) the grammar already enforces `run <algorithm> as <id>` arity — `shortest_path`/`max_flow` require `from … to …` and `topological_sort`/`components`/`scc`/`mst` forbid it — so no algorithm-arity semantic check is needed (confirmed against `BisonGrammar.y`).

---

## Execution

Plan complete. Recommended next step: execute task-by-task with `superpowers:subagent-driven-development` (fresh subagent per task, review between commits) or `superpowers:executing-plans` (inline, with checkpoints). Each Task is already scoped to a single commit.
