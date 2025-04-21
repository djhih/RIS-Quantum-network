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
*/

int main(){
    ofstream out("raw/dataset.txt");
    if(!out.is_open()){
        cout << "Error: Cannot open file raw/dataset.txt" << endl;
        exit(1);
    }
    srand(time(0));
    int I = 10; // number of users
    int K = 5; // number of RISs
    double R_bs_max = 100; // max rate of BS
    out << I << " " << K << endl;
    vector<double> R_user_max(I);
    vector<double> w(I);
    vector<vector<double>> fid_en(I, vector<double>(K));
    vector<vector<double>> fid_pur(I, vector<double>(K));
    vector<vector<double>> prob_en(I, vector<double>(K));
    vector<vector<double>> prob_pur(I, vector<double>(K));
    vector<vector<double>> n_pairs(I, vector<double>(K));
    vector<double> loc_x(I);
    vector<double> loc_y(I);
    vector<double> loc_x_ris(K);
    vector<double> loc_y_ris(K);
    vector<vector<double>> dis(I, vector<double>(K));
    vector<double> dis_bs(K);
    vector<vector<double>> dis_tot(I, vector<double>(K));
    double beta = 0.1; // parameter for entanglement fidelity
    double max_dis = 100; // max distance
    double min_dis = 10; // min distance
    double max_fid = 0.9; // max fidelity
    double min_fid = 0.5; // min fidelity
    // generate random locations for users and RISs
    for(int i = 0; i < I; i++){
        loc_x[i] = rand() % 100;
        loc_y[i] = rand() % 100;
    }
    for(int k = 0; k < K; k++){
        loc_x_ris[k] = rand() % 100;
        loc_y_ris[k] = rand() % 100;
    }
    // generate random distances
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            dis[i][k] = sqrt(pow(loc_x[i] - loc_x_ris[k], 2) + pow(loc_y[i] - loc_y_ris[k], 2));
            dis_tot[i][k] = dis[i][k] + dis_bs[k];
            if(dis[i][k] > max_dis){
                dis[i][k] = max_dis;
            } else if(dis[i][k] < min_dis){
                dis[i][k] = min_dis;
            }
        }
    }
    for(int k = 0; k < K; k++){
        dis_bs[k] = sqrt(pow(loc_x_ris[k] - 50, 2) + pow(loc_y_ris[k] - 50, 2));
        if(dis_bs[k] > max_dis){
            dis_bs[k] = max_dis;
        } else if(dis_bs[k] < min_dis){
            dis_bs[k] = min_dis;
        }
    }
    // generate random max rate for users
    for(int i = 0; i < I; i++){
        R_user_max[i] = rand() % 100 + 1;
        out << R_user_max[i] << " ";
    }
    out << endl;
    // generate random weights for users
    for(int i = 0; i < I; i++){
        w[i] = rand() % 10 + 1;
        out << w[i] << " ";
    }
    out << endl;
    // generate random fidelity for users and RISs
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            fid_en[i][k] = entangle_fidelity(dis[i][k], beta);
            if(fid_en[i][k] > max_fid){
                fid_en[i][k] = max_fid;
            } else if(fid_en[i][k] < min_fid){
                fid_en[i][k] = min_fid;
            }
            out << fid_en[i][k] << " ";
        }
        out << endl;
    }
    // generate random probability for users and RISs
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            prob_en[i][k] = entangle_success_prob(dis[i][k]);
            prob_pur[i][k] = purify_success_prob(fid_en[i][k], fid_pur[i][k]);
            if(prob_en[i][k] > 1){
                prob_en[i][k] = 1;
            } else if(prob_en[i][k] < 0){
                prob_en[i][k] = 0;
            }
            if(prob_pur[i][k] > 1){
                prob_pur[i][k] = 1;
            } else if(prob_pur[i][k] < 0){
                prob_pur[i][k] = 0;
            }
            out << prob_en[i][k] << " ";
        }
        out << endl;
    }
    // generate random number of entanglement pairs for users and RISs
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            n_pairs[i][k] = rand() % 10 + 1;
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
    cout << "fid_en: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << fid_en[i][k] << " ";
        }
        cout << endl;
    }
    cout << "prob_en: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << prob_en[i][k] << " ";
        }
        cout << endl;
    }
    cout << "prob_pur: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << prob_pur[i][k] << " ";
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
    cout << "loc_x: ";
    for(int i = 0; i < I; i++){
        cout << loc_x[i] << " ";
    }
    cout << endl;
    cout << "loc_y: ";
    for(int i = 0; i < I; i++){
        cout << loc_y[i] << " ";
    }
    cout << endl;
    cout << "loc_x_ris: ";
    for(int k = 0; k < K; k++){
        cout << loc_x_ris[k] << " ";
    }
    cout << endl;
    cout << "loc_y_ris: ";
    for(int k = 0; k < K; k++){
        cout << loc_y_ris[k] << " ";
    }
    cout << endl;
    cout << "dis: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << dis[i][k] << " ";
        }
        cout << endl;
    }
    cout << "dis_bs: ";
    for(int k = 0; k < K; k++){
        cout << dis_bs[k] << " ";
    }
    cout << endl;
    cout << "dis_tot: " << endl;
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            cout << dis_tot[i][k] << " ";
        }
        cout << endl;
    }
    return 0;
}