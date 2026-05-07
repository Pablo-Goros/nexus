#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

typedef struct Program Program;

typedef struct TopLevelDecl TopLevelDecl;
typedef struct TopLevelDeclList TopLevelDeclList;

typedef struct GraphDecl GraphDecl;
typedef struct DeriveDecl DeriveDecl;
typedef struct AnalysisDecl AnalysisDecl;

typedef struct NodeDecl NodeDecl;
typedef struct NodeDeclList NodeDeclList;

typedef struct EdgeDecl EdgeDecl;
typedef struct EdgeDeclList EdgeDeclList;

typedef struct GroupDecl GroupDecl;
typedef struct GroupDeclList GroupDeclList;

typedef struct IdList IdList;

typedef struct Constraint Constraint;
typedef struct ConstraintList ConstraintList;

typedef struct Predicate Predicate;

typedef struct Transformation Transformation;

typedef struct AnalysisStmt AnalysisStmt;
typedef struct AnalysisStmtList AnalysisStmtList;

typedef struct RunStmt RunStmt;
typedef struct ExportStmt ExportStmt;
typedef struct Algorithm Algorithm;

typedef enum {
	TOP_LEVEL_GRAPH,
	TOP_LEVEL_DERIVE,
	TOP_LEVEL_ANALYSIS
} TopLevelDeclType;

typedef enum {
	GRAPH_KIND_DIRECTED,
	GRAPH_KIND_UNDIRECTED
} GraphKind;

typedef enum {
	NODE_ATTR_NONE,
	NODE_ATTR_ROOT,
	NODE_ATTR_SOURCE,
	NODE_ATTR_SINK,
	NODE_ATTR_TERMINAL
} NodeAttr;

typedef enum {
	EDGE_OP_DIRECTED,
	EDGE_OP_UNDIRECTED
} EdgeOp;

typedef enum {
	SIMPLE_CONNECTED,
	SIMPLE_STRONGLY_CONNECTED,
	SIMPLE_ACYCLIC
} SimpleConstraintType;

typedef enum {
	CONSTRAINT_SIMPLE,
	CONSTRAINT_REACHABLE,
	CONSTRAINT_TREE,
	CONSTRAINT_BINARY_TREE,
	CONSTRAINT_FORALL
} ConstraintType;

typedef enum {
	DEGREE_FN_INDEGREE,
	DEGREE_FN_OUTDEGREE,
	DEGREE_FN_DEGREE
} DegreeFn;

typedef enum {
	CMP_EQ,
	CMP_NEQ,
	CMP_GEQ,
	CMP_LEQ,
	CMP_GT,
	CMP_LT
} Comparator;

typedef enum {
	TRANSFORMATION_TRANSPOSE,
	TRANSFORMATION_INDUCED_SUBGRAPH,
	TRANSFORMATION_REMOVE_SELF_LOOPS,
	TRANSFORMATION_UNDERLYING
} TransformationType;

typedef enum {
	ALGO_SHORTEST_PATH,
	ALGO_TOPOLOGICAL_SORT,
	ALGO_COMPONENTS,
	ALGO_SCC,
	ALGO_MST,
	ALGO_MAX_FLOW
} AlgorithmType;

typedef enum {
	EXPORT_TARGET_GRAPH,
	EXPORT_TARGET_RESULT
} ExportTargetType;

typedef enum {
	EXPORT_FORMAT_DOT,
	EXPORT_FORMAT_JSON
} ExportFormat;

typedef enum {
	ANALYSIS_STMT_RUN,
	ANALYSIS_STMT_EXPORT
} AnalysisStmtType;

typedef struct {
	bool weighted;
	bool capacitated;
} GraphTraits;

struct Program {
	TopLevelDeclList * decls;
};

struct TopLevelDecl {
	TopLevelDeclType type;
	union {
		GraphDecl * graphDecl;
		DeriveDecl * deriveDecl;
		AnalysisDecl * analysisDecl;
	};
};

struct TopLevelDeclList {
	TopLevelDecl * value;
	TopLevelDeclList * next;
};

struct NodeDecl {
	char * id;
	NodeAttr attr;
};

struct NodeDeclList {
	NodeDecl * value;
	NodeDeclList * next;
};

struct EdgeDecl {
	char * from;
	char * to;
	EdgeOp op;
	bool hasWeight;
	int weight;
	bool hasCapacity;
	int capacity;
};

struct EdgeDeclList {
	EdgeDecl * value;
	EdgeDeclList * next;
};

struct IdList {
	char * value;
	IdList * next;
};

struct GroupDecl {
	char * name;
	IdList * members;
};

struct GroupDeclList {
	GroupDecl * value;
	GroupDeclList * next;
};

struct Predicate {
	DegreeFn fn;
	char * var;
	Comparator cmp;
	int rhs;
};

struct Constraint {
	ConstraintType type;
	union {
		SimpleConstraintType simple;
		struct {
			char * from;
			char * to;
		} reachable;
		struct {
			char * root;
		} tree;
		struct {
			char * root;
		} binaryTree;
		struct {
			char * var;
			char * group;
			Predicate * predicate;
		} forall;
	};
};

struct ConstraintList {
	Constraint * value;
	ConstraintList * next;
};

struct Transformation {
	TransformationType type;
	char * group; /* Only for induced_subgraph. */
};

struct DeriveDecl {
	char * newId;
	char * fromId;
	Transformation * transformation;
};

