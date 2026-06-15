#! /bin/bash

set -u

BASE_PATH="$(dirname "$0")/../../.."
cd "$BASE_PATH"

GREEN='\033[0;32m'
RED='\033[0;31m'
OFF='\033[0m'
STATUS=0

RUNTIME="src/main/c/backend/code-generation/runtime/nexus_runtime.py"
WORK="$(mktemp -d)"
cp "$RUNTIME" "$WORK/"

echo "Code generation should produce runnable Python..."
echo ""

for test in \
    17-constraint-forall \
    18-derive-transpose 19-derive-induced 20-derive-remove-self-loops 21-derive-underlying \
    22-analysis-shortest-path 23-analysis-topological-sort 24-analysis-components \
    25-analysis-scc 26-analysis-mst 27-analysis-max-flow \
    28-export-graph 29-export-result 30-stage-i-integration; do
    cat "src/test/c/accept/$test" | LOGGING_LEVEL=CRITICAL ".build/Nexus" 2>/dev/null > "$WORK/program.py"
    if [ ! -s "$WORK/program.py" ]; then
        STATUS=1
        echo -e "    $test, ${RED}emitted no Python${OFF}"
        continue
    fi
    ( cd "$WORK" && python3 program.py ) >/dev/null 2>&1
    rc=$?
    ok=1
    [ "$rc" != "0" ] && ok=0
    if [ "$test" == "30-stage-i-integration" ]; then
        { [ -f "$WORK/Main_graph.dot" ] && [ -f "$WORK/Main_path.json" ]; } || ok=0
    fi
    rm -f "$WORK"/Main_*.dot "$WORK"/Main_*.json
    if [ "$ok" == "1" ]; then
        echo -e "    $test, ${GREEN}runs cleanly${OFF}"
    else
        STATUS=1
        echo -e "    $test, ${RED}runtime error or missing artifacts${OFF}"
    fi
done
echo ""

rm -rf "$WORK"
echo "All done."
exit $STATUS
