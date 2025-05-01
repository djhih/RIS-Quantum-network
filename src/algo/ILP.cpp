#include <iostream>
 #include <vector>
 #include <fstream>
 #include <map>
 #include <utility>
 #include <string>
 #include "gurobi_c++.h"
 #include "../formula.h"          // ← 若不需要可移除
 
 using namespace std;
 
 /* ---------- Global parameters & containers ---------- */
 const int INF = 1e9;
 
 int    I, K;                      // #users, #RIS
 double R_bs_max;                  // BS power budget
 
 vector<double> R_user_max, w;
 vector<vector<double>> prob_en, prob_pur, n_pairs;          // raw data
 vector<vector<double>> s, r_w, R_user, x;                   // model params / sol
 vector<double> V;
 
 /* can_serve[k,i]=1 if RIS k can serve user i */
 map<pair<int,int>, int> can_serve;
 
 /* file paths (overwritten by argv) */
 string infile  = "data/raw/dataset.txt";
 string outfile = "data/res/res_solver.txt";
 
 /* ---------- read dataset ---------- */
 void input_dataset() {
     ifstream in(infile);
     if (!in.is_open()) {
         cerr << "Error: cannot open " << infile << '\n';
         exit(1);
     }
 
     in >> I >> K;
     in >> R_bs_max;
 
     R_user_max.resize(I);
     w.resize(I);
     prob_en.assign(I, vector<double>(K));
     prob_pur.assign(I, vector<double>(K));
     n_pairs.assign(I, vector<double>(K));
 
     V.resize(I);
     s.assign(I, vector<double>(K));
     r_w.assign(I, vector<double>(K));
     R_user.assign(I, vector<double>(K));
     x.assign(I, vector<double>(K));
 
     for (int i = 0; i < I; ++i) in >> w[i] >> R_user_max[i];
 
     for (int i = 0; i < I; ++i)
         for (int k = 0; k < K; ++k)
             in >> prob_en[i][k] >> prob_pur[i][k] >> n_pairs[i][k];
 
     for (int k = 0; k < K; ++k) {
         int num_served;  in >> num_served;
         for (int t = 0; t < num_served; ++t) {
             int uid;  in >> uid;
             can_serve[{k, uid}] = 1;
         }
     }
     in.close();
 }
 
 /* ---------- preprocess for formulation ---------- */
 void data_process() {
     for (int i = 0; i < I; ++i) {
         V[i] = w[i] * R_user_max[i];
         for (int k = 0; k < K; ++k) {
             if (prob_en[i][k] == 0 || prob_pur[i][k] == 0) exit(1);
 
             if (!can_serve.count({k, i})) {
                 s[i][k] = INF;           // infeasible assignment
             } else {
                 s[i][k] = n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k])
                           * R_user_max[i];
             }
             r_w[i][k] = R_user_max[i] / s[i][k];   // for logging only
         }
     }
 }
 
 /* ---------- MILP solver ---------- */
 double solve_milp() {
     try {
         GRBEnv   env(true);
         env.set(GRB_IntParam_OutputFlag, 0);  // mute log
         env.start();
         GRBModel model(env);
 
         /* decision vars: x[i][k] ∈ {0,1} */
         vector<vector<GRBVar>> xvar(I, vector<GRBVar>(K));
         for (int i = 0; i < I; ++i)
             for (int k = 0; k < K; ++k)
                 xvar[i][k] = model.addVar(
                     0.0, 1.0, 0.0, GRB_BINARY,
                     "x_" + to_string(i) + "_" + to_string(k));
 
         /* objective: maximise Σ_i Σ_k V_i * x_ik */
         GRBLinExpr obj = 0;
         for (int i = 0; i < I; ++i)
             for (int k = 0; k < K; ++k)
                 obj += V[i] * xvar[i][k];
         model.setObjective(obj, GRB_MAXIMIZE);
 
         /* 1) BS power budget: Σ_i Σ_k s_ik * x_ik ≤ R_bs_max */
         GRBLinExpr sumP = 0;
         for (int i = 0; i < I; ++i)
             for (int k = 0; k < K; ++k)
                 sumP += s[i][k] * xvar[i][k];
         model.addConstr(sumP <= R_bs_max, "BS_budget");
 
         /* 2) at most one user per RIS */
         for (int k = 0; k < K; ++k) {
             GRBLinExpr sumX = 0;
             for (int i = 0; i < I; ++i) sumX += xvar[i][k];
             model.addConstr(sumX <= 1, "RIS_cap_" + to_string(k));
         }
 
         /* 3) each user at most one RIS */
         for (int i = 0; i < I; ++i) {
             GRBLinExpr sumX = 0;
             for (int k = 0; k < K; ++k) sumX += xvar[i][k];
             model.addConstr(sumX <= 1, "User_cap_" + to_string(i));
         }
 
         model.optimize();
 
         double z = model.get(GRB_DoubleAttr_ObjVal);

         /* ---------- write & print result ---------- */
         ofstream out(outfile);
         if (!out.is_open()) {
             cerr << "Error: cannot open " << outfile << '\n';  exit(1);
         }

         // 收集所有被接受的分配
         vector<pair<int, int>> accept_assign;
         for (int i = 0; i < I; ++i) {
             for (int k = 0; k < K; ++k) {
                 double val = xvar[i][k].get(GRB_DoubleAttr_X);
                 x[i][k] = val;
                 R_user[i][k] = R_user_max[i] * val;
                 if (val > 0.5) {  // chosen
                     accept_assign.push_back({i, k});
                 }
             }
         }

         /* Y Label : 
            Objective
            Generation Rate
            Connection Cost
            # Satisfied UEs
        */
        double objj = 0;
        double total_power = 0;
        double tmp_power = 0;    
        double generation_rate = 0;
        double connection_cost = 0;
        double satisfied_ues = 0;

         cout << "accept size " << accept_assign.size() << endl;
         out << "Accepted assignment: " << endl;
         for (auto it = accept_assign.begin(); it != accept_assign.end(); it++) {
             auto [i, k] = *it;
             out << "User " << i << " is assigned to RIS " << k << endl;
            //  out << " s = " << R_user_max[i] * (n_pairs[i][k] / (prob_en[i][k] * prob_pur[i][k]));
            //  out << " R_user_max " << R_user_max[i] << " prob_en " << prob_en[i][k] << " prob_pur " << prob_pur[i][k];
            //  out << " n_pairs " << n_pairs[i][k] << endl;
             
             objj += w[i] * R_user_max[i];
             tmp_power += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
             total_power += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
             generation_rate += R_user_max[i];
             connection_cost += R_user_max[i] * (n_pairs[i][k]) / (prob_en[i][k] * prob_pur[i][k]);
             satisfied_ues++;
         }
         out << "Total number of accepted assignment: " << accept_assign.size() << endl;
         out << "Objective value: " << objj << endl;
         out << "Total power usage: " << total_power << endl;
         out << "Generation rate: " << generation_rate << endl;
         out << "Connection cost: " << connection_cost << endl;
         out << "# Satisfied UEs: " << satisfied_ues << endl;

         // 也保留原來的控制台輸出，以便於看到更詳細的信息
         cout << "Total BS power limit = " << R_bs_max << '\n';
         cout << "---------------------------------------\n";
         for (auto [i, k] : accept_assign) {
             cout << "user " << i << " ← RIS " << k
                  << " | P=" << R_user[i][k]
                  << " | obj+=" << V[i]
                  << " | s=" << s[i][k] << '\n';
         }
         cout << "---------------------------------------\n";
         cout << "Optimal objective = " << z << "\n\n";

         out.close();
         return z;

     } catch (GRBException &e) {
         cerr << "Gurobi error " << e.getErrorCode()
              << ": " << e.getMessage() << '\n';
         return -1;
     }
 }
 
 /* ---------- main ---------- */
 int main(int argc, char* argv[]) {
     if (argc != 3) {
         cerr << "Usage : ./solver_bin <datasetfile> <outfile>\n";
         return 1;
     }
     infile  = argv[1];
     outfile = argv[2];
 
     input_dataset();
     data_process();
     solve_milp();
     return 0;
 }
