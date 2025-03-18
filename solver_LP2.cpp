#include <iostream>
#include <vector>
#include "gurobi_c++.h"

using namespace std;

const int K = 2; // # of RIS
const int I = 2; // # of GS

double w[I] = {1.2, 0.8}; // weight
double r_w[I][K] = {{4, 2}, {4, 5}}; // rate coefficients
double R_max[I] = {30, 40}; // max rate per user
int R_bs_max = 100; // total power limit
double x[100][100];
double R[100];

double solveOptimizedProblem() {
    try {
        // Create the Gurobi environment and enable logging
        GRBEnv env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 1);  // Enable solver log
        env.set(GRB_StringParam_LogFile, "gurobi_log.txt"); // Write log to file
        env.start();

        GRBModel model = GRBModel(env);

        // Decision variables
        vector<vector<GRBVar>> x_vars(I, vector<GRBVar>(K));
        vector<GRBVar> R_in(I);

        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                x_vars[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "x_" + to_string(i) + "_" + to_string(k));
            }
            R_in[i] = model.addVar(0.0, 100, 0.0, GRB_CONTINUOUS, "R_in_" + to_string(i));
        }

        // Objective function: maximize weighted sum of allocated power
        GRBLinExpr objective = 0;
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                objective += w[i] * r_w[i][k] * R_in[i];
            }
        }
        model.setObjective(objective, GRB_MAXIMIZE);

        // Constraint 1: Total power limit
        GRBLinExpr sum_r = 0;
        for (int i = 0; i < I; i++) {
            sum_r += R_in[i];
        }
        model.addConstr(sum_r <= R_bs_max, "Total_Power_Limit");

        // Constraint 2: Ensure rate proportionally to x
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                model.addConstr((r_w[i][k] * R_in[i]) / R_max[i] <= x_vars[i][k], "Rate_Constraint_" + to_string(i) + "_" + to_string(k));
            }
        }

        // Constraint 3: RIS capacity constraint
        for (int k = 0; k < K; ++k) {
            GRBLinExpr sum_x = 0;
            for (int i = 0; i < I; ++i) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= 1, "RIS_Capacity_" + to_string(k));
        }

        // Constraint 4: Each user can only use one RIS at most
        for (int i = 0; i < I; ++i) {
            GRBLinExpr sum_x = 0;
            for (int k = 0; k < K; ++k) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= 1, "User_RIS_Allocation_" + to_string(i));
        }

        // Solve the model
        model.optimize();

        // Retrieve the optimal solution
        double obj_value = model.get(GRB_DoubleAttr_ObjVal);

        cout << "Total Power Limit: " << R_bs_max << '\n';
        for (int i = 0; i < I; ++i) {
            R[i] = R_in[i].get(GRB_DoubleAttr_X);
            cout << "User " << i << " Power: " << R[i] << '\n';
            for (int k = 0; k < K; ++k) {
                x[i][k] = x_vars[i][k].get(GRB_DoubleAttr_X);
                cout << "x(" << i << ", " << k << ") = " << x[i][k] 
                     << " | Max Rate: " << R_max[i] 
                     << " | User Rate: " << r_w[i][k] * R[i] << '\n';
            }
        }

        cout << "Optimal Objective Value: " << obj_value << '\n';
        return obj_value;

    } catch (GRBException e) {
        cerr << "Gurobi Error: " << e.getErrorCode() << " - " << e.getMessage() << endl;
        return -1;
    }
}

int main() {
    solveOptimizedProblem();
    return 0;
}