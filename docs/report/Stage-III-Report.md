# Nexus — Informe Stage III

## 1. Introducción

Nexus es un compilador de un DSL (Domain-Specific Language) orientado a grafos, desarrollado para la materia **Autómatas, Teoría de Lenguajes y Compiladores** (ITBA). El proyecto se entrega en tres etapas:

1. **Stage I — Diseño:** Concepción del dominio, la gramática y la sintaxis del lenguaje.
2. **Stage II — Frontend:** Análisis léxico (Flex), análisis sintáctico (Bison) y construcción del AST.
3. **Stage III — Backend:** Análisis semántico, generación de código y este informe.

El objetivo de Stage III es completar el pipeline del compilador: validar el programa contra las reglas del dominio (análisis semántico), emitir código ejecutable (generación de código) y documentar las decisiones de diseño e implementación.

El compilador acepta programas Nexus que declaran grafos con nodos, aristas, grupos y restricciones; definen derivaciones (transformaciones de grafos); y especifican análisis (algoritmos de grafos con exportación de resultados). La salida es un programa Python que, junto con la biblioteca runtime `nexus_runtime.py`, construye los grafos, valida las restricciones, ejecuta los algoritmos y exporta los resultados a formato DOT o JSON.

---

## 2. Modelo Computacional

### 2.1 Dominio

El dominio de Nexus es el **modelado, validación y análisis de grafos**. El lenguaje permite:

- **Declarar grafos** dirigidos o no dirigidos, con traits opcionales (`weighted`, `capacitated`).
- **Definir estructura:** nodos con atributos opcionales (`root`, `source`, `sink`, `terminal`), aristas con peso y/o capacidad, y grupos de nodos.
- **Especificar restricciones** sobre la estructura del grafo: conectividad, aciclicidad, alcanzabilidad, estructura de árbol, y predicados cuantitativos sobre grado.
- **Derivar nuevos grafos** mediante transformaciones: transposición, subgrafo inducido, eliminación de self-loops, y grafo subyacente (undirected).
- **Ejecutar algoritmos:** camino mínimo (Dijkstra), orden topológico (Kahn), componentes conexas (BFS), componentes fuertemente conexas (Tarjan), árbol de expansión mínima (Kruskal), flujo máximo (Edmonds-Karp).
- **Exportar resultados** a DOT (para visualización con Graphviz) o JSON.

### 2.2 Lenguaje

Un programa Nexus consiste en una o más declaraciones de nivel superior: `graph`, `derive` y `analysis`.

```nexus
graph Build:
    kind directed
    traits weighted
    nodes:
        lexer
        parser
        ast
        emit
    edges:
        lexer -> parser weight 2
        parser -> ast weight 3
        ast -> emit weight 4
    groups:
        core = {lexer, parser, ast}
        sinks = {emit}
    constraints:
        assert acyclic
        assert reachable lexer -> emit
        assert forall n in sinks: outdegree(n) = 0

derive CoreBuild from Build using induced_subgraph(core)

analysis Main on Build:
    run shortest_path from lexer to emit as critical_path
    export graph to dot
    export result critical_path to json
```

**Construcciones del lenguaje:**

| Construcción | Significado |
|:---|:---|
| `graph <id>:` | Declara un grafo con un identificador único |
| `kind directed\|undirected` | Tipo del grafo (obligatorio) |
| `traits weighted capacitated` | Propiedades opcionales del grafo |
| `nodes:` | Sección de declaración de nodos |
| `<id> [root\|source\|sink\|terminal]` | Nodo con atributo opcional |
| `edges:` | Sección de declaración de aristas |
| `<from> -> <to> [weight N] [capacity N]` | Arista dirigida con peso/capacidad opcional |
| `<from> -- <to> [weight N] [capacity N]` | Arista no dirigida con peso/capacidad opcional |
| `groups:` | Sección de declaración de grupos |
| `<id> = {<n1>, <n2>, ...}` | Grupo nombrado de nodos |
| `constraints:` | Sección de restricciones |
| `assert connected` | Grafo conexo (solo undirected) |
| `assert strongly_connected` | Fuertemente conexo (solo directed) |
| `assert acyclic` | Grafo acíclico |
| `assert reachable <u> -> <v>` | Existe camino de u a v |
| `assert tree rooted_at <r>` | Es árbol con raíz r (solo directed) |
| `assert binary_tree rooted_at <r>` | Es árbol binario con raíz r (solo directed) |
| `assert forall <v> in <group>: <pred>` | Predicado de grado para todo nodo del grupo |
| `derive <id> from <src> using <transform>` | Crea un nuevo grafo derivado |
| `analysis <id> on <graph>:` | Bloque de análisis sobre un grafo |
| `run <algo> [from <u> to <v>] as <result>` | Ejecuta un algoritmo |
| `export graph to dot\|json` | Exporta el grafo |
| `export result <id> to json` | Exporta un resultado |

