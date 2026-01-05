import sys
import json
import networkx as nx
import numpy as np
import plotly.graph_objects as go

# python viz.py graph.json
# cat graph.json | python viz.py

def load_json_input():
    if len(sys.argv) >= 2:
        with open(sys.argv[1], "r") as f:
            return json.load(f)
    else:
        return json.loads(sys.stdin.read())

data = load_json_input()

nodes = data["nodes"]
edges = data["edges"]

G = nx.DiGraph()
for n in nodes:
    node_id = int(n["id"])
    G.add_node(node_id)
    if "color" in n:
        G.nodes[node_id]["group"] = int(n["color"])
    if "state" in n:
        G.nodes[node_id]["state"] = str(n["state"])
    if "value" in n:
        G.nodes[node_id]["value"] = int(n["value"])

for e in edges:
    u = int(e["u"])
    v = int(e["v"])
    G.add_edge(u, v)

for n in G.nodes():
    if "value" not in G.nodes[n]:
        G.nodes[n]["value"] = G.out_degree(n)

for n in G.nodes():
    if "group" not in G.nodes[n]:
        G.nodes[n]["group"] = 0

print("Loaded nodes:", G.number_of_nodes())
print("Loaded edges:", G.number_of_edges())

btw = nx.betweenness_centrality(G.to_undirected())
nx.set_node_attributes(G, btw, "betweenness")

pos = nx.spring_layout(G.to_undirected(), dim=3, seed=7, k=None, iterations=200)


# Format for Plotly
edge_x, edge_y, edge_z = [], [], []
for u, v in G.edges():
    x0, y0, z0 = pos[u]
    x1, y1, z1 = pos[v]
    edge_x += [x0, x1, None]
    edge_y += [y0, y1, None]
    edge_z += [z0, z1, None]

node_x = [pos[n][0] for n in G.nodes()]
node_y = [pos[n][1] for n in G.nodes()]
node_z = [pos[n][2] for n in G.nodes()]


# Sytle
deg = dict(G.degree())

val = np.array([G.nodes[n]["value"] for n in G.nodes()], dtype=float)
if np.ptp(val) == 0:
    val_norm = np.ones_like(val)
else:
    val_norm = (val - val.min()) / np.ptp(val)
node_size = (6 + 18 * val_norm).tolist()

group = [G.nodes[n]["group"] for n in G.nodes()]

palette = {
    -1: "#9CA3AF",  # Wins person on turn
     0: "#EF4444",  # P1 forced
     1: "#10B981",  # P2 forced
}
node_color = [palette.get(g, "#60A5FA") for g in group]

def get_btw(n):
    return G.nodes[n].get("betweenness", 0.0)

node_text = []
for n in G.nodes():
    state = G.nodes[n].get("state", "")
    st_line = f"<br>state: {state}" if state else ""
    node_text.append(
        f"id: {n}"
        f"{st_line}"
        f"<br>group: {G.nodes[n]['group']}"
        f"<br>value: {G.nodes[n]['value']}"
        f"<br>deg: {deg[n]}"
        f"<br>btw: {get_btw(n):.3f}"
    )

# Plotly 3d
fig = go.Figure()

fig.add_trace(go.Scatter3d(
    x=edge_x, y=edge_y, z=edge_z,
    mode="lines",
    line=dict(width=1),
    hoverinfo="skip",
    name="edges"
))

fig.add_trace(go.Scatter3d(
    x=node_x, y=node_y, z=node_z,
    mode="markers",
    marker=dict(size=node_size, color=node_color, opacity=0.95),
    text=node_text, hoverinfo="text",
    name="nodes"
))

fig.update_layout(
    template="plotly_dark",
    showlegend=False,
    scene=dict(
        xaxis=dict(visible=False),
        yaxis=dict(visible=False),
        zaxis=dict(visible=False)
    ),
    margin=dict(l=0, r=0, t=0, b=0)
)

fig.show()
