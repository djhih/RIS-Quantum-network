#include<bits/stdc++.h>
#include<cmath>
#include<vector>
#include<algorithm>
#include"gurobi_c++.h"
#include<cassert>
using namespace std;


const double fid_parameter_beta = 0.0044; 
const double pro_parameter_alpha = 0.0002;
const double eps = 0.0001;

int L_it; // L: # of iterations
int I_user; // I: # of users 
int K_ris; // K: # of RIS
int M_cap; // M: # of users that each RIS can serve simultaneously
double target_gap; // target gap: target optimization gap

double x_decision[100][100]; // x[i][k] = if ris_k used by user_i or not
double en_fid[100][100]; // fid[i][k] = fid for ris_k and user_i
double en_pro[100][100]; // probability i k
double pur_n_times[100][100]; // n_i_k
double dis[100][100]; // dis(i, k)
double rate[100][100]; // r(i, k)
double rate_in[100]; // r_in_i
double lambda[100];
double z_ub[100];
double z_lb[100];
double z_val[100];
double z_ans = 0;
double x_ans[100][100] ;

struct User {
	double id ;
	double weight;
	double x_loc, y_loc;
	int cur_use_ris_id;
	int fid_threshold;
	int max_rate_threshold;
	int pur_times[100]; // k for each ris
};

struct RIS {
	double id ;
	double x_loc, y_loc;
	vector<int>cur_served_user_id;
};

User user[100];
RIS ris[100];



/*
* simple entangle fidelity function 
* formula : 1/2 + e^(-beta*distance)/2
*/
double entangle_fid(double distance){
	return 0.5+0.5*exp(-fid_parameter_beta*distance)/2 ;
}

/*
* simple entangle probability function
* formula : e^(-alpha*distance)
*/
double entangle_prob(double distance){
	return exp(-pro_parameter_alpha*distance);
}

/*
* number of pumpping for user i => n_i  
* formula : n_i >= (ln(F_th/(1-F_th))/ln(F_e/(1-F_e)))-1
*/
double number_of_pumping(double threshold , double fid){
	return ceil((log(threshold/(1-threshold))/log(fid/(1-fid)))-1);
}

/*
* purification of fidelity 
* formula : f1*f2/(f1*f2+(1-f1)*(1-f2))
*/
double purification_fid(double f1 , double f2){
	return f1*f2/(f1*f2+(1-f1)*(1-f2));
}

/*
* purification of probability 
* pumping of fidelity 
* formula : f1*f2+(1-f1)*(1-f2)
*/
double pumping_fid(int distance, int n){

	if(n>1){
		return purification_fid(pumping_fid(distance, n-1),entangle_fid(distance));
	}else{
		return entangle_fid(distance);
	}

}

/* formula : f1*f2+(1-f1)*(1-f2) */
double purification_prob(double f1 , double f2){
	return f1*f2+(1-f1)*(1-f2);
}
/*
* pumping of probability
*
*/
double pumping_pro(int distance, int n){

	if(n>1){
		return purification_prob(pumping_fid(distance, n-1),entangle_fid(distance))*entangle_prob(distance)*pumping_pro(distance, n-1);
	}else{
		return entangle_prob(distance);
	}
}
	
void get_dis(){
	for(int i=0; i<I_user; i++){
		for(int k=0; k<K_ris; k++){
			dis[i][k] = sqrt(abs(user[i].x_loc-ris[i].x_loc)*abs(user[i].x_loc-ris[i].x_loc) + 
							 abs(user[i].y_loc-ris[i].y_loc)*abs(user[i].y_loc-ris[i].y_loc));
		}
	}
}
// not sure
void get_fid(){
	for(int i=0; i<I_user; i++){
		for(int k=0; k<K_ris; k++){
			en_fid[i][k] = entangle_fid(dis[i][k]);
			pur_n_times[i][k] = number_of_pumping(user[i].fid_threshold, en_fid[i][k]);
			en_pro[i][k] = entangle_prob(dis[i][k]);
		}
	}
}

void Initialization(){
	get_dis();
	get_fid();

 //debug
	for(int k=0; k<K_ris; k++){
		for(int i=0; i<I_user; i++){
			ris[k].cur_served_user_id.push_back(i);
			rate[i][k] = 10;
		}
	}
	//number_of_pumping();
}

double g_path(){
	// fid[th][i] - f_func(i, k) * x[i][k];
	// f_func => fidelity_count

	double sum_g = 0 ;

	for (int k = 0; k < K_ris; ++k) {
		for  (int i : ris[k].cur_served_user_id) {
			sum_g += x_decision[i][k] * rate[i][k] - user[i].max_rate_threshold;
		}
	}

	return sum_g;
}