**Predicados de grado:** `degree(n)`, `indegree(n)`, `outdegree(n)` comparados con `= != >= <= < >` y un valor entero.

**Transformaciones:** `transpose`, `induced_subgraph(<group>)`, `remove_self_loops`, `underlying`.

**Algoritmos:** `shortest_path`, `topological_sort`, `components`, `scc`, `mst`, `max_flow`.

---

## 3. Implementación

### 3.1 Frontend (llevado de Stage II)

El frontend fue implementado en Stage II y no sufrió modificaciones en Stage III.

**Análisis léxico (Flex):** El lexer (`FlexPatterns.l`) reconoce los tokens del lenguaje: palabras reservadas (`graph`, `kind`, `directed`, `undirected`, `traits`, `weighted`, `capacitated`, `nodes`, `edges`, `groups`, `constraints`, `assert`, `connected`, `strongly_connected`, `acyclic`, `reachable`, `tree`, `binary_tree`, `rooted_at`, `forall`, `in`, `derive`, `from`, `using`, `transpose`, `induced_subgraph`, `remove_self_loops`, `underlying`, `analysis`, `on`, `run`, `as`, `export`, `result`, `to`, `dot`, `json`, `root`, `source`, `sink`, `terminal`, `degree`, `indegree`, `outdegree`, `shortest_path`, `topological_sort`, `components`, `scc`, `mst`, `max_flow`, `weight`, `capacity`), operadores (`->`, `--`, `=`, `!=`, `>=`, `<=`, `<`, `>`), delimitadores (`:`, `{`, `}`, `,`, `(`, `)`), identificadores (`[a-zA-Z_][a-zA-Z0-9_]*`), enteros, y comentarios de línea (`//`).

**Análisis sintáctico (Bison):** La gramática (`BisonGrammar.y`) define las producciones del lenguaje y las acciones semánticas que construyen el AST. La gramática es LALR(1), libre de conflictos.

**AST (`AbstractSyntaxTree.h/c`):** Define los tipos del árbol con un diseño de unión etiquetada (tagged union). El nodo raíz es `Program { TopLevelDeclList * decls }`. Cada `TopLevelDecl` es una unión sobre `GraphDecl`, `DeriveDecl` y `AnalysisDecl`. Se incluyen constructores, destructores y cobertura de `%destructor` en Bison para evitar memory leaks. El AST es el contrato de entrada para Stage III.

### 3.2 Backend

#### 3.2.1 Tabla de Símbolos

La tabla de símbolos se implementa mediante `IdSet`, un hash-set de strings con resolución de colisiones por encadenamiento (`IdSet.h/c`, 75 líneas). Ofrece tres operaciones:

- `idSetAdd(set, id)` — inserta una copia del identificador; retorna `false` si ya existía (detecta duplicados).
- `idSetContains(set, id)` — consulta si el identificador está presente.
- `idSetForEach(set, visit, ctx)` — itera sobre todos los identificadores (usado para copiar nodos/grupos a grafos derivados).

El analizador semántico (`SemanticAnalyzer.c`) mantiene las siguientes tablas:

| Tabla | Scope | Contenido |
|:---|:---|:---|
| `graphIds` | Global | Identificadores de grafos declarados y derivados |
| `analysisIds` | Global | Identificadores de análisis |
| `GraphInfo.nodes` | Per-graph | Nodos declarados en cada grafo |
| `GraphInfo.groups` | Per-graph | Grupos declarados en cada grafo |
| `results` (local) | Per-analysis | Resultados de `run ... as <id>` |

Adicionalmente, `GraphInfo` almacena el `kind` y `traits` de cada grafo en un registro enlazado, permitiendo validar operaciones cross-graph (e.g., derivaciones y análisis que referencian grafos por nombre).

#### 3.2.2 Sistema de Tipos (Graph-Property Checking)

