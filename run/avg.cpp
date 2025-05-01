#include<bits/stdc++.h>
#include<cstdlib>
using namespace std;
// read files in res/ and calculate average Objective value
int main(){
    string algo_path = "src/algo/";
    string res_path = "data/res/";
    string name_algo[6] = {"a0", "greedy_cp", "greedy_obj", "SA_not_random", "greedy_w", "ILP"};
    double data[20][20][30][30];
    memset(data, 0, sizeof(data));
    int test_case = 5, sub_test_case = 10, number_algo = 4, number_graph = 5;

    // i0: algo, i1: dataset, i2: subdataset
    // graph
    string target[5] = {"Objective value:", "Generation rate:", "Connection cost:", "# Satisfied UEs:", "Total power usage:"};
    for(int g=0; g<number_graph; g++){
        for(int i0=0; i0<number_algo; i0++){
            for(int i=1; i<=test_case; i++){
                int tot = 10;
                for(int j=1; j<=sub_test_case; j++){
                    ifstream in;
                    string filename = res_path + name_algo[i0] + "_" + to_string(i) + "_" + to_string(j) + ".txt";
                    in.open(filename);
                    if(!in.is_open()){
                        cout << "Error: Cannot open file " << filename << endl;
                        tot--;
                        continue;
                    }

                    string line;
                    double obj = 0;
                    
                    while(getline(in, line)){
                        if(line.find(target[g]) != string::npos){
                            istringstream iss(line);
                            string tmp;
                            if(g < 3)
                                iss >> tmp >> tmp >> obj;
                            else 
                                iss >> tmp >> tmp >> tmp >> obj;
                            data[g][i0][i][j] = obj;
                        }
                    }
                    if((i0==0 || i0==5) && i==2 && g==0)
                    cout << "open file: " << filename << " " << target[g] << obj << endl;
                    in.close();
                }
            }
        }
    }
    

    vector<vector<vector<double>>> avg_obj(6, vector<vector<double>>(6, vector<double>(11, 0)));
    // count avg
    for(int g=0; g<number_graph; g++){
        for(int i=1; i<=test_case; i++){
            vector<bool>ok(sub_test_case, false);
            for(int j=1; j<=sub_test_case; j++){
                for(int k=0; k<number_algo; k++){
                    if(data[g][k][i][j] != 0){
                        ok[j] = true;
                    }
                }
            }
            double cnt = 0;
            for(int j=1; j<=sub_test_case; j++){
                if(ok[j]){
                    cnt++;
                    for(int k=0; k<number_algo; k++){
                        avg_obj[g][k][i] += data[g][k][i][j];
                    }
                }
            }
            for(int k=0; k<number_algo; k++){
                avg_obj[g][k][i] /= cnt;
            }
        }
    }
    
    string opname[6] = {"avg_obj", "avg_gen", "avg_cost", "avg_satis", "avg_power"};
    for(int g=0; g<number_graph; g++){
        for(int i=0; i<number_algo; i++){
            // cout << "Algorithm: " << name_algo[i] << endl;
            ofstream out;
            string filename = res_path + "avg/" + name_algo[i] + "_" + opname[g] + "_avg.txt";
            out.open(filename);
            if(!out.is_open()){
                cout << "Error: Cannot open file " << filename << endl;
                exit(1);
            }
            for(int j=1; j<=test_case; j++){
                out << avg_obj[g][i][j] << endl;
            }
        }    
    }
    
}