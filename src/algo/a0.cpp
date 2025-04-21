#include<iostream>
#include"../formula.h"
using namespace std;

/* From dataset
    1. I: # of users
    2. K: # of RISs
    3. R_bs_max: max rate of BS
    4. R_user_max[|I|]: max rate of users
    5. w[i]: weight of user i
    6. fid_en[|I|][|K|]: fidelity of user i create entanglement RIS k
    7. fid_pur[|I|][|K|]: fidelity of user i purify entanglement with RIS k
    8. prob_en[|I|][|K|]: probability of user i create entanglement with RIS k
    9. prob_pur[|I|][|K|]: probability of user i purify entanglement with RIS k
    10. n[|I|][|K|]: # of entanglement pairs for user i purify with RIS k
*/

/* From solver generated file
    1. x[|I|][|K|]: x[i][k] \in {0, 1} if user is assigned to RIS k, continous float
    2. R_user[|I|][|k|]: R_user[i][k] \in [0, R_user_max[i]]: rate of user i with RIS k
*/

/* --- Global Variables --- */
int I, K;
double R_bs_max;
vector<double> R_user_max, w;
vector<vector<double>> prob_en, prob_pur, n; // s_ik
vector<vector<double>> x, R_user; 

/* --- input from solver generated file --- */
void input_solver_gen_data(string solver_gen_file = "raw/solver_gen.txt"){
    ifstream in(solver_gen_file);
    if(!in.is_open()){
        cout << "Error: Cannot open file " << solver_gen_file << endl;
        exit(1);
    }
    for(int i = 0; i < I; i++){ for(int k = 0; k < K; k++){ cin >> x[i][k]; }}
    for(int i = 0; i < I; i++){ for(int k = 0; k < K; k++){ cin >> R_user[i][k]; }}
}

/* --- input from dataset --- */
void input_dataset(string dataset_file = "raw/dataset.txt"){
    ifstream in(dataset_file);
    if(!in.is_open()){
        cout << "Error: Cannot open file " << dataset_file << endl;
        exit(1);
    }
    in >> I >> K;
    in >> R_bs_max;
    R_user_max.resize(I);
    w.resize(I);
    prob_en.resize(I, vector<double>(K));
    prob_pur.resize(I, vector<double>(K));
    n.resize(I, vector<double>(K));
    x.resize(I, vector<double>(K));
    R_user.resize(I, vector<double>(K));
    for(int i = 0; i < I; i++){ in >> R_user_max[i]; }
    for(int i = 0; i < I; i++){ in >> w[i]; }
    for(int i = 0; i < I; i++){ for(int j = 0; j < K; j++){ in >> prob_en[i][j]; } }
    for(int i = 0; i < I; i++){ for(int j = 0; j < K; j++){ in >> prob_pur[i][j]; } }
    for(int i = 0; i < I; i++){ for(int j = 0; j < K; j++){ in >> n[i][j]; } }
}

int main(){
    input_dataset();
    input_solver_gen_data();
}