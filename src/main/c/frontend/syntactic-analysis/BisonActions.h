#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdbool.h>
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/**
 * Bison semantic actions.
 */

Program * ProgramSemanticAction(TopLevelDeclList * decls);
TopLevelDeclList * EmptyTopLevelDeclListSemanticAction();
TopLevelDeclList * AppendTopLevelDeclSemanticAction(TopLevelDeclList * list, TopLevelDecl * decl);

TopLevelDecl * GraphTopLevelDeclSemanticAction(GraphDecl * graphDecl);
TopLevelDecl * DeriveTopLevelDeclSemanticAction(DeriveDecl * deriveDecl);
TopLevelDecl * AnalysisTopLevelDeclSemanticAction(AnalysisDecl * analysisDecl);

GraphKind DirectedKindSemanticAction();
GraphKind UndirectedKindSemanticAction();

GraphTraits EmptyTraitsSemanticAction();
GraphTraits TraitsSemanticAction(bool weighted, bool capacitated);
GraphTraits AddWeightedTraitSemanticAction(GraphTraits traits);
GraphTraits AddCapacitatedTraitSemanticAction(GraphTraits traits);

NodeAttr EmptyNodeAttrSemanticAction();
NodeAttr RootNodeAttrSemanticAction();
NodeAttr SourceNodeAttrSemanticAction();
NodeAttr SinkNodeAttrSemanticAction();
NodeAttr TerminalNodeAttrSemanticAction();

NodeDeclList * EmptyNodeDeclListSemanticAction();
NodeDeclList * AppendNodeDeclSemanticAction(NodeDeclList * list, NodeDecl * decl);
NodeDecl * NodeDeclSemanticAction(char * id, NodeAttr attr);

EdgeOp DirectedEdgeOpSemanticAction();
EdgeOp UndirectedEdgeOpSemanticAction();

EdgeDeclList * EmptyEdgeDeclListSemanticAction();
EdgeDeclList * AppendEdgeDeclSemanticAction(EdgeDeclList * list, EdgeDecl * decl);
EdgeDecl * EdgeDeclSemanticAction(char * from, EdgeOp op, char * to, bool hasWeight, int weight, bool hasCapacity, int capacity);

IdList * SingleIdListSemanticAction(char * id);
IdList * AppendIdListSemanticAction(IdList * list, char * id);

GroupDeclList * EmptyGroupDeclListSemanticAction();
GroupDeclList * AppendGroupDeclSemanticAction(GroupDeclList * list, GroupDecl * decl);
GroupDecl * GroupDeclSemanticAction(char * name, IdList * members);

ConstraintList * EmptyConstraintListSemanticAction();
ConstraintList * AppendConstraintSemanticAction(ConstraintList * list, Constraint * constraint);

Constraint * SimpleConnectedConstraintSemanticAction();
Constraint * SimpleStronglyConnectedConstraintSemanticAction();
Constraint * SimpleAcyclicConstraintSemanticAction();
Constraint * ReachableConstraintSemanticAction(char * from, char * to);
Constraint * TreeConstraintSemanticAction(char * root);
Constraint * BinaryTreeConstraintSemanticAction(char * root);
Constraint * ForallConstraintSemanticAction(char * var, char * group, Predicate * predicate);

DegreeFn IndegreeFnSemanticAction();
DegreeFn OutdegreeFnSemanticAction();
DegreeFn DegreeFnSemanticAction();
Comparator EqComparatorSemanticAction();
Comparator NeqComparatorSemanticAction();
Comparator GeqComparatorSemanticAction();
Comparator LeqComparatorSemanticAction();
Comparator GtComparatorSemanticAction();
Comparator LtComparatorSemanticAction();
Predicate * PredicateSemanticAction(DegreeFn fn, char * var, Comparator cmp, int rhs);

Transformation * TransposeTransformationSemanticAction();
Transformation * UnderlyingTransformationSemanticAction();
Transformation * RemoveSelfLoopsTransformationSemanticAction();
Transformation * InducedSubgraphTransformationSemanticAction(char * group);
DeriveDecl * DeriveDeclSemanticAction(char * newId, char * fromId, Transformation * transformation);

Algorithm * ShortestPathAlgorithmSemanticAction(char * from, char * to);
Algorithm * TopologicalSortAlgorithmSemanticAction();
Algorithm * ComponentsAlgorithmSemanticAction();
Algorithm * SccAlgorithmSemanticAction();
Algorithm * MstAlgorithmSemanticAction();
Algorithm * MaxFlowAlgorithmSemanticAction(char * from, char * to);

AnalysisStmtList * EmptyAnalysisStmtListSemanticAction();
AnalysisStmtList * AppendAnalysisStmtSemanticAction(AnalysisStmtList * list, AnalysisStmt * stmt);

RunStmt * RunStmtSemanticAction(Algorithm * algorithm, char * resultId);
AnalysisStmt * RunAnalysisStmtSemanticAction(RunStmt * runStmt);
ExportStmt * ExportGraphStmtSemanticAction(ExportFormat format);
ExportStmt * ExportResultStmtSemanticAction(char * resultId, ExportFormat format);
AnalysisStmt * ExportAnalysisStmtSemanticAction(ExportStmt * exportStmt);

ExportFormat DotFormatSemanticAction();
ExportFormat JsonFormatSemanticAction();

AnalysisDecl * AnalysisDeclSemanticAction(char * id, char * onGraphId, AnalysisStmtList * statements);

GraphDecl * GraphDeclSemanticAction(char * id, GraphKind kind, GraphTraits traits, NodeDeclList * nodes, EdgeDeclList * edges, GroupDeclList * groups, ConstraintList * constraints);

#endif
