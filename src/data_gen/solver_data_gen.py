import random

def generate_random_test_data(min_dim=2, max_dim=5):
    I = random.randint(min_dim, max_dim)
    K = random.randint(I, max_dim)

    w = [round(random.uniform(0.5, 10.0), 2) for _ in range(I)]

    r_w = [[1.0 / random.randint(1, 15) for _ in range(K)] for _ in range(I)]

    R_max = [random.randint(10, 100) for _ in range(I)]
    R_bs_max = random.randint(50, 1000)

    
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
    generate_random_test_data(min_dim=2, max_dim=10)
