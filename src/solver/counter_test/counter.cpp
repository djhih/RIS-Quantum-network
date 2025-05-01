#include<iostream>
#include<vector>
#include <fstream>
#include <cassert>
#include"gurobi_c++.h"
using namespace std;

int K; // # RIS
int I; // # GS

double w[1000]; // weight
double R_bs_max;
double x[100][100];
double R[100][100];
double V[100]; // w[i] * R_max[i]
double s[100][100];

void input(){
//    ifstream in("src/solver/counter_test/solver_data.txt");
	ifstream in("solver_data.txt");
    	assert(in.is_open());
    in >> K >> I; //
    in >> R_bs_max;
    for(int i=0; i<I; i++) in >> w[i];
    for(int i=0; i<I; i++) in >> V[i];
    for(int i=0; i<I; i++) for(int k=0; k<K; k++) in >> s[i][k];
    in.close();
}

double solveRelaxedProblem() {
    try {
        GRBEnv env = GRBEnv(true);
        env.set(GRB_IntParam_OutputFlag, 0); 
        env.set(GRB_StringParam_LogFile, "gurobi_log.txt");
        env.start();
        GRBModel model = GRBModel(env);
        

        // set x in [0, 1]
        vector<vector<GRBVar>> x_vars(I, vector<GRBVar>(K));
        // vector<vector<GRBVar>> R_in(I, vector<GRBVar>(K)); // R_i, k

        //GRB_INTEGER
        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                x_vars[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS,"x_vars"+to_string(i)+to_string(k));
                // R_in[i][k] = model.addVar(0.0, 1000, 0.0, GRB_CONTINUOUS,"R_in"+to_string(i)+to_string(k));
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
        // model.addConstr(sum_x <= m_k[k]);


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
        cout<<"total power "<<R_bs_max<<'\n';

        for (int i = 0; i < I; ++i) {
            for (int k = 0; k < K; ++k) {
                x[i][k] = x_vars[i][k].get(GRB_DoubleAttr_X);
                R[i][k] = s[i][k]* x_vars[i][k].get(GRB_DoubleAttr_X);
                if(x[i][k] > 0.0001){
                    cout << "x[" << i << "][" << k << "]: " << x[i][k] << ' ';
                    cout << "R_bs for user " << i << " - RIS " << k << " : " << R[i][k] << endl;
                }
                // cout << "x " << i << "  " << k << ": " << x[i][k] << ' ';
                // cout << "R " << i << "  " << k << ": " << R[i][k] << ' ';
                // cout << "R_user " << i << "  " << k << ": " << s[i][k] * x[i][k] << endl;
            }
        }
        cout << "obj val: " << obj_value << '\n';
        return obj_value;

    } catch (GRBException e) {
        cerr << "Error code = " << e.getErrorCode() << ": " << e.getMessage() << endl;
        return -1;
    }
}

int main(){
    input();
    solveRelaxedProblem();
}
