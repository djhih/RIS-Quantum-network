#include<iostream>
#include<vector>
#include"gurobi_c++.h"
using namespace std;

/* 
    \min_{\lambda_{i,k}} \max_{x_{ik}} \sum_{k} ^ K \sum_{i}^{I_k} x_{ik}w_iR(i,k) + \lambda_{i,k}(F_{th,i}-F(i,k)x_{ik})

    x_{ik}R(i,k) \le R_{max,i} , \forall i \in I_k, \forall k \in K
    \sum_{i}^{I_k}x_{ik} <= m_{k} , \forall k\in K
    \sum_{k}^K{x_{ik} <= 1,\forall i \in I_k},
    x_{ik} \in [0,1] , \forall i \in I, \forall k \in K
*/

// parameter
//! assume all users can use all RIS
// R[i][j]: Rate for user i use RIS j
//? maybe we should change the relationship (i for RIS, j for user)

const int K = 3; // # RIS
const int I = 5; // # GS
double w[I] = {1.2, 0.8, 1.5, 1.0, 0.9}; // weight
double R[I][K] = {{10, 12, 15}, {8, 9, 11}, {14, 13, 16}, {11, 10, 14}, {9, 8, 10}}; 
double R_max[I] = {0};
double F_th[I] = {1, 1, 1, 1, 1};  // f[i][j]: Fidelity for the commection. (user i, RIS j and BS)
double F[I][K];
double lambda[I][K];
int m_k[K] = {2, 3, 2}; // capacity limit for RIS

double solveRelaxedProblem(vector<vector<double>>& x) {
    try {
        GRBEnv env = GRBEnv(true);
        env.start();
        GRBModel model = GRBModel(env);

        // set x in [0, 1]
        vector<vector<GRBVar>> x_vars(I, vector<GRBVar>(K));
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                x_vars[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
            }
        }

        // set obj (z)
        GRBLinExpr objective = 0;
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                objective += x_vars[i][k] * w[i] * R[i][k] + lambda[i][k] * (F_th[i] - F[i][k] * x_vars[i][k]);
            }
        }
        model.setObjective(objective, GRB_MAXIMIZE);

        // constraint 1: x_{ik} R(i,k) <= R_{max,i}
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                model.addConstr(x_vars[i][k] * R[i][k] <= R_max[i]);
            }
        }

        // constraint 2: sum_i x_{ik} <= m_k
        for (int k = 0; k < K; ++k) {
            GRBLinExpr sum_x = 0;
            for (int i = 0; i < I; ++i) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= m_k[k]);
        }

        // constraint 3: sum_k x_{ik} <= 1
        for (int i = 0; i < I; ++i) {
            GRBLinExpr sum_x = 0;
            for (int k = 0; k < K; ++k) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= 1);
        }

        model.optimize();

        // get solution
        double obj_value = model.get(GRB_DoubleAttr_ObjVal);
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                x[i][k] = x_vars[i][k].get(GRB_DoubleAttr_X);
            }
        }

        // check constraint 1

        // check constraint 2
        
        // check constraint 3

        return obj_value;

    } catch (GRBException e) {
        cerr << "Error code = " << e.getErrorCode() << ": " << e.getMessage() << endl;
        return -1;
    }
}

int main(){
    
}