#!/bin/bash

# 檢查是否提供檔案名稱
if [ $# -lt 1 ]; then
    echo "❌ 錯誤：請提供 C++ 檔案名稱！"
    echo "📌 用法: ./run_solver.sh <source_file.cpp>"
    exit 1
fi

# 取得輸入的 C++ 檔案名稱
SOURCE_FILE="$1"

# 確保檔案存在
if [ ! -f "$SOURCE_FILE" ]; then
    echo "❌ 錯誤：檔案 '$SOURCE_FILE' 不存在！"
    exit 1
fi

# 設定 Gurobi 環境變數（請根據你的安裝路徑修改）
export GUROBI_HOME="/trainingData/lab401b/gurobi951/linux64"
export GRB_LICENSE_FILE="/home/waa/gurobi.lic"
export PATH="$GUROBI_HOME/bin:$PATH"
export LD_LIBRARY_PATH="$GUROBI_HOME/lib:$LD_LIBRARY_PATH"
export CPATH="$GUROBI_HOME/include:$CPATH"
export LIBRARY_PATH="$GUROBI_HOME/lib:$LIBRARY_PATH"

# 設定 C++ 編譯器
CXX=g++
CXXFLAGS="-std=c++17"
INCLUDE_PATH="$GUROBI_HOME/include"
LIB_PATH="$GUROBI_HOME/lib"
LIBS="-lgurobi_c++ -lgurobi95 -lm"

# 設定輸出執行檔名稱（去掉 .cpp 副檔名）
OUTPUT_FILE="${SOURCE_FILE%.cpp}"  # 例如：輸入 my_solver.cpp，則輸出 my_solver

# 開始編譯
echo "🔧 正在編譯 Solver..."
$CXX $CXXFLAGS -I $INCLUDE_PATH $SOURCE_FILE -o $OUTPUT_FILE -L $LIB_PATH $LIBS

# 檢查編譯是否成功
if [ $? -eq 0 ]; then
    echo "✅ 編譯成功，執行 Solver..."
    ./$OUTPUT_FILE  # 執行 Solver
else
    echo "❌ 編譯失敗，請檢查錯誤訊息！"
    exit 1
fi
