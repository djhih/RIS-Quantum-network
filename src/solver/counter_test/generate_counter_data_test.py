import random 
def generate_solver_data_file(K, I):
    # 隨機數值產生
    R_bs_max = random.randint(100, 300)
    w = [round(random.uniform(0.5, 2.0), 2) for _ in range(I)]
    V = [random.randint(80, 100) for _ in range(I)]
    s = [[random.uniform(10, 50) for _ in range(K)] for _ in range(I)]

    with open("solver_data.txt", "w") as f:
#    with open("src/solver/counter_test/solver_data.txt", "w") as f:
        f.write(f"{K} {I}\n")
        f.write(f"{R_bs_max}\n")
        f.write(" ".join(map(str, w)) + "\n")
        f.write(" ".join(map(str, V)) + "\n")
        for row in s:
            f.write(" ".join(map(str, row)) + "\n")
        f.write(f"{R_bs_max}\n")

# 使用範例
K = random.randint(3, 5)  # RIS 數量
I = random.randint(5, 7)  # User 數量
generate_solver_data_file(K, I)

print(f"已產生 input_function.cpp 與 solver_data.txt (K={K}, I={I})")
