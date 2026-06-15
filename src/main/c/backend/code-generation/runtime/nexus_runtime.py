"""Nexus Runtime Library — stdlib-only support for generated Nexus programs."""

import heapq
import json
from collections import deque


class Graph:
    """Core graph data structure supporting directed/undirected, weighted, and capacitated graphs."""

    def __init__(self, directed=False, weighted=False, capacitated=False):
        self.directed = directed
        self.weighted = weighted
        self.capacitated = capacitated
        self.nodes = {}
        self.adj = {}
        self.groups = {}

    def add_node(self, node_id, attr=None):
        self.nodes[node_id] = attr
        if node_id not in self.adj:
            self.adj[node_id] = []

    def add_edge(self, u, v, weight=None, capacity=None):
        entry = {"to": v, "weight": weight, "capacity": capacity}
        self.adj.setdefault(u, []).append(entry)
        if not self.directed:
            reverse = {"to": u, "weight": weight, "capacity": capacity}
            self.adj.setdefault(v, []).append(reverse)

    def add_group(self, name, members):
        self.groups[name] = members

    def neighbors(self, u):
        return [e["to"] for e in self.adj.get(u, [])]


# --- Derivations (pure functions returning new Graph) ---

def transpose(g):
    t = Graph(directed=g.directed, weighted=g.weighted, capacitated=g.capacitated)
    for nid, attr in g.nodes.items():
        t.add_node(nid, attr)
    for u, edges in g.adj.items():
        for e in edges:
            t.adj.setdefault(e["to"], []).append(
                {"to": u, "weight": e["weight"], "capacity": e["capacity"]}
            )
    for name, members in g.groups.items():
        t.add_group(name, list(members))
    return t


def induced_subgraph(g, group_name):
    members = set(g.groups.get(group_name, []))
    sub = Graph(directed=g.directed, weighted=g.weighted, capacitated=g.capacitated)
    for nid in members:
        if nid in g.nodes:
            sub.add_node(nid, g.nodes[nid])
    for u in members:
        for e in g.adj.get(u, []):
            if e["to"] in members:
                sub.adj.setdefault(u, []).append(
                    {"to": e["to"], "weight": e["weight"], "capacity": e["capacity"]}
                )
    for name, group_members in g.groups.items():
        kept = [node for node in group_members if node in members]
        if kept:
            sub.add_group(name, kept)
    return sub


def remove_self_loops(g):
    r = Graph(directed=g.directed, weighted=g.weighted, capacitated=g.capacitated)
    for nid, attr in g.nodes.items():
        r.add_node(nid, attr)
    for u, edges in g.adj.items():
        for e in edges:
            if e["to"] != u:
                r.adj.setdefault(u, []).append(
                    {"to": e["to"], "weight": e["weight"], "capacity": e["capacity"]}
                )
    for name, members in g.groups.items():
        r.add_group(name, list(members))
    return r


def underlying(g):
    u_graph = Graph(directed=False, weighted=g.weighted, capacitated=g.capacitated)
    for nid, attr in g.nodes.items():
        u_graph.add_node(nid, attr)
    seen = set()
    for u, edges in g.adj.items():
        for e in edges:
            key = frozenset([u, e["to"]])
            if key not in seen:
                seen.add(key)
                u_graph.add_edge(u, e["to"], weight=e["weight"], capacity=e["capacity"])
    for name, members in g.groups.items():
        u_graph.add_group(name, list(members))
    return u_graph


# --- Algorithms (return dicts) ---

def shortest_path(g, source, target):
    dist = {n: float("inf") for n in g.nodes}
    dist[source] = 0
    prev = {n: None for n in g.nodes}
    heap = [(0, source)]
    while heap:
        d, u = heapq.heappop(heap)
        if d > dist[u]:
            continue
        if u == target:
            break
        for e in g.adj.get(u, []):
            w = e["weight"] if e["weight"] is not None else 1
            nd = d + w
            if nd < dist[e["to"]]:
                dist[e["to"]] = nd
                prev[e["to"]] = u
                heapq.heappush(heap, (nd, e["to"]))
    path = []
    cur = target
    while cur is not None:
        path.append(cur)
        cur = prev[cur]
    path.reverse()
    return {
        "source": source,
        "target": target,
        "distance": dist[target],
        "path": path if dist[target] < float("inf") else [],
    }


