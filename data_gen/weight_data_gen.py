import random

# 固定隨機種子以便重現結果
random.seed(42)

# 設定數值範圍或固定數值（可依需求調整）
# L_it = random.randint(1, 100)       # 例如：迭代次數或其他參數
L_it = 1000
I_user = random.randint(3, 10)       # 用戶數量
K_ris = random.randint(2, 5)         # RIS 數量
M_cap = random.randint(50, 150)      # 例如：容量或其他參數

# 輸出結果檔案名稱
output_file = "raw/testdata.txt"

# 定義座標產生範圍
coord_min, coord_max = 10000, 100000

with open(output_file, "w") as f:
    # 輸出第一行參數
    f.write(f"{L_it} {I_user} {K_ris} {M_cap}\n")
    
    # 產生 I_user 筆用戶資料：x_loc, y_loc, weight
    for _ in range(I_user):
        x = random.uniform(coord_min, coord_max)
        y = random.uniform(coord_min, coord_max)
        weight = random.uniform(0.1, 10)  # 假設 weight 為浮點數，範圍 0.1 ~ 10
        # 輸出時保留兩位小數
        f.write(f"{x:.2f} {y:.2f} {weight:.2f}\n")
    
    # 產生 K_ris 筆 RIS 資料：x_loc, y_loc
    for _ in range(K_ris):
        x = random.uniform(coord_min, coord_max)
        y = random.uniform(coord_min, coord_max)
        f.write(f"{x:.2f} {y:.2f}\n")

print(f"Test data has been written to {output_file}")
