#include<bits/stdc++.h>
#include<cstdlib>
#include<cerrno>
#include<cstring>
#include<chrono>
using namespace std;

class algorithm{
public:
    string name;
    string exe;
    string filename;
    string infile;
    string outfile;
    // string algo_opname;  // This member variable is used in original code but not defined in class declaration
    
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
        string compile_cmd = "g++ " + filename + " -o " + exe + " -std=c++17";
        string cmd = "./" + exe + " " + infile + " " + outfile;
        
        cout << "Executing compile command: " << compile_cmd << endl;
        int compile_result = system(compile_cmd.c_str());
        if (compile_result != 0) {
            cerr << "Compilation failed: " << filename << " Error code: " << compile_result << endl;
            return false;
        }
        
        cout << "Executing algorithm: " << cmd << endl;
        int run_result = system(cmd.c_str());
        if (run_result != 0) {
            cerr << "Execution failed: " << exe << " Error code: " << run_result << endl;
            return false;
        }
        
        // cout << "Successfully executed algorithm: " << name << endl;
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

    bool generate(int I, int K, double R_bs_max, double fidelity_threshold, double avg_load, int seed){
        //Usage: ./gen <I> <K> <R_bs_max> <fidelity_threshold> <avg_load>
        string cmd = "g++ " + genfile + " -o " + genexe;
        cout << "Compiling data generator: " << cmd << endl;
        int compile_result = system(cmd.c_str());
        if (compile_result != 0) {
            cerr << "Failed to compile data generator: " << genfile << " Error code: " << compile_result << endl;
            return false;
        }
        
        string cmd_run = "./" + genexe + " " + dataset_file + " " + to_string(I) + " " + to_string(K) + " " + to_string(R_bs_max) + " " + to_string(fidelity_threshold) + " " + to_string(avg_load) + " " + to_string(seed);
        cout << "Generating data: " << cmd_run << endl;
        int run_result = system(cmd_run.c_str());
        if (run_result != 0) {
            cerr << "Data generation failed: " << genexe << " Error code: " << run_result << endl;
            return false;
        }
        
        // cout << "Successfully generated data: " << dataset_file << endl;
        return true;
    }
};

int main(){
    // remove old res
    string rm_cmd = "rm -rf data/res";
    string mkdir_cmd = "mkdir -p data/res";
    string mkdir_avg_cmd = "mkdir -p data/res/avg";
    system(rm_cmd.c_str());
    system(mkdir_cmd.c_str());
    system(mkdir_avg_cmd.c_str());
    cout << "Removed old res and created new res directory" << endl;

    // start time
    auto start = chrono::high_resolution_clock::now();
    string dataset_file = "dataset.txt";
    string algo_path = "src/algo/";
    string exe_path = algo_path + "bin/";
    string res_path = "data/res/";

    // Define algorithms
    string name_algo[5] = {"a0", "greedy_w", "greedy_cp", "greedy_obj", "SA_not_random"};
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

    int I = 100, K = 10, R_bs_max = 2e5;
    double fidelity_threshold = 0.85, avg_load = 1;
    int seed = 0;

    // Run algorithms
    for(int i=1; i<=5; i++){
        // Modify parameter for dataset
        I = i * 50 + 50;
        for(int j=1; j<=5; j++){
            seed = 0 + j;
            // seed = j-1;
            string cur_dataset = gen_data_path + "dataset_" + to_string(i) + "_" + to_string(j) + ".txt";
            gen.dataset_file = cur_dataset;
            
            if (!gen.generate(I, K, R_bs_max, fidelity_threshold, avg_load, seed)) {
                cerr << "Skipping algorithm tests for dataset: " << cur_dataset << endl;
                continue;
            }
            
            // Run algorithms
            for(int k=0; k<6; k++){
                if(k == 0){
                    string tmp = "./run/run_solver.sh src/algo/exp_solver.cpp " + cur_dataset + " data/res/res_solver.txt";
                    system(tmp.c_str());
                }
                if(k == 5){
                    cout << "run ILP" << endl;
                    string tmp = "./run/run_solver.sh src/algo/ILP.cpp " + cur_dataset + " " + res_path + "ILP_" + to_string(i) + "_" + to_string(j) + ".txt";
                    system(tmp.c_str());
                    continue;
                }
                algo[k].infile = cur_dataset;
                algo[k].outfile = res_path + name_algo[k] + "_" + to_string(i) + "_" + to_string(j) + ".txt";  

                if (!algo[k].run()) {
                    cerr << "Algorithm " << algo[k].name << " execution failed, continuing to next algorithm" << endl;
                }
            }
        }
    }
    
    string run_avg_path = "run/avg.cpp";
    string run_avg_exe = "run/bin/avg.exe";
    string run_avg_cmd = "g++ " + run_avg_path + " -o " + run_avg_exe;
    string run_avg_cmd_run = "./" + run_avg_exe;
    cout << "Compiling average calculation: " << run_avg_cmd << endl;
    int compile_result = system(run_avg_cmd.c_str());

    // end time
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end - start);
    cout << "Total time taken: " << duration.count() << " seconds" << endl;
    return 0;
}