double solveRelaxedProblem(double lambda) {
    
    try {
        GRBEnv env = GRBEnv(true);
        env.start();
        GRBModel model = GRBModel(env);

        // set x in [0, 1]
        vector<vector<GRBVar>> x_vars(I_user, vector<GRBVar>(K_ris));
        for (int i = 0; i < I_user; ++i) {
            for (int k = 0; k < K_ris; ++k) {
                x_vars[i][k] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
            }
        }
        

        // set obj (z)
        GRBLinExpr objective = 0;
        for (int k = 0; k < K_ris; ++k) {
            for (int i : ris[k].cur_served_user_id) {
                objective += x_vars[i][k] * user[i].weight * rate[i][k] + lambda * x_vars[i][k] * rate[i][k] - user[i].max_rate_threshold;
				 
			}
        }
		model.setObjective(objective, GRB_MAXIMIZE);
        // constraint 2: sum_i x_{ik} <= m_k
        for (int k = 0; k < K_ris; ++k) {
            GRBLinExpr sum_x = 0;
            for (int i : ris[k].cur_served_user_id) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= M_cap);
        }

        // constraint 3: sum_k x_{ik} <= 1
        for (int i = 0; i < I_user; ++i) {
            GRBLinExpr sum_x = 0;
            for (int k = 0; k < K_ris; ++k) {
                sum_x += x_vars[i][k];
            }
            model.addConstr(sum_x <= 1);
        }

        model.optimize();
        
        double obj_value = model.get(GRB_DoubleAttr_ObjVal);
        
        
        // get solution
        for (int i = 0; i < I_user; ++i) {
            for (int k = 0; k < K_ris; ++k) {
                x_decision[i][k] = x_vars[i][k].get(GRB_DoubleAttr_X);
                cout << "x " << i << " k " << k << ' ' << x_decision[i][k] << '\n';
            }
        }

        cout << "obj val: " << obj_value << '\n';
        // check constraint 1

        // check constraint 2
        
        // check constraint 3
       
        return obj_value ;

    } catch (GRBException e) {
        cerr << "Error code = " << e.getErrorCode() << ": " << e.getMessage() << endl;
        return -1;
    }
}



void path_selection_problem(){
	double g[L_it+1]  ;
	double gap[L_it+1] ;
	double step[L_it+1] ;

	for(int i = 0; i < L_it; i++){ 								// loop L times
		double x[L_it][L_it] ;// remove

		z_val[i] = solveRelaxedProblem(lambda[i]);			// upd x[i][], z_val[i], where x is a 2-D array
		g[i] = g_path(); 										// x generate by lamda_i, g is a number?
		
		// z_val是當前relax problem的答案,尚未賦值
		z_ub[i+1] = min(z_ub[i], z_val[i]); 	//? we should decrease upper-bound
												
		if(g >= 0){

			z_lb[i] = max(z_lb[i], z_val[i] - lambda[i] * g[i]); // lower bound update 

			if(z_val[i] - lambda[i] * g[i] >= z_ans) {	//? why eq
				z_ans = z_val[i] - lambda[i] * g[i];

				for(int a = 0 ; a < I_user ; a++){
					for(int b = 0 ; b < K_ris ; b++){
						x_ans[a][b] = x_decision[a][b];
					}
				}

			}

		} else {
			z_lb[i+1] = z_lb[i];
		}

		gap[i+1] = (z_ub[i+1]-z_lb[i+1]) / z_lb[i+1];
		step[i] = (z_val[i] - z_lb[0]) / sqrt(pow(g[i],2)); //? why abs , step function not sure if correct 
		lambda[i+1] = max(0.0, lambda[i] - step[i] * g[i]);

		if(gap[i+1] <= eps){
			return ;
		}

	}
	cout<<"diverge !\n" ;
	return ;
}

/*
double g_rate(vecot<int>& Rin){
	// Rmax - sigma(Rin[i])
	int Rsum = 0; for(int i=0; i<I; i++) Rsum += Rin[in];
	return  Rmax - Rsum;
}


void gereation_rate_problem(){
	for(int i = 0; i < L; i++){ 								// loop L times
		rate_solver(lamda2[i]);								// upd rin, z_val2
		g2[i] = g_rate(x); 										// x generate by lamda_i, g is a number?
		z_ub2[i] = min(z_ub2[i-1], z_val2[i]); 	//? we should decrease upper-bound
												
		if(g2[i] > 0){
			z_lb2[i] = max(z_lb2[i], z_val2[i] - lamda[i] * g[i]);
			if(z[i] - lamda2[i] * g2[i] >= z_ans) {	//? why eq
				z_ans = z_val2[i] - lmada2[i] * g2[i]; //? should z_val = z_val2 ?
				Rin_ans = rin;
			}

		} else {
			z_lb2[i+1] = z_lb2[i];
		}

		gap2[i+1] = (z_ub2[i+1]-z_lb2[i+1]) / z_lb2[i+1];
		step2[i] = (z_val2[i] - z_lb2[0]) / sqrt(abs(g2[i])); //? why abs
		lmada2[i+1] = max(0, lmada2[i+1] - step2[i] * g2[i]);

		if(gap2[i+1] < eps){
			return;
		}

	}
}

*/




void InputFromFile(const string &filename) {
    ifstream fin(filename);
    if (!fin) {
        cerr << "無法開啟檔案: " << filename << endl;
        exit(1);
    }
    
    // 讀取主要參數
    fin >> L_it >> I_user >> K_ris >> M_cap;
    
    // 讀取用戶資料：x_loc, y_loc, weight
    for (int i = 0; i < I_user; i++) {
        fin >> user[i].x_loc >> user[i].y_loc >> user[i].weight;
    }
    
    // 讀取 RIS 資料：x_loc, y_loc
    for (int i = 0; i < K_ris; i++) {
        fin >> ris[i].x_loc >> ris[i].y_loc;
    }
    
    fin.close();
}


void test(){
	cout <<"hello";
	for (int k = 0; k < K_ris; ++k) {
		for (int i : ris[k].cur_served_user_id) {
			cout << "value "<< user[i].weight * rate[i][k] + lambda[0] * rate[i][k] - user[i].max_rate_threshold <<"\n";
		}
	}


}


int main(int argc, char* argv[]){
	
	// 1. call path_selection_problem with the Rin and update the X
	// 2. call generation_rate_problem with the X and update Rin
	// 3. do 1 untill the answer converge(the answer improve < target_gap)
	string filename;
    if (argc >= 2) {
        filename = argv[1];
    } else {
        filename = "testdata.txt";  // 預設檔案名稱
    }
    
    InputFromFile(filename);
	Initialization();
	//test();
	path_selection_problem();
	//solveRelaxedProblem(1);
}