#include "Generator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: Generator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeGeneratorModule() {
	_logger = createLogger("Generator");
	return _shutdownGeneratorModule;
}

/* OUTPUT HELPERS */

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

/* GRAPH CONSTRUCTION */

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

/* HEADER */

static void _generateHeader(void) {
	_out("from nexus_runtime import Graph\n");
	_out("from nexus_runtime import transpose, induced_subgraph, remove_self_loops, underlying\n");
	_out("from nexus_runtime import shortest_path, topological_sort, components, scc, mst, max_flow\n");
	_out("from nexus_runtime import assert_connected, assert_strongly_connected, assert_acyclic\n");
	_out("from nexus_runtime import assert_reachable, assert_tree, assert_binary_tree, assert_forall\n");
	_out("from nexus_runtime import write_dot, write_json\n\n");
}

/** PUBLIC FUNCTIONS */

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
