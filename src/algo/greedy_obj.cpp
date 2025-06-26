// greedy weight first and then greedy size
#include<iostream>
#include<chrono>
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
vector<vector<int>> ris_served_user; // ris_served_user[k] = i, user_served[i] = k
map<pair<int, int>, int> can_serve; // serve_map[k, i] = 1: k can serve to i

string infile = "data/raw/dataset.txt";
string outfile = "data/res/res_greedy_obj.txt";

struct UE_RIS {
    int i, k;
    double w, r, s;
    UE_RIS(int i, int k, double w, double r, double s) : i(i), k(k), w(w), r(r), s(s) {}
    bool operator<(const UE_RIS& other) const {
        if (w == other.w) {
            return s > other.s; // Choose the smaller s
        }
        return w < other.w; // Choose the max w * r
    }
};

/* --- greedy --- */
// greedy using obj
void greedy(){
    vector<bool> user_assigned(I, false);
    vector<bool> ris_assigned(K, false);
    priority_queue<UE_RIS> pq; // {w[i], {i, k}}
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            if(!user_assigned[i] && !ris_assigned[k]){
                pq.push({i, k, w[i], R_user_max[i],
                        R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k])});
            }
        }
    }
    while(!pq.empty()){
        auto [i, k, w_i, r_i, s_i] = pq.top();
        pq.pop();
        // check if we can assign user i to RIS k
        if(can_serve.count({k, i}) == 0){
            continue;
        }
        if(cur_power_used + R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]) > R_bs_max){
            continue;
        }
        if(!user_assigned[i] && !ris_assigned[k]){
            user_assigned[i] = true;
            ris_assigned[k] = true;
            accept_assign.push_back({i, k});
            cur_power_used += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
        }
    }
}

/* --- output result --- */
void output_accept(){
    ofstream out(outfile);
    if(!out.is_open()){
        cout << "Error: Cannot open file data/output/res_greedy_w.txt" << endl;
        exit(1);
    }    
    /* Y Label : 
        Objective
        Generation Rate
        Connection Cost
        # Satisfied UEs
    */
    double obj = 0;
    double total_power = 0;
    double tmp_power = 0;    
    double generation_rate = 0;
    double connection_cost = 0;
    double satisfied_ues = 0;
    out << "Accepted assignment: " << endl;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        out << "User " << i << " is assigned to RIS " << k;
        out << " s = " << R_user_max[i] * (n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k]));
        out << " R_user_max " << R_user_max[i] << " prob_en " << prob_en[i][k] << " prob_pur " << prob_pur[i][k];
        out << " n_pairs " << n_pairs[i][k] << endl;
        
        obj += w[i] * R_user_max[i];
        tmp_power += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
        total_power += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
        generation_rate += R_user_max[i];
        connection_cost += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
        satisfied_ues++;
    }
    out << "Total number of accepted assignment: " << accept_assign.size() << endl;
    out << "Objective value: " << obj << endl;
    out << "Total power usage: " << total_power << endl;
    out << "Generation rate: " << generation_rate << endl;
    out << "Connection cost: " << connection_cost << endl;
    out << "# Satisfied UEs: " << satisfied_ues << endl;
    

    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        generation_rate += R_user_max[i];
        connection_cost += n_pairs[i][k];
        satisfied_ues++;
    }
}



/* --- input from dataset --- */
void input_dataset(){
    ifstream in(infile);
    if(!in.is_open()){
        cout << "Error: Cannot open file " << infile << endl;
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
            // assumtion: n_pairs[i][j] > 0, prob_pur[i][j] = 1 when we not doing purification
            in >> prob_en[i][j] >> prob_pur[i][j] >> n_pairs[i][j]; 
        }
    }
    for(int k = 0; k < K; k++){
        int num_served;
        in >> num_served;
        ris_served_user.push_back(vector<int>(num_served));
        for(int i = 0; i < num_served; i++){
            in >> ris_served_user[k][i];
            can_serve[{k, ris_served_user[k][i]}] = 1;
        }
    }
    for(int k = 0; k < K; k++){
        for(int i = 0; i < I; i++){
            if(can_serve.count({k, i}) == 0){
                n_pairs[i][k] = INF;
            }
        }
    }
    in.close();
}

int main(int argc, char* argv[]){
    // start time
    auto start = chrono::high_resolution_clock::now();
    if(argc != 3){
        cout << "Usage: ./greedy_obj <datasetfile> <outfile>" << endl;
        exit(1);
    }
    infile = argv[1];
    outfile = argv[2];
    input_dataset();
    greedy();
    output_accept();
    // end time
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    ofstream log_file("data/res/log.txt", ios::app);
    if (!log_file.is_open()) {
        cout << "Error: Cannot open log file" << endl;
        exit(1);
    }
    log_file << "Obj Time taken: " << duration.count() << " ms" << endl;
    log_file.close();
    cout << "Time taken: " << duration.count() << " ms" << endl;
    return 0;
}