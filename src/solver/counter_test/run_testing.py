import subprocess

def modify_file(file_path: str, new_x_value: int):
    with open(file_path, 'r') as f:
        lines = f.readlines()

    if not lines:
        raise ValueError("檔案是空的")

    lines[-1] = f"{new_x_value}\n"

    with open(file_path, 'w') as f:
        f.writelines(lines)

def run_command() -> str:
    try:
        result = subprocess.run(
            ["./run_solver.sh", "counter.cpp"],
            capture_output=True,
            text=True,
            check=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        return f"[Error] {e.stderr.strip()}"

def main():
    file_path = "solver_data.txt"       # 輸入檔案名稱
    output_path = "results.txt"   # 結果儲存檔案
    x_lower = 250.0                   # X 下界
    x_upper = 300.0                 # X 上界（包含）

    # run command: python generate_counter_data_test.py

    command = ["python", "generate_counter_data_test.py"]
    try:
        subprocess.run(command, check=True)
    except subprocess.CalledProcessError as e:
        print(f"執行 {command} 時發生錯誤: {e}")
        return

    with open(output_path, 'w') as out_file:
        x = x_lower
        while(x < x_upper):
            print(f"正在執行 BS max rate = {x}...")
            modify_file(file_path, x)
            result = run_command()
            out_file.write(f"{result}\n{'-'*40}\n")
            x += 3

    print("所有執行完畢，結果已儲存至", output_path)

if __name__ == "__main__":
    main()
