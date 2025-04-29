#include<bits/stdc++.h>
#include<cstdlib>
#include<cerrno>
#include<cstring>
using namespace std;

class algorithm{
public:
    string name;
    string exe;
    string filename;
    string infile;
    string outfile;
    string algo_opname;  // 這個成員變數在原代碼中有使用但未在類聲明中定義
    
    algorithm(){
        this->name = "";
        this->exe = "";
        this->filename = "";
        this->infile = "";
        this->outfile = "";
   }
    algorithm(string name, string exe, string filename, string outfile){
        this->name = name;
        this->exe = exe;
        this->filename = filename;
        this->outfile = outfile;
    }

    // g++ filename -o exe ; ./exe infile outfile
    bool run(){
        string compile_cmd = "g++ " + filename + " -o " + exe;
        string cmd = "./" + exe + " " + infile + " " + outfile;
        
        cout << "執行編譯命令: " << compile_cmd << endl;
        int compile_result = system(compile_cmd.c_str());
        if (compile_result != 0) {
            cerr << "編譯失敗: " << filename << " 錯誤碼: " << compile_result << endl;
            cerr << "可能的原因: " << strerror(errno) << endl;
            return false;
        }
        
        cout << "執行算法: " << cmd << endl;
        int run_result = system(cmd.c_str());
        if (run_result != 0) {
            cerr << "執行失敗: " << exe << " 錯誤碼: " << run_result << endl;
            cerr << "可能的原因: " << strerror(errno) << endl;
            return false;
        }
        
        cout << "成功執行算法: " << name << endl;
        return true;
    }
};

class data_generator{
public:
    string genfile;
    string genexe;
    string dataset_file;
    data_generator(){
        this->genfile = "";
        this->genexe = "";
        this->dataset_file = "";
    }
    data_generator(string genfile, string genexe, string dataset_file){
        this->genfile = genfile;
        this->genexe = genexe;
        this->dataset_file = dataset_file;
    }

    bool generate(int I, int K, double R_bs_max, double fidelity_threshold, double avg_load){
        //Usage: ./gen <I> <K> <R_bs_max> <fidelity_threshold> <avg_load>
        string cmd = "g++ " + genfile + " -o " + genexe;
        cout << "編譯資料生成器: " << cmd << endl;
        int compile_result = system(cmd.c_str());
        if (compile_result != 0) {
            cerr << "編譯資料生成器失敗: " << genfile << " 錯誤碼: " << compile_result << endl;
            cerr << "可能的原因: " << strerror(errno) << endl;
            return false;
        }
        
        string cmd_run = "./" + genexe + " " + dataset_file + " " + to_string(I) + " " + to_string(K) + " " + to_string(R_bs_max) + " " + to_string(fidelity_threshold) + " " + to_string(avg_load);
        cout << "執行資料生成: " << cmd_run << endl;
        int run_result = system(cmd_run.c_str());
        if (run_result != 0) {
            cerr << "資料生成失敗: " << genexe << " 錯誤碼: " << run_result << endl;
            cerr << "可能的原因: " << strerror(errno) << endl;
            return false;
        }
        
        cout << "成功生成資料: " << dataset_file << endl;
        return true;
    }
};

int main(){
    string dataset_file = "dataset.txt";
    string algo_path = "src/algo/";
    string exe_path = algo_path + "bin/";
    string res_path = "data/res/";

    // Define algorithms
    string name_algo[5] = {"a0", "greedy_w", "greedy_cp", "greedy_obj", "SA_v0"};
    algorithm algo[5];
    for(int i = 0; i < 5; i++){
        algo[i].name = name_algo[i];
        algo[i].filename =  algo_path + name_algo[i] + ".cpp";  
        algo[i].exe = exe_path + name_algo[i] + ".exe"; 
        algo[i].outfile = "res_" + name_algo[i] + ".txt";  
    }
    
    // Define data generator
    string gen_data_path = "data/raw/";
    string genfile = "src/data_gen/gen.cpp";
    string genexe = "src/data_gen/bin/gen.exe";
    data_generator gen(genfile, genexe, dataset_file);

    int I = 100, K = 10, R_bs_max = 100;
    double fidelity_threshold = 0.85, avg_load = 1;

    // Run algorithms
    for(int i=1; i<=5; i++){
        // Modify parameter for dataset
        I = i * 100;
        for(int j=1; j<=10; j++){
            string cur_dataset = gen_data_path + "dataset_" + to_string(i) + "_" + to_string(j) + ".txt";
            gen.dataset_file = cur_dataset;
            
            if (!gen.generate(I, K, R_bs_max, fidelity_threshold, avg_load)) {
                cerr << "跳過資料集: " << cur_dataset << " 的演算法測試" << endl;
                continue;
            }
            
            // Run algorithms
            for(int k=0; k<5; k++){
                algo[k].infile = cur_dataset;
                algo[k].outfile = res_path + name_algo[k] + "_" + to_string(i) + "_" + to_string(j) + ".txt";  

                if (!algo[k].run()) {
                    cerr << "算法 " << algo[k].name << " 執行失敗，繼續下一個算法" << endl;
                }
            }
        }
    }
    
    return 0;
}