#include<bits/stdc++.h>
#include<cstdlib>
using namespace std;
// read files in res/ and calculate average Objective value
int main(){
    string algo_path = "src/algo/";
    string res_path = "data/res/";
    string name_algo[6] = {"a0", "greedy_cp", "greedy_obj", "SA_not_random", "ILP", "greedy_w"};
    // double data[20][20][30][30];
    // memset(data, 0, sizeof(data));
    int test_case = 5, sub_test_case = 200, number_algo = 6, number_graph = 5;

    vector<vector<vector<vector<double>>>> data(number_graph, vector<vector<vector<double>>>(number_algo, vector<vector<double>>(test_case+1, vector<double>(sub_test_case+1, 0))));

    // i0: algo, i1: dataset, i2: subdataset
    // graph
    string target[5] = {"Objective value:", "Generation rate:", "Connection cost:", "# Satisfied UEs:", "Total power usage:"};
    for(int g=0; g<number_graph; g++){
        for(int i0=0; i0<number_algo; i0++){
            for(int i=1; i<=test_case; i++){
                for(int j=1; j<=sub_test_case; j++){
                    // if( j < 90 && j > 50) continue;
                    ifstream in;
                    int tg_file = i;
                    if(tg_file == 1) tg_file = 6;
                    else tg_file -= 1;

                    string filename = res_path + name_algo[i0] + "_" + to_string(tg_file) + "_" + to_string(j) + ".txt";
                    in.open(filename);
                    if(!in.is_open()){
                        // cout << "Error: Cannot open file " << filename << endl;
                        data[g][i0][i][j] = -1;
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
                    // if((i0==0 || i0==2) && i==1 && g==1)
                    // cout << "open file: " << filename << " " << target[g] << obj << endl;
                    in.close();
                }
            }
        }
    }
    

    vector<vector<vector<double>>> avg_obj(number_graph, vector<vector<double>>(number_algo, vector<double>(test_case+1, 0)));
    // count avg
    for(int g=0; g<number_graph; g++){
        for(int i=1; i<=test_case; i++){
            vector<bool> valid_test(sub_test_case+1, false);
            
            // 檢查每個子測資是否有任何一個演算法有無效資料
            for(int j=1; j<=sub_test_case; j++){
                bool any_valid = true;
                for(int k=0; k<number_algo; k++){
                    if(data[g][k][i][j] <= 0){
                        any_valid = false;
                        break;
                    }
                }
                valid_test[j] = any_valid;
                if(g == 0 && !any_valid){
                    // cout << "not ok on " << g << " " << i << " " << j << endl;
                }
            }
            
            // 計算有效的子測試案例數量
            int valid_count = 0;
            for(int j=1; j<=sub_test_case; j++){
                if(valid_test[j]){
                    valid_count++;
                    // 累加有效資料到平均值
                    for(int k=0; k<number_algo; k++){
                        avg_obj[g][k][i] += data[g][k][i][j];
                    }
                }
            }
            
            // 計算平均值，避免除以零
            if(valid_count > 0){
                for(int k=0; k<number_algo; k++){
                    // if(g == 0)
                        // cout << "Algorithm: " << name_algo[k] << " i " << i << " cnt " << valid_count << endl;
                    avg_obj[g][k][i] /= valid_count;
                    // cout << "Algorithm: " << name_algo[k] << " i " << i << " cnt " << valid_count;
                    // cout << " avg_obj: " << avg_obj[g][k][i] << endl;
                }
            } else {
                if(g == 0)
                    cout << "Warning: No valid data for test case " << i << endl;
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
