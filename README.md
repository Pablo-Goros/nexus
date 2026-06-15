[![Release](https://img.shields.io/badge/Release-v2.0.0-ffb600.svg?style=for-the-badge)](https://github.com/Pablo-Goros/nexus/releases)

[![CI](https://github.com/Pablo-Goros/nexus/actions/workflows/pipeline.yaml/badge.svg?branch=production)](https://github.com/Pablo-Goros/nexus/actions/workflows/pipeline.yaml)

# Nexus

Nexus is a graph-oriented DSL compiler project for Automatas, Teoria de Lenguajes y Compiladores.

Nexus is a complete compiler: a Flex/Bison frontend (lexical analysis, syntactic analysis, AST construction) plus a backend with semantic analysis and Python code generation. The project is built from the required `Flex-Bison-Compiler` `v2.0.0` base and keeps its Docker, CMake, and shell-script workflow.

* [Requirements](#requirements)
* [Configuration](#configuration)
* [Commands](#commands)
* [CI/CD](#cicd)
* [Recommended Extensions](#recommended-extensions)

## Requirements

* [Docker v28.3.2](https://www.docker.com/)
* Python 3 — to execute the programs emitted by the compiler.
* [Graphviz](https://graphviz.org/) (optional) — to render exported `.dot` files as images.

## Configuration

Set the following environment variables to control and configure the behaviour of the application:

| Name                  | Default | Description                                                                                                                                                           |
| :-------------------- | :-----: | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ENVIRONMENT`         | `Local` | The active environment name. The available environments are: `Local`, `Development` and `Production`.                                                                 |
| `LOG_IGNORED_LEXEMES` | `true`  | When `true`, logs all of the ignored lexemes found with Flex at `DEBUGGING` level. To remove those logs from the console output set it to `false`.                    |
| `LOGGING_LEVEL`       | `ALL`   | The minimum level to log in the console output. From lower to higher, the available levels are: `ALL`, `DEBUGGING`, `INFORMATION`, `WARNING`, `ERROR` and `CRITICAL`. |

_Docker Compose_ can read the variables from an `.env` file too (see `compose.yaml` file).

## Commands

### Start

Rises an ephemeral container, ready to start development:

```bash
docker compose run --rm compiler
```

### Build

Builds or rebuilds the entire compiler:

```bash
src/main/bash/build.sh
```

### Run

Compiles and runs a Nexus program end-to-end:

```bash
src/main/bash/run.sh <program> [output-dir]
```

where `<program>` is the path to a Nexus source file. This generates the Python
translation of the program, executes it against the bundled runtime, and writes
the resulting artifacts (`.dot` / `.json`) into `<output-dir>` (default `out/`).
The generated Python source is left at `<output-dir>/program.py` for inspection.

A program that violates a domain rule is rejected at compile time, printing the
semantic error instead of producing artifacts.

### Generate & visualize

Build a weighted DAG, run a shortest-path analysis, and render the graph:

```bash
src/main/bash/build.sh                                    # build the compiler (once)
src/main/bash/run.sh src/test/c/accept/32-example-build out/

cat out/Main_critical_path.json                           # the shortest-path result
dot -Tpng out/Main_graph.dot -o out/graph.png             # render the graph (needs Graphviz)
```

To compile your own program, write a `.nex` file using the language constructs
described in `doc/Stage-III-Report.pdf` and pass it to `run.sh` the same way.
Ready-made example programs live under `src/test/c/accept/`.

### Test

Executes every available unit-test under `src/test/c` folder:

```bash
src/main/bash/test.sh
```

### Stop

Logout, destroy the ephemeral containers and shutdowns the cluster:

```bash
exit
docker compose down
```

### Docker

| Command                                 | Description                                             |
| :-------------------------------------- | :------------------------------------------------------ |
| `docker builder prune --all`            | Removes all builds and complete build cache.            |
| `docker compose --progress=plain build` | Forces a build or rebuild of the images in the cluster. |
| `docker image prune`                    | Removes all of the dangling images from Docker.         |
| `docker network prune`                  | Removes unused networks from Docker.                    |
| `docker volume prune`                   | Removes unused volumes from Docker.                     |

## CI/CD

To trigger an automatic integration on every push or PR (_Pull Request_), you must activate _GitHub Actions_ in the _Settings_ tab. Use the following configuration:

| Key                                                        | Value                                               |
| :--------------------------------------------------------- | :-------------------------------------------------- |
| `Actions permissions`                                      | `Allow all actions and reusable workflows`          |
| `Allow GitHub Actions to create and approve pull requests` | `false`                                             |
| `Artifact and log retention`                               | `30 days`                                           |
| `Fork pull request workflows from outside collaborators`   | `Require approval for all outside collaborators`    |
| `Workflow permissions`                                     | `Read repository contents and packages permissions` |

## Recommended Extensions

* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* [Yash](https://marketplace.visualstudio.com/items?itemName=daohong-emilio.yash)
