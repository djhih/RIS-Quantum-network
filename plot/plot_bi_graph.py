import re
import itertools
import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch

# 讀取資料檔案
filename = "results_tmp.txt"
edges = []

with open(filename, "r", encoding="utf-8") as f:
    for line in f:
        if line.strip().startswith("x : i"):
            m = re.search(r"x\s*:\s*i\s*(\d+)\s*k\s*(\d+)\s*decision\s*([\d\.]+)", line)
            if m:
                u, v, xval = f"i{m.group(1)}", f"k{m.group(2)}", float(m.group(3))
                if xval > 0:
                    edges.append((u, v, xval))

# 建圖
B = nx.Graph()
for u, v, xval in edges:
    B.add_node(u, bipartite=0)
    B.add_node(v, bipartite=1)
    B.add_edge(u, v, xval=xval)

# 定位
left = {n for n,d in B.nodes(data=True) if d['bipartite']==0}
right = set(B) - left
pos = {n:(0,i) for i,n in enumerate(sorted(left))}
pos.update({n:(1,i) for i,n in enumerate(sorted(right))})

# 繪圖
plt.figure(figsize=(8,6))
nx.draw_networkx_nodes(B, pos, node_size=700, node_color="lightblue")
nx.draw_networkx_labels(B, pos)

# 交替使用不同的弧度並擴大間隔
edge_count = len(B.edges())
max_rad = 0.35 if edge_count > 10 else 0.25  # 增加最大弧度

# 生成變化更大的弧度序列
rads = []
for i in range(edge_count):
    # 交替正負，並增加變化
    sign = 1 if i % 2 == 0 else -1
    magnitude = 0.1 + (i // 2) * 0.05  # 逐漸增加弧度大小
    if magnitude > max_rad:
        magnitude = 0.1 + ((i // 2) % 5) * 0.05  # 循環使用較小的弧度
    rads.append(sign * magnitude)

# 使用 PathCollection 和 FancyArrowPatch 直接控制繪圖
fig = plt.gcf()
ax = plt.gca()

for (u, v, data), rad in zip(B.edges(data=True), rads):
    # 獲取節點位置
    x1, y1 = pos[u]
    x2, y2 = pos[v]
    
    # 繪製彎曲的邊
    arrow = FancyArrowPatch(
        (x1, y1), (x2, y2),
        connectionstyle=f'arc3,rad={rad}',
        arrowstyle='-',
        mutation_scale=15,
        lw=1.5,
        alpha=0.7,
        color='black'
    )
    ax.add_patch(arrow)
    
    # 計算弧線上的點，使用參數方程式
    # 對於弧線，我們使用二次貝茲曲線來近似
    
    # 計算控制點
    mx, my = (x1 + x2) / 2, (y1 + y2) / 2
    # 控制點垂直於線段
    dx = x2 - x1
    dy = y2 - y1
    cx = mx - rad * 2 * dy  # 控制點x座標
    cy = my + rad * 2 * dx  # 控制點y座標
    
    # 計算曲線上的點 (參數t從0到1)
    t = 0.5  # 在弧線中點放置標籤
    
    # 二次貝茲曲線公式
    label_x = (1-t)**2 * x1 + 2*(1-t)*t * cx + t**2 * x2
    label_y = (1-t)**2 * y1 + 2*(1-t)*t * cy + t**2 * y2
    
    # 計算標籤的額外偏移，使其遠離弧線
    # 控制點與中點的方向向量
    dcx = cx - mx
    dcy = cy - my
    # 標準化
    norm = np.sqrt(dcx**2 + dcy**2) or 1
    dcx, dcy = dcx/norm * 0.03, dcy/norm * 0.03
    
    # 增加背景框，提高標籤可讀性
    bbox_props = dict(
        boxstyle="round,pad=0.3", 
        fc="white", 
        ec="gray", 
        alpha=0.9
    )
    
    # 在計算好的位置放置標籤
    plt.text(
        label_x + dcx,  # 加上偏移
        label_y + dcy,  # 加上偏移
        f"{data['xval']:.2f}",
        ha="center", 
        va="center",
        bbox=bbox_props,
        fontsize=9,
        zorder=10  # 確保標籤在最上層
    )

plt.title("Decision‑Based Bipartite Graph (curved edges)")
plt.axis('off')
plt.tight_layout()
plt.savefig("image/graph_with_curved_edges.png")
plt.clf()