En Nexus no existe un sistema de tipos tradicional con coerción o inferencia. El "sistema de tipos" se reinterpreta como **validación de propiedades de grafos**: cada operación tiene precondiciones sobre el kind y traits del grafo objetivo. La siguiente matriz resume las validaciones implementadas:

**Validaciones de identificadores:**

| Validación | Ubicación |
|:---|:---|
| Identificador de grafo duplicado | `_checkTopLevel` |
| Identificador de análisis duplicado | `_checkTopLevel` |
| Identificador de nodo duplicado | `_checkGraphScope` |
| Identificador de grupo duplicado | `_checkGraphScope` |
| Identificador de resultado duplicado | `_checkAnalysisScope` |
| Arista paralela duplicada | `_checkGraphScope` |

**Validaciones de referencias:**

| Validación | Ubicación |
|:---|:---|
| Endpoint de arista no declarado | `_checkGraphScope` |
| Miembro de grupo no declarado | `_checkGraphScope` |
| Grafo fuente de derive no declarado | `_checkTopLevel` |
| Grafo de análisis no declarado | `_checkAnalysisScope` |
| Nodo de constraint no declarado | `_checkConstraints` |
| Grupo de forall no declarado | `_checkConstraints` |
| Endpoint de run no declarado | `_checkAnalysisScope` |
| Resultado de export no declarado | `_checkAnalysisScope` |

**Validaciones de traits y operadores:**

| Validación | Ubicación |
|:---|:---|
| `weight` sin trait `weighted` | `_checkGraphScope` |
| `capacity` sin trait `capacitated` | `_checkGraphScope` |
| `->` en grafo `undirected` | `_checkGraphScope` |
| `--` en grafo `directed` | `_checkGraphScope` |

**Validaciones de constraint vs. kind:**

| Validación | Ubicación |
|:---|:---|
| `connected` en grafo `directed` | `_checkConstraints` |
| `strongly_connected` en grafo `undirected` | `_checkConstraints` |
| `tree` en grafo `undirected` | `_checkConstraints` |
| `binary_tree` en grafo `undirected` | `_checkConstraints` |
| `tree rooted_at` o `binary_tree rooted_at` distinto del nodo `root` declarado | `_checkConstraints` |
| `assert acyclic` sobre grafos con ciclos o self-loops | `_checkConstraints` |

**Validaciones de atributos de nodo:**

| Validación | Ubicación |
|:---|:---|
| `source` en grafo `undirected` | `_checkGraphScope` |
| `sink` en grafo `undirected` | `_checkGraphScope` |
| Múltiples nodos `root` | `_checkGraphScope` |
| Múltiples nodos `source` | `_checkGraphScope` |
| Múltiples nodos `sink` | `_checkGraphScope` |
| `root` con indegree distinto de cero en grafo dirigido | `_checkGraphScope` |
| `source` con indegree distinto de cero u outdegree nulo | `_checkGraphScope` |
| `sink` con outdegree distinto de cero o indegree nulo | `_checkGraphScope` |
| `terminal` con salidas en grafo dirigido o grado distinto de uno en grafo no dirigido | `_checkGraphScope` |

**Validaciones de transformación vs. kind:**

| Validación | Ubicación |
|:---|:---|
| `transpose` en grafo `undirected` | `_checkTopLevel` |
| `underlying` en grafo `undirected` | `_checkTopLevel` |
| `induced_subgraph` con grupo no declarado | `_checkTopLevel` |

**Validaciones de algoritmo vs. kind/traits:**

| Validación | Ubicación |
|:---|:---|
| `topological_sort` en grafo `undirected` | `_checkAlgorithm` |
| `topological_sort` en grafo cíclico | `_checkAlgorithm` |
| `scc` en grafo `undirected` | `_checkAlgorithm` |
| `components` en grafo `directed` | `_checkAlgorithm` |
| `mst` en grafo `directed` | `_checkAlgorithm` |
| `mst` en grafo sin `weighted` | `_checkAlgorithm` |
| `max_flow` en grafo `undirected` | `_checkAlgorithm` |
| `max_flow` sin `capacitated` | `_checkAlgorithm` |
| `max_flow` con endpoints distintos de `source`/`sink` declarados | `_checkAlgorithm` |
| `shortest_path` sin `weighted` | `_checkAlgorithm` |

**Validaciones estructurales:**

