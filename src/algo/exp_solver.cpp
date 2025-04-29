#include<iostream>
#include<vector>
#include <fstream>
#include"gurobi_c++.h"
#include"../formula.h"
using namespace std;

/* --- Global Variables --- */
const int INF = 1e9;
int I, K;
double R_bs_max;
vector<double> R_user_max, w;
vector<vector<double>> prob_en, prob_pur, n_pairs; // s_ik
vector<pair<int, int>>accept_assign; // ris_assign[i] = k, user_assign[k] = i
// problem formulatoin
vector<double> V;
vector<vector<double>> s, r_w, R_user, x;
map<pair<int, int>, int> can_serve; // serve_map[k, i] = 1: k can serve to i

string infile = "data/raw/dataset.txt";
string outfile = "data/res/res_solver.txt";

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
    V.resize(I);
    s.resize(I, vector<double>(K));
    r_w.resize(I, vector<double>(K));
    R_user.resize(I, vector<double>(K));
    x.resize(I, vector<double>(K));

    for(int i = 0; i < I; i++){ in >> w[i] >> R_user_max[i]; }
    for(int i = 0; i < I; i++){ 
        for(int j = 0; j < K; j++){ 
            in >> prob_en[i][j] >> prob_pur[i][j] >> n_pairs[i][j]; 
        }
    }
    for(int k = 0; k < K; k++){
        int num_served;
        in >> num_served;
        for(int i = 0; i < num_served; i++){
            int user_id;
            in >> user_id;
            can_serve[{k, user_id}] = 1;
        }
    }
    in.close();
}

/* --- trans data to fit formulation*/
void data_process(){
    // v[i] = w[i] * R_user_max[i]
    // s[i][k] = n_pairs[i][k] / (pe[i][k] * pur[i][k])

    for(int i = 0; i < I; i++){
        V[i] = w[i] * R_user_max[i];
        for(int k = 0; k < K; k++){
            if(prob_en[i][k] == 0 || prob_pur[i][k] == 0){
                // cout << "Error: prob_en or prob_pur is 0." << endl;
                exit(1);
            }
            if(can_serve.count({k, i}) == 0){
                s[i][k] = INF;
            } else {
                s[i][k] = (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]) * R_user_max[i];
            }
            
            r_w[i][k] = R_user_max[i] / s[i][k];
        }
    }

}

/* --- solver --- */
double solver(){
    try {
        GRBEnv env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 0); 
        // env.set(GRB_StringParam_LogFile, "gurobi_log.txt");
        env.start();
        GRBModel model = GRBModel(env);
        

        vector<vector<GRBVar>> x_vars(I, vector<GRBVar>(K));

        //GRB_INTEGER
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                x_vars[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS,"x_vars"+to_string(i)+to_string(k));
            }
        }
        

        // set obj (z)
        GRBLinExpr objective = 0;
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                objective +=  V[i] * x_vars[i][k];
            }
        }
        model.setObjective(objective, GRB_MAXIMIZE);

        // constraint 1 : sum R_in_i < R^BS_max
        GRBLinExpr sum_r =  0 ; 
        for(int k = 0 ; k < K ; k++){
            for(int i = 0 ; i < I ; i++){
                sum_r += s[i][k] * x_vars[i][k];
            }
        }
        model.addConstr(sum_r <= R_bs_max);

        //constraint 3: sum_i x_{ik} <= m_k
        for (int k = 0; k < K; ++k) {
            GRBLinExpr sum_x = 0;
            for (int i = 0; i < I; ++i) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= 1);
        }

        // constraint 4: sum_k x_{ik} <= 1
        for (int i = 0; i < I; ++i) {
            GRBLinExpr sum_x = 0;
            for (int k = 0; k < K; ++k) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= 1);
        }

        model.optimize();
        double obj_value = model.get(GRB_DoubleAttr_ObjVal);        
        
        // get solution
        ofstream out(outfile);
        if(!out.is_open()){
            cout << "Error: Cannot open file data/output/res_greedy_w.txt" << endl;
            exit(1);
        }
        for(int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                out << x_vars[i][k].get(GRB_DoubleAttr_X) << " ";
            }
            out << '\n';
        }
        for(int i=0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                out << R_user[i][k] << " ";
            }
            out << '\n';
        }
        out.close();
        cout << "total power " << R_bs_max << '\n';
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                
                x[i][k] = x_vars[i][k].get(GRB_DoubleAttr_X);
                R_user[i][k] = R_user_max[i] * x_vars[i][k].get(GRB_DoubleAttr_X);

                if(x[i][k] == 0){
                    continue;
                }

                cout << "user " << i << " power : " << R_user[i][k] << '\n';
                cout << "x : i " << i << " k " << k << " decision " << x[i][k] << " obj " << w[i] * r_w[i][k] * R_user[i][k] << " w' : "<< w[i] * r_w[i][k] <<" w : " << w[i] <<" r_w "<<r_w[i][k] <<" max rate "<<R_user_max[i] <<" user rate "<<r_w[i][k]*R_user[i][k]<< '\n';
            
            }
        }

        cout << "obj val: " << obj_value << '\n';
        return obj_value;

    } catch (GRBException e) {
        cerr << "Error code = " << e.getErrorCode() << ": " << e.getMessage() << endl;
        return -1;
    }
}

int main(int argc, char *argv[]){
    if(argc != 3){
        cout << "Usage: ./solver <datasetfile> <outfile>" << endl;
        exit(1);
    }
    infile = argv[1];
    outfile = argv[2];
    input_dataset();
    data_process();
    solver();
}