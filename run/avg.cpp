#include<bits/stdc++.h>
#include<cstdlib>
using namespace std;
// read files in res/ and calculate average Objective value
int main(){
    string algo_path = "src/algo/";
    string res_path = "data/res/";
    string name_algo[6] = {"a0", "greedy_w", "greedy_cp", "greedy_obj", "SA_not_random", "ILP"};
    double avg_obj[10][20];
    memset(avg_obj, 0, sizeof(avg_obj));
    // i0: algo, i1: dataset, i2: subdataset
    for(int i0=0; i0<6; i0++){
        for(int i=1; i<=5; i++){
            int tot = 10;
            for(int j=1; j<=10; j++){
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
                    if(line.find("Objective value:") != string::npos){
                        istringstream iss(line);
                        string tmp;
                        iss >> tmp >> tmp >> obj;
                        avg_obj[i0][i] += obj;
                        if(avg_obj[i0][i] == 0){
                            tot--;
                            break;
                        }
                    }
                }
                cout << "open file: " << filename << " obj " << obj << endl;
                in.close();
            }
            avg_obj[i0][i] /= tot;
        }
    }

    for(int i=0; i<6; i++){
        cout << "Algorithm: " << name_algo[i] << endl;
        ofstream out;
        string filename = res_path + "avg/" + name_algo[i] + "_avg.txt";
        out.open(filename);
        if(!out.is_open()){
            cout << "Error: Cannot open file " << filename << endl;
            exit(1);
        }
        for(int j=1; j<6; j++){
            out << avg_obj[i][j] << endl;
        }
    }
}