#include<iostream>
#include<fstream>
#include<random>
#include<ctime>
#include<cmath>
#include<limits>
#include"../formula.h"
using namespace std;

/* This code is used to generate test cases, containing the following data:
    1. I: # of users
    2. K: # of RISs
    3. R_bs_max: max rate of BS
    4. R_user_max[|I|]: max rate of users
    5. w[i]: weight of user i
    6. fid[|I|][|K|]: fidelity of user i to RIS k
    7. prob_en[|I|][|K|]: probability of user i create entanglement with RIS k
    8. prob_pur[|I|][|K|]: probability of user i purify entanglement with RIS k
    9. n[|I|][|K|]: # of entanglement pairs for user i purify with RIS k
    10. loc_x[|I|]: x coordinate of user i
    11. loc_y[|I|]: y coordinate of user i
    12. loc_x[|K|]: x coordinate of RIS k
    13. loc_y[|K|]: y coordinate of RIS k
    14. dis[|I|][|K|]: dis(i, k)
    15. dis_bs[|k|]: dis(k, bs)
    16. dis_tot[|I|][|K|]: dis(i, k) + dis(k, bs)

output: 
    1. I
    2. K
    3. R_bs_max
    4. R_user_max[|I|]
    5. w[i]
    6. prob_en[|I|][|K|]
    7. prob_pur[|I|][|K|]
    8. n[|I|][|K|]
*/

int I = 10; // number of users
int K = 5; // number of RISs
const double beta = 0.00438471;
double R_bs_max = 100; // max rate of BS
const double fidelity_threshold = 0.8;

struct purify_table {
    double dis;
    double fid_en;
    double prob_en;
    double prob_pur;
    vector<double> fid_pur_times;

    purify_table() : dis(0), fid_en(0), prob_en(1), prob_pur(1) {}
    purify_table(double dis, double fid_en, double prob_en, double prob_pur)
        : dis(dis), fid_en(fid_en), prob_en(prob_en), prob_pur(prob_pur) {}
};

int main(){
    ofstream out("raw/dataset.txt");
    if(!out.is_open()){
        cout << "Error: Cannot open file raw/dataset.txt" << endl;
        exit(1);
    }
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist_int(0, 99);
    uniform_real_distribution<> dist_real(0, 100);

    vector<double> R_user_max(I);
    vector<double> w(I);
    vector<vector<double>> n_pairs(I, vector<double>(K));
    vector<pair<double, double>> loc_user(I);
    vector<pair<double, double>> loc_ris(I);
    vector<double> dis_bs(K);
    vector<vector<double>> dis(I, vector<double>(K));
    vector<vector<double>> dis_tot(I, vector<double>(K));
    vector<vector<purify_table>> data_i_k(I, vector<purify_table>(K));

    out << I << " " << K << endl;
    for(int i = 0; i < I; i++){
        int x = dist_int(gen), y = dist_int(gen);
        while(x == 0 && y == 0){
            x = dist_int(gen);
            y = dist_int(gen);
        }
        loc_user[i] = {x, y};
    }
    for(int k = 0; k < K; k++){
        int x = dist_int(gen), y = dist_int(gen);
        while(x == 0 && y == 0){
            x = dist_int(gen);
            y = dist_int(gen);
        }
        loc_ris[k] = {x, y};
    }

    // generate random distances
    // assume the BS is at (0, 0)
    for(int k = 0; k < K; k++){
        dis_bs[k] = sqrt(pow(loc_ris[k].first, 2) + pow(loc_ris[k].second, 2));
    }
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            dis[i][k] = sqrt(pow(loc_user[i].first - loc_ris[k].first, 2) + pow(loc_user[i].second - loc_ris[k].second, 2));
            dis_tot[i][k] = dis[i][k] + dis_bs[k];
            data_i_k[i][k] = purify_table(dis[i][k], 1, 1, 1);
        }
    }

    // generate random max rate for users
    for(int i = 0; i < I; i++){
        R_user_max[i] = dist_int(gen);
        out << R_user_max[i] << " ";
    }
    out << endl;

    // generate random weights for users
    for(int i = 0; i < I; i++){
        uniform_real_distribution<> weight_dist(1, 10);
        w[i] = weight_dist(gen);
        out << w[i] << " ";
    }
    out << endl;

    // generate random fidelity for users and RISs
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            data_i_k[i][k].fid_en = entangle_fidelity(dis[i][k], beta);
            data_i_k[i][k].prob_en = entangle_success_prob(dis[i][k]);
        }
    }

    // generate random probability for users and RISs
    // ! not sure
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            data_i_k[i][k].prob_en = entangle_success_prob(dis[i][k]);
            // try to purify the entanglement until the fidelity over the threshold
            for(int t=0; t<50; t++){
                if(data_i_k[i][k].fid_pur_times.back() >= fidelity_threshold){
                    n_pairs[i][k] = t;
                    break;
                }
                double purify_fid = purify_fidelity(data_i_k[i][k].fid_en, data_i_k[i][k].fid_pur_times.back());  
                data_i_k[i][k].prob_pur *= purify_success_prob(data_i_k[i][k].fid_en, data_i_k[i][k].fid_en);
                data_i_k[i][k].fid_pur_times.push_back(purify_fid);
            }
            out << data_i_k[i][k].prob_en << " ";
        }
        out << endl;
    }

    // we have to count the times needed for user_i, ris_k to purify the entanglement

    // generate random number of entanglement pairs for users and RISs
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            out << n_pairs[i][k] << " ";
        }
        out << endl;
    }
    // generate random distance for users and RISs
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            out << dis[i][k] << " ";
        }
        out << endl;
    }
    // generate random distance for RISs and BS
    for(int k = 0; k < K; k++){
        out << dis_bs[k] << " ";
    }
    out << endl;
    // generate random total distance for users and RISs
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            out << dis_tot[i][k] << " ";
        }
        out << endl;
    }
    out.close();
    cout << "Test case generated successfully!" << endl;
    cout << "I: " << I << endl;
    cout << "K: " << K << endl;
    cout << "R_bs_max: " << R_bs_max << endl;
    cout << "R_user_max: ";
    for(int i = 0; i < I; i++){
        cout << R_user_max[i] << " ";
    }
    cout << endl;
    cout << "w: ";
    for(int i = 0; i < I; i++){
        cout << w[i] << " ";
    }
    cout << endl;
    cout << "prob_en: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << data_i_k[i][k].prob_en << " ";
        }
        cout << endl;
    }
    cout << "prob_pur: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << data_i_k[i][k].prob_pur << " ";
        }
        cout << endl;
    }
    cout << "n_pairs: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << n_pairs[i][k] << " ";
        }
        cout << endl;
    }
    return 0;
}