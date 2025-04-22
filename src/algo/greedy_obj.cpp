#include<iostream>
#include"../formula.h"
using namespace std;

/* --- Global Variables --- */
const int INF = 1e9;
int I, K;
double R_bs_max;
vector<double> R_user_max, w;
vector<vector<double>> prob_en, prob_pur, n_pairs; // s_ik
vector<pair<int, int>>accept_assign; // ris_assign[i] = k, user_assign[k] = i
double cur_power_used = 0; // current power used by BS

/* --- greedy --- */
// greedy using obj
void greedy(){
    vector<bool> user_assigned(I, false);
    vector<bool> ris_assigned(K, false);
    priority_queue<pair<double, pair<int, int>>> pq; // {w[i], {i, k}}
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            if(!user_assigned[i] && !ris_assigned[k]){
                pq.push({w[i] * R_user_max[i], {i, k}});
            }
        }
    }
    while(!pq.empty()){
        auto [w_i, pair_ik] = pq.top();
        pq.pop();
        auto [i, k] = pair_ik;
        // check if we can assign user i to RIS k
        if(cur_power_used + R_user_max[i] * (n_pairs[i][k]+1) / (prob_en[i][k] * prob_pur[i][k]) > R_bs_max){
            continue;
        }
        if(!user_assigned[i] && !ris_assigned[k]){
            user_assigned[i] = true;
            ris_assigned[k] = true;
            accept_assign.push_back({i, k});
            cur_power_used += R_user_max[i] * (n_pairs[i][k]+1) / (prob_en[i][k] * prob_pur[i][k]);
        }
    }
}

/* --- output result --- */
void output_accept(){
    ofstream out("data/res/res_greedy_obj.txt");
    if(!out.is_open()){
        cout << "Error: Cannot open file data/output/res_greedy_w.txt" << endl;
        exit(1);
    }
    out << "Accepted assignment: " << endl;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        out << "User " << i << " is assigned to RIS " << k << endl;
    }
    out << "Total number of accepted assignment: " << accept_assign.size() << endl;
    double obj = 0;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        obj += w[i] * R_user_max[i];
    }
    out << "Objective value: " << obj << endl;
    double total_power = 0;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        total_power += R_user_max[i] * (n_pairs[i][k]+1) / (prob_en[i][k] * prob_pur[i][k]);
    }
    out << "Total power usage: " << total_power << endl;
}



/* --- input from dataset --- */
void input_dataset(string dataset_file = "data/raw/dataset.txt"){
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
    n_pairs.resize(I, vector<double>(K));

    for(int i = 0; i < I; i++){ in >> w[i] >> R_user_max[i]; }
    for(int i = 0; i < I; i++){ 
        for(int j = 0; j < K; j++){ 
            in >> prob_en[i][j] >> prob_pur[i][j] >> n_pairs[i][j]; 
            if(prob_pur[i][j] == 0){
                prob_pur[i][j] = 1;
            }
        }
    }
    in.close();
}

int main(){
    input_dataset();
    greedy();
    output_accept();
    return 0;
}