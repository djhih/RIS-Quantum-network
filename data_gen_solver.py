import random

def generate_random_test_data(min_dim=2, max_dim=5):
    I = random.randint(min_dim, max_dim)
    K = random.randint(min_dim, max_dim)

    w = [round(random.uniform(0.5, 2.0), 2) for _ in range(I)]

    r_w = []
    for i in range(I):
        row = [round(random.uniform(1, 10), 2) for _ in range(K)]
        r_w.append(row)

    R_max = [random.randint(10, 50) for _ in range(I)]
    R_bs_max = random.randint(50, 200)

    
    with open('solver_data.txt', 'w', encoding='utf-8') as f:
        f.write(f"{K}\n")
        f.write(f"{I}\n")
        for i in range(I):
            f.write(str(w[i]) + ' ')
        f.write("\n") 
        for i in range(I):
            for k in range(K):
                f.write(str(r_w[i][k]) + ' ')
        f.write("\n") 
        for i in range(I):
            f.write(str(R_max[i]) + ' ')
        f.write(str(R_bs_max) + "\n")

if __name__ == "__main__":
    generate_random_test_data(min_dim=3, max_dim=5)