| Validación | Ubicación |
|:---|:---|
| Grafo sin nodos | `_checkGraphScope` |
| Análisis sin statements | `_checkAnalysisScope` |

En total se implementaron **47 validaciones semánticas** con tests de rechazo dedicados para identificadores, referencias, compatibilidad de traits/kinds, atributos estructurales de nodos, aristas paralelas, constraints y algoritmos.

#### 3.2.3 Generación de Código

El generador (`Generator.c`, 252 líneas) realiza un tree-walk syntax-directed sobre el AST, emitiendo Python a stdout mediante `printf`/`fputs`. No requiere indentación ni manejo de scopes en la salida porque Python no necesita bloques para las operaciones generadas (todo es secuencial a nivel de módulo).

La función principal `executeGenerator` recorre la lista de `TopLevelDecl` y despacha a:

- `_generateGraph(varName, g)` — emite la construcción del grafo: `Graph(directed=..., weighted=..., capacitated=...)`, luego `add_node`, `add_edge`, `add_group` y las constraint assertions.
- `_generateDerive(d)` — emite una asignación con la llamada a la transformación correspondiente.
- `_generateAnalysis(an)` — emite los `run` statements como asignaciones al resultado del algoritmo, y los `export` statements como llamadas a `write_dot` o `write_json`.

El header generado importa todos los símbolos de `nexus_runtime`:

```python
from nexus_runtime import Graph
from nexus_runtime import transpose, induced_subgraph, remove_self_loops, underlying
from nexus_runtime import shortest_path, topological_sort, components, scc, mst, max_flow
from nexus_runtime import assert_connected, assert_strongly_connected, assert_acyclic
from nexus_runtime import assert_reachable, assert_tree, assert_binary_tree, assert_forall
from nexus_runtime import write_dot, write_json
```

**Decisión de diseño:** Se optó por emitir un programa Python plano (sin clases, funciones o indentación) porque el modelo de ejecución de Nexus es estrictamente secuencial: declarar, derivar, analizar. Esto simplifica el generador y hace que la salida sea directamente legible.

#### 3.2.4 Runtime

El runtime (`nexus_runtime.py`, 430 líneas) es una biblioteca Python stdlib-only que se distribuye como asset junto al compilador. No se enlaza al compilador; es una dependencia del programa generado.

**Componentes del runtime:**

| Componente | Función |
|:---|:---|
| `Graph` | Clase principal: `add_node`, `add_edge`, `add_group`, `neighbors`. Representación interna: lista de adyacencia con diccionarios `{to, weight, capacity}`. |
| Derivaciones | `transpose`, `induced_subgraph`, `remove_self_loops`, `underlying` — retornan un nuevo `Graph`. |
| Algoritmos | `shortest_path` (Dijkstra), `topological_sort` (Kahn), `components` (BFS), `scc` (Tarjan), `mst` (Kruskal con Union-Find), `max_flow` (Edmonds-Karp). |
| Assertions | `assert_connected`, `assert_strongly_connected`, `assert_acyclic`, `assert_reachable`, `assert_tree`, `assert_binary_tree`, `assert_forall` — lanzan `AssertionError` si la condición no se cumple. |
| Exportadores | `write_dot` (genera `.dot` para grafos), `write_result_dot` (genera `.dot` para resultados), `write_json` (serializa a JSON). |

**Compile-time vs. runtime assertions:** Las restricciones del lenguaje se validan en dos momentos:
- **Compile-time:** El analizador semántico verifica que las restricciones sean válidas para el tipo de grafo (e.g., `tree` solo en directed). Esto previene errores de tipo.
- **Runtime:** Las assertions ejecutables verifican que el grafo concreto cumple la restricción (e.g., que efectivamente es un árbol). Esto previene errores de valor.

### 3.3 Adicionales

**Exportador DOT:** El runtime genera archivos `.dot` compatibles con Graphviz, incluyendo atributos de peso y capacidad como labels en las aristas, y atributos de nodo como tooltip/label.

**Exportador JSON:** Serializa grafos como `{nodes: [...], edges: [...]}` y resultados de algoritmos como diccionarios/listas Python directamente a JSON.

**Grafos derivados en el análisis semántico:** Cuando se procesa un `derive`, el analizador sintetiza un `GraphInfo` para el grafo derivado, copiando los nodos y grupos del grafo fuente. Esto permite que análisis posteriores referencien grafos derivados con validación completa. La transformación `underlying` además cambia el kind a `undirected`.