def topological_sort(g):
    in_deg = {n: 0 for n in g.nodes}
    for u in g.adj:
        for e in g.adj[u]:
            if e["to"] in in_deg:
                in_deg[e["to"]] += 1
    q = deque(n for n in g.nodes if in_deg[n] == 0)
    order = []
    while q:
        u = q.popleft()
        order.append(u)
        for e in g.adj.get(u, []):
            if e["to"] in in_deg:
                in_deg[e["to"]] -= 1
                if in_deg[e["to"]] == 0:
                    q.append(e["to"])
    return {"order": order, "acyclic": len(order) == len(g.nodes)}


def components(g):
    visited = set()
    comps = []
    for n in g.nodes:
        if n not in visited:
            comp = []
            q = deque([n])
            visited.add(n)
            while q:
                u = q.popleft()
                comp.append(u)
                for e in g.adj.get(u, []):
                    if e["to"] not in visited and e["to"] in g.nodes:
                        visited.add(e["to"])
                        q.append(e["to"])
            comps.append(comp)
    return {"components": comps, "count": len(comps)}


def scc(g):
    index_counter = [0]
    stack = []
    on_stack = set()
    index = {}
    lowlink = {}
    result = []

    def strongconnect(v):
        index[v] = index_counter[0]
        lowlink[v] = index_counter[0]
        index_counter[0] += 1
        stack.append(v)
        on_stack.add(v)
        for e in g.adj.get(v, []):
            w = e["to"]
            if w not in index:
                strongconnect(w)
                lowlink[v] = min(lowlink[v], lowlink[w])
            elif w in on_stack:
                lowlink[v] = min(lowlink[v], index[w])
        if lowlink[v] == index[v]:
            comp = []
            while True:
                w = stack.pop()
                on_stack.discard(w)
                comp.append(w)
                if w == v:
                    break
            result.append(comp)

    for n in g.nodes:
        if n not in index:
            strongconnect(n)
    return {"components": result, "count": len(result)}


def mst(g):
    edges = []
    seen = set()
    for u in g.adj:
        for e in g.adj[u]:
            key = frozenset([u, e["to"]])
            if key not in seen:
                seen.add(key)
                w = e["weight"] if e["weight"] is not None else 0
                edges.append((w, u, e["to"]))
    edges.sort()
    parent = {n: n for n in g.nodes}
    rank = {n: 0 for n in g.nodes}

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    def union(a, b):
        ra, rb = find(a), find(b)
        if ra == rb:
            return False
        if rank[ra] < rank[rb]:
            ra, rb = rb, ra
        parent[rb] = ra
        if rank[ra] == rank[rb]:
            rank[ra] += 1
        return True

    mst_edges = []
    total = 0
    for w, u, v in edges:
        if union(u, v):
            mst_edges.append({"from": u, "to": v, "weight": w})
            total += w
    return {"edges": mst_edges, "total_weight": total}


def max_flow(g, source, target):
    cap = {}
    adj_flow = {}
    for u in g.adj:
        for e in g.adj[u]:
            c = e["capacity"] if e["capacity"] is not None else 0
            cap[(u, e["to"])] = cap.get((u, e["to"]), 0) + c
            cap.setdefault((e["to"], u), 0)
            adj_flow.setdefault(u, set()).add(e["to"])
            adj_flow.setdefault(e["to"], set()).add(u)

    flow = {k: 0 for k in cap}
    total_flow = 0

    while True:
        parent = {source: None}
        q = deque([source])
        while q and target not in parent:
            u = q.popleft()
            for v in adj_flow.get(u, set()):
                if v not in parent and cap[(u, v)] - flow[(u, v)] > 0:
                    parent[v] = u
                    q.append(v)
        if target not in parent:
            break
        path_flow = float("inf")
        v = target
        while v != source:
            u = parent[v]
            path_flow = min(path_flow, cap[(u, v)] - flow[(u, v)])
            v = u
        v = target
        while v != source:
            u = parent[v]
            flow[(u, v)] += path_flow
            flow[(v, u)] -= path_flow
            v = u
        total_flow += path_flow

    return {"source": source, "target": target, "max_flow": total_flow}


# --- Constraint assertions ---

def assert_connected(g):
    ug = underlying(g) if g.directed else g
    res = components(ug)
    assert res["count"] == 1, f"Graph is not connected ({res['count']} components)"


