#include<iostream>
#include<chrono>
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

/* --- greedy cp value --- */
void greedy_cp(double cur_power_used){
    // return;
    vector<bool> user_assigned(I, false);
    vector<bool> ris_assigned(K, false);

    for(auto assign: accept_assign){
        auto [i, k] = assign;
        user_assigned[i] = true;
        ris_assigned[k] = true;
    }

    priority_queue<pair<double, pair<int, int>>> pq; // {w[i], {i, k}}
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            if(!user_assigned[i] && !ris_assigned[k]){
                double cp_value = w[i] * (prob_en[i][k] * prob_pur[i][k]) / n_pairs[i][k]; //! not sure cp
                pq.push({cp_value, {i, k}});
            }
        }
    }

    while(!pq.empty()){
        auto [cp_i, pair_ik] = pq.top();
        pq.pop();
        auto [i, k] = pair_ik;
        // check if we can assign user i to RIS k
        if(cur_power_used + R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]) > R_bs_max){
            continue;
        }
        if(!user_assigned[i] && !ris_assigned[k]){
            user_assigned[i] = true;
            ris_assigned[k] = true;
            accept_assign.push_back({i, k});
            cur_power_used += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
            cout << "greedy add " << i << " " << k << endl;
        }
    }
}

/* --- process data from solver and do mathcing --- */
void data_process(){
    map<pair<int, int>, double> rem_pair_x;
    vector<int> user_cnt_ris(I, 0);
    
    for(int i=0; i<I; i++){
        for(int k=0; k<K; k++){
            if(x[i][k] == 1){
                accept_assign.push_back({i, k});
                user_cnt_ris[i] ++;
            } else if(x[i][k] > 0 && x[i][k] < 1 && can_serve.count({k, i}) > 0){
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
        if(i1 != i2 && k1 != k2){
            cout << "user " << i1 << " is assigned to RIS " << k1 << " = " << it1->second << endl;
            cout << "user " << i2 << " is assigned to RIS " << k2 << " = " << it2->second << endl;
            cout << "Error: Remaining pairs are not user-ris1, user-ris2 pair." << endl;
            exit(1);
        } else if(i1 != i2 && k1 == k2){
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
                current_useage += R_user_max[i2] * n_pairs[i2][k2] / (prob_en[i2][k2] * prob_pur[i2][k2]);

                //! try to add more ans from all user
                greedy_cp(current_useage);

            } else {
                // move power from k2 to k1
                if(power1 + current_useage > R_bs_max){
                    cout << "Error: Cannot move power from k2 to k1." << endl;
                    exit(1);
                }
                accept_assign.push_back({i1, k1});
                compare_pair = {i2, k2};
                current_useage += R_user_max[i1] * n_pairs[i1][k1] / (prob_en[i1][k1] * prob_pur[i1][k1]);

                //! try to add more ans from all user
                greedy_cp(current_useage);
            }
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
                current_useage += R_user_max[i2] * n_pairs[i2][k2] / (prob_en[i2][k2] * prob_pur[i2][k2]);
                //! try to add more ans from all user
                greedy_cp(current_useage);

            } else {
                // move power from k2 to k1
                if(power1 + current_useage > R_bs_max){
                    cout << "Error: Cannot move power from k2 to k1." << endl;
                    exit(1);
                }
                accept_assign.push_back({i1, k1});
                compare_pair = {i2, k2};
                current_useage += R_user_max[i1] * n_pairs[i1][k1] / (prob_en[i1][k1] * prob_pur[i1][k1]);

                //! try to add more ans from all user
                greedy_cp(current_useage);

            }
        }
    } else if(rem_pair_x.size() == 1){
        // cout << "remaining pairs == 1" << endl;
        compare_pair = {rem_pair_x.begin()->first};
        // cout << "compare pair " << compare_pair.first << " " << compare_pair.second << endl;
    }

    // if remaining pairs == 1, just use the current answer set.
    if(rem_pair_x.empty()){
        cout << "remaining pairs == 0" << endl;
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
    // cout << "call comapre " << '\n';
    double cur_obj = 0;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        cur_obj += w[i] * R_user_max[i];
    }

    double cmp_obj = 0;
    auto [i, k] = compare_pair;
    cmp_obj += w[i] * R_user_max[i];
    // cout << "compare pair " << i << " " << k << endl;
    
    double cmp_useage = 0;
    cmp_useage += R_user_max[i] * n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k]);
    if(cmp_useage > R_bs_max){
        return;
    }

    vector<pair<int, int>> cmp_pairs;
    cmp_pairs.push_back(compare_pair);

    // ! sort all unserved users by cp_value = w[i] * (prob_en[i][k] * prob_pur[i][k]) / n_pairs[i][k];

    vector<bool> user_assigned(I, false);
    vector<bool> ris_assigned(K, false);

    for(auto assign: cmp_pairs){
        auto [i, k] = assign;
        user_assigned[i] = true;
        ris_assigned[k] = true;
    }

    priority_queue<pair<double, pair<int, int>>> pq; // {w[i], {i, k}}
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            if(!user_assigned[i] && !ris_assigned[k]){
                double cp_value = w[i] * (prob_en[i][k] * prob_pur[i][k]) / n_pairs[i][k]; //! not sure cp
                pq.push({cp_value, {i, k}});
            }
        }
    }

    while(!pq.empty()){
        auto [cp_i, pair_ik] = pq.top();
        pq.pop();
        auto [i, k] = pair_ik;
        // check if we can assign user i to RIS k
        if(cmp_useage + R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]) > R_bs_max){
            continue;
        }
        if(!user_assigned[i] && !ris_assigned[k]){
            user_assigned[i] = true;
            ris_assigned[k] = true;
            cmp_pairs.push_back({i, k});
            cmp_useage += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
            cmp_obj += w[i] * R_user_max[i];
        }
    }

    cout << "cur obj " << cur_obj << " cmp obj " << cmp_obj << endl;

    if(cmp_obj > cur_obj){
        cout << "cur obj " << cur_obj << endl;
        cout << "compare obj " << cmp_obj << endl;
        accept_assign = cmp_pairs;
    }

    // sort cmp_usage by cp_value = w[i] * (prob_en[i][k] * prob_pur[i][k]) / n_pairs[i][k];
    // sort(accept_assign.begin(), accept_assign.end(), [](pair<int, int> a, pair<int, int> b){
    //     auto [i1, k1] = a;
    //     auto [i2, k2] = b;
    //     return (w[i1] * (prob_en[i1][k1] * prob_pur[i1][k1]) / n_pairs[i1][k1]) > (w[i2] * (prob_en[i2][k2] * prob_pur[i2][k2]) / n_pairs[i2][k2]);
    // });

    // try to add more ans from accept_assign
    // for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
    //     auto [i, k] = *it;
    //     // count usage if we add this pair
    //     cmp_useage += R_user_max[i] * n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k]);
    //     if(cmp_useage > R_bs_max){
    //         cmp_useage -= R_user_max[i] * n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k]);
    //         continue;
    //     } else {
    //         cmp_pairs.push_back({i, k});
    //         compare_obj += w[i] * R_user_max[i];
    //     }
    // }
    // cout << "cmp useage " << cmp_useage << endl;
    // cout << "compare obj " << compare_obj << endl;
    // if(compare_obj > cur_obj){
    //     cout << "cur obj " << cur_obj << endl;
    //     cout << "compare obj " << compare_obj << endl;
    //     accept_assign.erase(accept_assign.begin(), accept_assign.end());
    //     accept_assign.push_back(compare_pair);
    //     accept_assign.clear();
    //     accept_assign = cmp_pairs;
    // }
    // return;

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
    for(int k = 0; k < K; k++){
        for(int i = 0; i< I; i++){
            if(can_serve.count({k, i}) == 0){
                n_pairs[i][k] = 1e9;
            }
        }
    }
    // if user i is not served by any RIS, then set prob_en[i][k] = 0
    in.close();
}

int main(int argc, char *argv[]){
    // start time
    auto start = chrono::high_resolution_clock::now();
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
    // end time in ms
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);   
    ofstream log_file("data/res/log.txt", ios::app);
    if (!log_file.is_open()) {
        cout << "Error: Cannot open log file" << endl;
        exit(1);
    }
    log_file << "a0 Time taken: " << duration.count() << " ms" << endl;
    log_file.close();
    cout << "Time taken: " << duration.count() << " ms" << endl;
    return 0;
}