### 3.4 Dificultades Encontradas

1. **Resolución de nodos cross-graph:** Los grafos derivados necesitan conocer los nodos del grafo fuente para validar análisis posteriores. Se resolvió con `idSetForEach` para copiar los conjuntos de nodos y grupos al `GraphInfo` del derivado.

2. **Debug logs contaminando stdout:** El sistema de logging del proyecto base envía DEBUG/INFO a stdout. Como el generador también emite a stdout, los logs se mezclaban con el código Python generado. Se resolvió usando la variable de entorno `LOGGING_LEVEL=CRITICAL` al capturar la salida del compilador.

3. **Predicados de grado a runtime:** Los predicados `forall n in group: degree(n) >= K` no pueden verificarse en compile-time porque requieren conocer la estructura concreta del grafo. Se delegaron al runtime como assertions ejecutables que reciben el grafo, el nombre del grupo, la función de grado, el comparador y el valor esperado como strings.

4. **Memory ownership bajo Bison:** El AST mantiene ownership de todos los strings (duplicados con `strdup` en las acciones semánticas). El analizador semántico y el generador usan punteros prestados al AST, sin liberar ni duplicar. Los `IdSet` sí duplican las keys internamente para independencia del AST.

5. **Decisiones grammar vs. semantic:** Algunas restricciones podrían haberse implementado en la gramática (e.g., forzar `edges:` después de `nodes:`), pero se optó por una gramática permisiva con validación semántica posterior. Esto simplifica la gramática y produce mejores mensajes de error.

---

## 4. Futuras Extensiones

- **Type coercion:** Permitir `shortest_path` en grafos no weighted asumiendo peso unitario.
- **Más algoritmos:** Dijkstra con nodo fuente único, BFS/DFS traversal, detección de ciclos con reporte del ciclo encontrado, algoritmo de flujo de costo mínimo.
- **Multi-file output:** Emitir cada análisis a un archivo separado en vez de un único programa.
- **Constraint language más rico:** Permitir combinaciones lógicas (`and`, `or`, `not`) de constraints, y predicados sobre propiedades de aristas.
- **Targets alternativos:** Además de Python, emitir C++, Rust, o directamente archivos DOT/JSON sin runtime.
- **Importación de grafos:** Permitir cargar grafos desde archivos DOT o JSON existentes.
- **Visualización interactiva:** Generar HTML con visualización interactiva del grafo (e.g., usando D3.js o Cytoscape.js).

---

## 5. Conclusiones

El compilador Nexus implementa exitosamente las tres etapas del proyecto: un frontend libre de conflictos que construye un AST completo, un analizador semántico con 47 validaciones que cubren identificadores, referencias, traits, operadores, constraints, transformaciones, algoritmos y reglas estructurales de grafos, y un generador de código que produce programas Python ejecutables.

La decisión de generar Python con un runtime stdlib-only resultó acertada: permite ejecutar los programas generados sin dependencias externas (salvo Python 3 y opcionalmente Graphviz para visualizar los DOT), y facilitó el testing end-to-end de la generación de código.

El enfoque TDD (write test, verify RED, implement, verify GREEN) fue especialmente efectivo para las validaciones semánticas, donde cada validación tiene un test de rechazo dedicado que documenta exactamente qué programa inválido se detecta.

La suite de tests final incluye 35 tests de aceptación, 12 tests de rechazo sintáctico, 47 tests de rechazo semántico, y 24 tests de generación de código end-to-end que verifican que los programas generados se ejecutan correctamente y producen los artefactos esperados.

---

## 6. Referencias

- **Stage I — Diseño del lenguaje Nexus:** `TLA_Stage_I.pdf` (documento de diseño original).
- **Stage II — Frontend:** Branch `main`, commit `e0807e3`.
- **Flex-Bison-Compiler v2.0.0:** https://github.com/agustin-golmar/Flex-Bison-Compiler/tree/v2.0.0 — proyecto base.
- **Material de la cátedra:** Slides de análisis semántico, tabla de símbolos, sistema de tipos y generación de código.

---

## 7. Bibliografía

- Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2006). *Compilers: Principles, Techniques, and Tools* (2nd ed.). Addison-Wesley.
- Levine, J. (2009). *flex & bison*. O'Reilly Media.
- Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2009). *Introduction to Algorithms* (3rd ed.). MIT Press.