def assert_strongly_connected(g):
    res = scc(g)
    assert res["count"] == 1, f"Graph is not strongly connected ({res['count']} SCCs)"


def assert_acyclic(g):
    if g.directed:
        res = topological_sort(g)
        assert res["acyclic"], "Directed graph contains a cycle"
    else:
        edge_count = sum(len(g.adj[u]) for u in g.adj) // 2
        assert edge_count < len(g.nodes), "Undirected graph contains a cycle"


def assert_reachable(g, source, target):
    visited = set()
    q = deque([source])
    visited.add(source)
    while q:
        u = q.popleft()
        if u == target:
            return
        for e in g.adj.get(u, []):
            if e["to"] not in visited:
                visited.add(e["to"])
                q.append(e["to"])
    assert False, f"Node '{target}' is not reachable from '{source}'"


def assert_tree(g, root):
    assert_connected(g)
    edge_count = sum(len(g.adj[u]) for u in g.adj)
    if g.directed:
        edge_count_undirected = edge_count
    else:
        edge_count_undirected = edge_count // 2
    assert edge_count_undirected == len(g.nodes) - 1, \
        f"Not a tree: {edge_count_undirected} edges, {len(g.nodes)} nodes"


def assert_binary_tree(g, root):
    assert_tree(g, root)
    for u in g.nodes:
        children = [e["to"] for e in g.adj.get(u, [])]
        assert len(children) <= 2, \
            f"Node '{u}' has {len(children)} children (max 2 for binary tree)"


def assert_forall(g, group_name, degree_fn, cmp, rhs):
    members = g.groups.get(group_name, [])
    cmp_ops = {
        "=": lambda a, b: a == b,
        "!=": lambda a, b: a != b,
        ">=": lambda a, b: a >= b,
        "<=": lambda a, b: a <= b,
        ">": lambda a, b: a > b,
        "<": lambda a, b: a < b,
    }
    op = cmp_ops[cmp]
    for node in members:
        edges = g.adj.get(node, [])
        if degree_fn == "degree":
            val = len(edges)
        elif degree_fn == "indegree":
            val = sum(1 for u in g.adj for e in g.adj[u] if e["to"] == node)
        elif degree_fn == "outdegree":
            val = len(edges)
        else:
            raise ValueError(f"Unknown degree function: {degree_fn}")
        assert op(val, rhs), \
            f"Constraint failed: {degree_fn}({node}) = {val}, expected {cmp} {rhs}"


# --- Exporters ---

def write_dot(g, filename):
    kw = "digraph" if g.directed else "graph"
    arrow = " -> " if g.directed else " -- "
    with open(filename, "w") as f:
        f.write(f"{kw} G {{\n")
        for nid, attr in g.nodes.items():
            if attr:
                f.write(f'  {nid} [label="{nid} ({attr})"];\n')
            else:
                f.write(f"  {nid};\n")
        seen = set()
        for u in g.adj:
            for e in g.adj[u]:
                key = (u, e["to"]) if g.directed else frozenset([u, e["to"]])
                if key in seen:
                    continue
                seen.add(key)
                labels = []
                if e["weight"] is not None:
                    labels.append(f'w={e["weight"]}')
                if e["capacity"] is not None:
                    labels.append(f'c={e["capacity"]}')
                label = f' [label="{", ".join(labels)}"]' if labels else ""
                f.write(f"  {u}{arrow}{e['to']}{label};\n")
        f.write("}\n")


def write_result_dot(value, filename):
    with open(filename, "w") as f:
        f.write("digraph Result {\n")
        if isinstance(value, dict) and "path" in value:
            path = value.get("path", [])
            for node in path:
                f.write(f'  "{node}";\n')
            for i in range(len(path) - 1):
                f.write(f'  "{path[i]}" -> "{path[i + 1]}";\n')
        elif isinstance(value, dict) and "edges" in value:
            for edge in value.get("edges", []):
                u = edge.get("from")
                v = edge.get("to")
                label = edge.get("weight")
                suffix = f' [label="{label}"]' if label is not None else ""
                f.write(f'  "{u}" -> "{v}"{suffix};\n')
        else:
            label = json.dumps(value, default=str).replace('"', '\\"')
            f.write(f'  result [shape=box, label="{label}"];\n')
        f.write("}\n")


def write_json(value, filename):
    with open(filename, "w") as f:
        json.dump(value, f, indent=2, default=str)
        f.write("\n")