struct Algorithm {
	AlgorithmType type;
	char * from;
	char * to;
};

struct RunStmt {
	Algorithm * algorithm;
	char * resultId;
};

struct ExportStmt {
	ExportTargetType targetType;
	char * resultId; /* Only for EXPORT_TARGET_RESULT. */
	ExportFormat format;
};

struct AnalysisStmt {
	AnalysisStmtType type;
	union {
		RunStmt * run;
		ExportStmt * exportStmt;
	};
};

struct AnalysisStmtList {
	AnalysisStmt * value;
	AnalysisStmtList * next;
};

struct AnalysisDecl {
	char * id;
	char * onGraphId;
	AnalysisStmtList * statements;
};

struct GraphDecl {
	char * id;
	GraphKind kind;
	GraphTraits traits;
	NodeDeclList * nodes;
	EdgeDeclList * edges;
	GroupDeclList * groups;
	ConstraintList * constraints;
};

/* Constructors */

Program * createProgram(TopLevelDeclList * decls);
TopLevelDecl * createTopLevelGraphDecl(GraphDecl * graphDecl);
TopLevelDecl * createTopLevelDeriveDecl(DeriveDecl * deriveDecl);
TopLevelDecl * createTopLevelAnalysisDecl(AnalysisDecl * analysisDecl);
TopLevelDeclList * prependTopLevelDecl(TopLevelDeclList * list, TopLevelDecl * value);
TopLevelDeclList * reverseTopLevelDeclList(TopLevelDeclList * list);

GraphDecl * createGraphDecl(char * id, GraphKind kind, GraphTraits traits, NodeDeclList * nodes, EdgeDeclList * edges, GroupDeclList * groups, ConstraintList * constraints);
GraphTraits createGraphTraits(bool weighted, bool capacitated);

NodeDecl * createNodeDecl(char * id, NodeAttr attr);
NodeDeclList * prependNodeDecl(NodeDeclList * list, NodeDecl * value);
NodeDeclList * reverseNodeDeclList(NodeDeclList * list);

EdgeDecl * createEdgeDecl(char * from, EdgeOp op, char * to, bool hasWeight, int weight, bool hasCapacity, int capacity);
EdgeDeclList * prependEdgeDecl(EdgeDeclList * list, EdgeDecl * value);
EdgeDeclList * reverseEdgeDeclList(EdgeDeclList * list);

IdList * prependId(IdList * list, char * value);
IdList * reverseIdList(IdList * list);

GroupDecl * createGroupDecl(char * name, IdList * members);
GroupDeclList * prependGroupDecl(GroupDeclList * list, GroupDecl * value);
GroupDeclList * reverseGroupDeclList(GroupDeclList * list);

Predicate * createPredicate(DegreeFn fn, char * var, Comparator cmp, int rhs);

Constraint * createSimpleConstraint(SimpleConstraintType type);
Constraint * createReachableConstraint(char * from, char * to);
Constraint * createTreeConstraint(char * root);
Constraint * createBinaryTreeConstraint(char * root);
Constraint * createForallConstraint(char * var, char * group, Predicate * predicate);
ConstraintList * prependConstraint(ConstraintList * list, Constraint * value);
ConstraintList * reverseConstraintList(ConstraintList * list);

Transformation * createTransformation(TransformationType type, char * group);
DeriveDecl * createDeriveDecl(char * newId, char * fromId, Transformation * transformation);

Algorithm * createAlgorithm(AlgorithmType type, char * from, char * to);
RunStmt * createRunStmt(Algorithm * algorithm, char * resultId);
ExportStmt * createExportGraphStmt(ExportFormat format);
ExportStmt * createExportResultStmt(char * resultId, ExportFormat format);
AnalysisStmt * createRunAnalysisStmt(RunStmt * runStmt);
AnalysisStmt * createExportAnalysisStmt(ExportStmt * exportStmt);
AnalysisStmtList * prependAnalysisStmt(AnalysisStmtList * list, AnalysisStmt * value);
AnalysisStmtList * reverseAnalysisStmtList(AnalysisStmtList * list);
AnalysisDecl * createAnalysisDecl(char * id, char * onGraphId, AnalysisStmtList * statements);

/* Destructors */

void destroyProgram(Program * program);
void destroyTopLevelDecl(TopLevelDecl * decl);
void destroyTopLevelDeclList(TopLevelDeclList * list);

void destroyGraphDecl(GraphDecl * graphDecl);
void destroyDeriveDecl(DeriveDecl * deriveDecl);
void destroyAnalysisDecl(AnalysisDecl * analysisDecl);

void destroyNodeDecl(NodeDecl * nodeDecl);
void destroyNodeDeclList(NodeDeclList * list);

void destroyEdgeDecl(EdgeDecl * edgeDecl);
void destroyEdgeDeclList(EdgeDeclList * list);

void destroyIdList(IdList * list);

void destroyGroupDecl(GroupDecl * groupDecl);
void destroyGroupDeclList(GroupDeclList * list);

void destroyPredicate(Predicate * predicate);
void destroyConstraint(Constraint * constraint);
void destroyConstraintList(ConstraintList * list);

void destroyTransformation(Transformation * transformation);

void destroyAlgorithm(Algorithm * algorithm);
void destroyRunStmt(RunStmt * runStmt);
void destroyExportStmt(ExportStmt * exportStmt);
void destroyAnalysisStmt(AnalysisStmt * stmt);
void destroyAnalysisStmtList(AnalysisStmtList * list);

#endif
