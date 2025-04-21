/*
    3/13 need to fix \sum^I \sum^K and set lambda
*/

#include<iostream>
#include<vector>
#include <fstream>
#include"gurobi_c++.h"
using namespace std;


int K; // # RIS
int I; // # GS

double w[1000]; // weight

double r_w[1000][1000]; // const

double R_max[1000]; // user limit

double R_bs_max;

double x[100][100];
double R[100][100];
double V[100]; // w[i] * R_max[i]
double s[100][100];


void writer(){

    ofstream outFile("decision_file.txt");

    if (!outFile) {
        cerr << "無法開啟檔案進行寫入！" << endl;
        return ;
    }
    outFile << I ;
    outFile <<endl ;
    outFile << K ;
    outFile <<endl ;

    // 將陣列寫入檔案
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < K; j++) {
            outFile << x[i][j] << " ";
        }
        outFile << endl;
    }

    for(int i = 0 ; i < I ; i++){
        for(int j = 0 ; j < K ; j++){
            outFile << r_w[i][j] <<" ";
        }
    }
    outFile << endl;

    for(int i = 0 ; i < I ; i++){
        for(int j = 0 ; j < K ; j++){
            outFile << R[i][j] << " ";
        }
    }
    outFile << endl;

    for(int i = 0 ; i < I ; i++){
        outFile<< R_max[i] << " ";
    }
    outFile << endl;
    
    for(int i = 0 ; i < I ;i++){
        outFile<<w[i] <<" ";
    }

    outFile.close();
    // cout << "陣列已成功寫入檔案" << endl;

}


void input(){
    ifstream in("solver_data.txt");
    in >> K >> I;
    for(int i=0; i<I; i++) in >> w[i];
    for(int i=0; i<I; i++) for(int k=0; k<K; k++) in >> r_w[i][k];
    for(int i=0; i<I; i++) in >> R_max[i];
    in >> R_bs_max;
    for(int i=0; i<I; i++) V[i] = w[i] * R_max[i];
    for(int i=0; i<I; i++) for(int k=0; k<K; k++) s[i][k] = R_max[i] / r_w[i][k];
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
                R[i][k] = R_max[i] * x_vars[i][k].get(GRB_DoubleAttr_X);

               cout <<"user "<< i << " power : "<< R[i][k] << '\n';
               cout << "x : i " << i << " k " << k << ' ' << "decision "<< x[i][k] <<" obj "<<w[i] * r_w[i][k]*R[i][k]<< " w' : "<< w[i] * r_w[i][k] <<" w : " << w[i] <<" r_w "<<r_w[i][k] <<" max rate "<<R_max[i] <<" user rate "<<r_w[i][k]*R[i][k]<< '\n';
                
            }
        }
        writer();
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