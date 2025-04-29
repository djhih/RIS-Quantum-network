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
const int INF = 1e9;
int I, K;
double R_bs_max;
vector<double> R_user_max, w;
vector<vector<double>> prob_en, prob_pur, n_pairs; // s_ik
vector<vector<double>> x, R_user; 
vector<pair<int, int>>accept_assign; // ris_assign[i] = k, user_assign[k] = i
pair<int, int> compare_pair; // pair of user and RIS to compare
map<pair<int, int>, int> can_serve; // serve_map[k, i] = 1: k can serve to i
vector<vector<int>> ris_served_user; // ris_served_user[k] = i, user_served[i] = k

string infile = "data/raw/dataset.txt";
string outfile = "data/res/res_greedy_w.txt";

/* --- process data from solver and do mathcing --- */
void data_process(){
    map<pair<int, int>, double> rem_pair_x;
    vector<int> user_cnt_ris(I, 0);
    
    for(int i=0; i<I; i++){
        for(int k=0; k<K; k++){
            if(x[i][k] == 1){
                accept_assign.push_back({i, k});
                user_cnt_ris[i] ++;
            } else if(x[i][k] > 0 && x[i][k] < 1){
                rem_pair_x[{i, k}] = x[i][k];
            }
        }
    }

    // check if solver assign one user to more than three RISs
    
    int cnt_less_1 = 0;
    for(auto it = rem_pair_x.begin(); it != rem_pair_x.end(); it++){
        auto [i, k] = it->first;
        user_cnt_ris[i] += x[i][k];
    }
    for(int i=0; i<I; i++){
        if(user_cnt_ris[i] < 1 && user_cnt_ris[i] > 0){
            cout << "Error: user " << i << " is assigned to less than 1 RIS." << endl;
            cnt_less_1 ++;
        }
    }
    if(cnt_less_1 > 2){
        cout << "Error: less than 1 more than 2." << endl;
        exit(1);
    }

    // check the remaining pairs
    // if remaining pairs > 2, then drop dataset
    if(rem_pair_x.size() > 2){
        cout << "Error: Remaining pairs > 2." << endl;
        exit(1);
    }

    // if remaining pairs == 2, 
    //   then check if it's a user-ris1, user-ris2 pair
    //   if not, then exit
    //   if yes, then try to move one to the lower power used pair
    if(rem_pair_x.size() == 2){
        auto it1 = rem_pair_x.begin();
        auto it2 = ++rem_pair_x.begin();
        auto [i1, k1] = it1->first;
        auto [i2, k2] = it2->first;
        if(i1 != i2){
            cout << "Error: Remaining pairs are not user-ris1, user-ris2 pair." << endl;
            exit(1);
        } else {
            // try move power from one to another
            double power1 = R_user_max[i1] * n_pairs[i1][k1] / (prob_en[i1][k1] * prob_pur[i1][k1]);
            double power2 = R_user_max[i2] * n_pairs[i2][k2] / (prob_en[i2][k2] * prob_pur[i2][k2]);
            double current_useage = 0;
            for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
                auto [i, k] = *it;
                current_useage += R_user_max[i] * n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k]);
            }

            if(power1 > power2){
                // move power from k1 to k2
                if(power2 + current_useage > R_bs_max){
                    cout << "Overload: Cannot move power from k1 to k2." << endl;
                    exit(1);
                }
                accept_assign.push_back({i2, k2});
                compare_pair = {i1, k1};
            } else {
                // move power from k2 to k1
                if(power1 + current_useage > R_bs_max){
                    cout << "Error: Cannot move power from k2 to k1." << endl;
                    exit(1);
                }
                accept_assign.push_back({i1, k1});
                compare_pair = {i2, k2};
            }
        }
    }

    // if remaining pairs == 1, just use the current answer set.
    if(rem_pair_x.empty()){
        return;
    }
    compare_pair = {rem_pair_x.begin()->first};
    return;
}

void compare(){
    // 1. count current accepted assignment obj value
    // 2. count compare pair obj value
    // 3. if compare pair > current, then swap
    // 4. else, do nothing
    double cur_obj = 0;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        cur_obj += w[i] * R_user_max[i];
    }
    double compare_obj = 0;
    auto [i, k] = compare_pair;
    compare_obj += w[i] * R_user_max[i];
    if(compare_obj > cur_obj){
        accept_assign.erase(accept_assign.begin(), accept_assign.end());
        accept_assign.push_back(compare_pair);
    }
    return;

}

/* --- output result --- */
void output_accept(){
    ofstream out(outfile);
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
        total_power += R_user_max[i] * n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k]);
    }
    out << "Total power usage: " << total_power << endl;
}

/* --- input from solver generated file --- */
void input_solver_gen_data(string solver_gen_file = "data/res/res_solver.txt"){
    ifstream in(solver_gen_file);
    if(!in.is_open()){
        cout << "Error: Cannot open file " << solver_gen_file << endl;
        exit(1);
    }
    x.resize(I, vector<double>(K));
    R_user.resize(I, vector<double>(K));
    for(int i = 0; i < I; i++){ for(int k = 0; k < K; k++){ in >> x[i][k]; }}
    for(int i = 0; i < I; i++){ for(int k = 0; k < K; k++){ in >> R_user[i][k]; }}
    in.close();
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
            in >> prob_en[i][j] >> prob_pur[i][j] >> n_pairs[i][j]; 
            if(prob_pur[i][j] == 0){
                prob_pur[i][j] = 1;
            }
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
    // if user i is not served by any RIS, then set prob_en[i][k] = 0
    in.close();
}

int main(int argc, char *argv[]){
    if(argc != 3){
        cout << "Usage: ./greedy_w <datasetfile> <outfile>" << endl;
        exit(1);
    }
    infile = argv[1];
    outfile = argv[2];
    input_dataset();
    input_solver_gen_data();
    data_process();
    compare();
    output_accept();
    return 0;
}