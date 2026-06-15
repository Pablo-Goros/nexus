#! /bin/bash

# Compile and run a Nexus program end-to-end.
#
#   run.sh <program.nex> [output-dir]
#
# Generates the Python translation of the program (semantic analysis + code
# generation), executes it against the bundled runtime, and writes the
# resulting artifacts (.dot / .json) into <output-dir> (default "out/").
# The generated Python source is left at <output-dir>/program.py for inspection.

set -euo pipefail

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

PROGRAM="${1:?usage: run.sh <program.nex> [output-dir]}"
OUT_DIR="${2:-out}"
COMPILER=".build/Nexus"
RUNTIME="src/main/c/backend/code-generation/runtime/nexus_runtime.py"

if [ ! -x "$COMPILER" ]; then
	echo "Compiler not built. Run 'src/main/bash/build.sh' first." >&2
	exit 1
fi

mkdir -p "$OUT_DIR"
cp "$RUNTIME" "$OUT_DIR/"

# 1) Generate the Python translation. A program that violates a domain rule is
#    rejected here (non-zero exit) and its error is reported.
if ! LOGGING_LEVEL=ERROR "$COMPILER" < "$PROGRAM" > "$OUT_DIR/program.py" 2> "$OUT_DIR/compile.log"; then
	echo "Compilation rejected:" >&2
	cat "$OUT_DIR/compile.log" >&2
	exit 1
fi

# 2) Execute the generated program: builds the graphs, checks the constraints as
#    runtime assertions, runs the algorithms, and writes the exported artifacts.
if ( cd "$OUT_DIR" && python3 program.py ); then
	echo "Artifacts written to $OUT_DIR/:"
	ls -1 "$OUT_DIR"/*.dot "$OUT_DIR"/*.json 2>/dev/null || echo "  (this program declares no exports)"
else
	echo "The generated program raised an error at runtime (e.g. a failed constraint assertion)." >&2
	exit 1
fi
