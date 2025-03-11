#include<bits/stdc++.h>
using namespace std;

void Input(){
	// L: # of iterations
	// target gap: target optimization gap
	// I: # of users 
	// K: # of RIS
	// I_k: Users who can be served by RIS k 
	// l_i: position for each user 
	// l_j: position for each candidate location
	// M: # of users that each RIS can serve simultaneously
	// w_i: weight for each user 
}
	

void Initialization(){
	// x[K][I], fesible answer
	//? r[K][L], I forgot this
	// lamda[L] = vector<int>(0);
	// z_ub[L] obj value, inf
	// z_lb[L] obj value for initial x
	// z_val[L]
	// double z_ans = 0; // record the best z value we got
	// x_ans[k][I] // record the x answer with best z
}

double g_path(vector<int>& x){
	// fid[th][i] - f_func(i, k) * x[i][k];
	// f_func => fidelity_count
	return fid[th][i] - f_func(i, k) * x[i][k];
}

void path_selection_problem(){
	for(int i = 0; i < L; i++){ 								// loop L times
		path_solver(lamda[i]);								// upd x[i][], z_val[i], where x is a 2-D array
		g[i] = g_path(x); 										// x generate by lamda_i, g is a number?
		z_ub[i] = min(z_ub[i-1], z_val[i]); 	//? we should decrease upper-bound
												
		if(g[i] > 0){
			z_lb[i] = max(z_lb[i], z[i] - lamda[i] * g[i]);
			if(z[i] - lamda[i] * g[i] >= z_ans) {	//? why eq
				z_ans = z_val[i] - lmada[i] * g[i];
				x_ans = x;
			}

		} else {
			z_lb[i+1] = z_lb[i];
		}

		gap[i+1] = (z_ub[i+1]-z_lb[i+1]) / z_lb(i+1);
		step[i] = (z_val[i] - z_lb[0]) / sqrt(abs(g[i])); //? why abs
		lmada[i+1] = max(0, lmada[i+1] - step[i] * g[i]);

		if(gap[i+1] < eps){
			return;
		}

	}
}

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

main(){
	// 1. call path_selection_problem with the Rin and update the X
	// 2. call generation_rate_problem with the X and update Rin
	// 3. do 1 untill the answer converge(the answer improve < target_gap)
}