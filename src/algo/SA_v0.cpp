#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <algorithm> // use shuffle for randomization?
#include <random>
#include <fstream>
#include"../formula.h"
using namespace std;

/* --- Global Variables --- */
const int INF = 1e9;
int I, K;
double R_bs_max;
vector<double> R_user_max, w;
vector<vector<double>> prob_en, prob_pur, n_pairs; // s_ik
vector<pair<int, int>>accept_assign; // ris_assign[i] = k, user_assign[k] = i
double R_user[100];
// double R_user[100][100]; // R_user[i][k] \in [0, R_user_max[i]]: rate of user i with RIS k
double cur_power_used = 0;
vector<vector<int>> ris_served_user;
// map<pair<int, int>, int> can_serve;


string infile = "data/raw/dataset.txt";
string outfile = "data/res/res_greedy_cp.txt";

// L: # of iterations
// I: # of users
// K: # of RIS
// I_k: Users who can be served by RIS k
// l_i: position for each user
// l_j: position for each candidate location
// M: # of users that each RIS can serve simultaneously
// m_k: # of users each RIS can serve
// w_i: weight for each user
// f_th_i: fidelity threshold --> diff for each users?

// x(i, k): whether user i is paired w/ RIS k
// 2D plan, I suppose
// R(i,k) = entangle_prob(i,k) * Rin,i
// max: sum sum xik * wi * entangle_prob(i,k) * Rin,i
// SA choose best --> check wi * entangle_prob(i,k) * Rin,i

// const double fid_parameter_beta = 0.0044;
// const double pro_parameter_alpha = 0.0002;

mt19937 gen(42);

/* --- SA --- */
int L = 5; // # of iterations
int m_k = 1; // # of users each RIS can serve
double T = 10;
double Tmin = 1;
double T_alpha = 0.95;

struct Vec{
    double x, y;
};

struct R_pos{
    double x, y;
};

Vec base;
vector<Vec> I_pos;
vector<Vec> Ris_pos;

struct Solution{ //sol_current; sol_ans; sol_opt
    // Vec pos;   // a.pos.x; a.pos.y
    vector<double> Rin;  // EGR for each user
    vector<double> fid;
    double WFI;
    double obj; // ours --> max sum xik * wi * entangle_prob(i,k) * Rin,i
    double Rin_left;
    vector<double> match; // since it is one-to-one, I guess I don't need to use x[i][k]?
    vector<int> user_left; // users not yet been served --> their ID
};

// double entangle_prob(double distance)
// {
    // return exp( -pro_parameter_alpha*distance );
// }

// double entangle_fid(double distance){
	// return 0.5 + 0.5*exp( -fid_parameter_beta*distance )/2 ;
// }

double calculate_WFI(Solution& sol)
{
    double nu, de;  // Numerator, Denominator
    // cout << "\n";
    for(int i=0; i<I; i++)
    {
        int k = sol.match[i];
        // if(k != -1)
            // cout << "user_" << i << ": RIS_" << k << "\n";
        // else
            // cout << "user_" << i << ": not served\n";
        double RE2E_i = prob_en[i][k]*sol.Rin[i];
        nu += RE2E_i;
        de += pow(RE2E_i, 2)/w[i];
    }
    double cal_WFI = pow(nu, 2)/de;
    // cout << "WFI: " << cal_WFI << "\n";
    return cal_WFI;
}

void checkpoint(Solution& sol, int k)
{
    // cout << "checkpoint\n";
    int user_left_num = sol.user_left.size();
    // cout << "USER LEFT NUM:" << user_left_num << "\n";
    int loop = user_left_num;
    int user_served_num = 0;

    for (int j = 0; j < loop; j++)
    {
        int i = sol.user_left[j];
        // cout << "user_" << i << " | n:" << n_pairs[i][k] << " rate:" << sol.Rin[i] << "\n";

        if(sol.Rin[i] != 0)
        {
            // cout << "ACCEPT\n\n";
            user_served_num++;
        }
        else // refuse --> remove from match --> set -1
        {
            // if(n_pairs[i][k]+1 > 1)
                // cout << "REJECT: FIDELITY TOO LOW\n";
            // if(sol.Rin[i] == 0)
                // cout << "REJECT: NO RATE\n";
            sol.match[i] = -1;
            sol.Rin[i] = 0;
            // cout << "\n";
        }
    }

    // rank performance --> pick best
    // cout << "user_served_num:" << user_served_num << "\n";
    int pick_user_id = -1;
    if(user_served_num != 0)
    {
        double max_obj = 0;
        for(int j=0; j<user_left_num; j++)
        {
            int i = sol.user_left[j];
            if(sol.match[i] == k)
            {
                double currernt_obj = w[i]*sol.Rin[i]*sol.Rin[i]/R_user_max[i];
                // cout << "user_" << i << " | performance(obj):" << currernt_obj << "\n";
                if(currernt_obj > max_obj)
                {
                    max_obj = currernt_obj;
                    pick_user_id = i;
                }
            }
        }
        // cout << "BEST: user_" << pick_user_id << " | performance:" << max_obj << "\n";
    }

    for(int j=0; j<user_left_num; j++)
    {
        int i = sol.user_left[j];
        if(i == pick_user_id)
        {
            // cout << "RIS_" << k << " serves: user_" << pick_user_id << "\n";
            sol.match[pick_user_id] = k;
            auto it = find(sol.user_left.begin(), sol.user_left.end(), i);
            sol.user_left.erase(it);
            user_left_num--;
        }
    }

    for(int j=0; j<user_left_num; j++)
    {
        int i = sol.user_left[j];
        if(i != pick_user_id) // remove
        {
            sol.Rin_left += sol.Rin[i]*prob_en[i][k];
            sol.match[i] = -1;
            sol.Rin[i] = 0;
        }
    }

    /*cout << "user left: ";
    for(int i=0; i<user_left_num; i++)
    {
        cout << sol.user_left[i] << " ";
    }
    cout << "\n";*/
}

void rate_distribution(Solution& sol, int k, Solution& old_sol, int initial)
{
    // cout << "\n\nRIS_" << k << ":" << endl;
    // cout << "\nrandom_rate_distribute!\n";

    // cout << sol.Rin_left << " rate left\n" << sol.user_left.size() << " users left\n";

    // uniform_int_distribution<> dis(0, 1); // decide whether to keep that answer
    // int bit = dis(gen);
    int bit = 1;

    for(int j=0; j<sol.user_left.size(); j++)
    {
        int i = sol.user_left[j];

        int can_serve = 0;  // check whether RIS k can serve user i
        for(int user : ris_served_user[k])
        {
            if (user == i)
            {
                can_serve = 1;
                break;
            }
        }

        if( sol.Rin_left == 0 || n_pairs[i][k] > 1 || can_serve == 0 )
        {
            sol.Rin[i] = 0;
        }
        else if( j == sol.user_left.size()-1 ) // last user take all that is left
        {
            sol.Rin[i] = min(sol.Rin_left*prob_en[i][k], R_user_max[i]);
        }
        else
        {
            uniform_int_distribution<> dis(0, min(sol.Rin_left*prob_en[i][k], R_user_max[i]));
            sol.Rin[i] = dis(gen);
        }
        sol.Rin_left -= old_sol.Rin[i]/prob_en[i][k];
        // cout << "user" << i << ": " << sol.Rin[i] << "  ";
    }
    // cout << "R_bs_rate left: "<< sol.Rin_left << "\n\n";
}

void random_pick_ris(int table[])
{
    // cout << "\nrandom_pick_ris!\n";

    vector<int> random_ris;
    for (int i = 0; i < K; i++)
    {
        random_ris.push_back(i);
    }

    shuffle(random_ris.begin(), random_ris.end(), gen);

    for(int i=0; i<K; i++)
    {
        table[i] = random_ris[i];
    }
}

double calculate_obj(Solution& sol)
{
    double obj = 0;
    for(int i=0; i<I; i++)
    {
        int k = sol.match[i];

        if(sol.Rin[i] != 0)
            obj = obj + w[i]*sol.Rin[i]*sol.Rin[i]/R_user_max[i];
            // obj = obj + w[i]*E2E_rate(i, sol.Rin[i], Ris_pos[k].x, Ris_pos[k].y);
        // cout << "E2E_rate: " << E2E_rate(i, sol.Rin[i], Ris_pos[k].x, Ris_pos[k].y) << " | weight: " << w[i] << "\n";
    }
    return obj;
}

void SA()
{
    int ris_table[K]; // pick RIS sequence

    // initialize the current sol
    Solution sol_current;
    sol_current.Rin_left = R_bs_max;
    for(int i=0; i<I; i++)
    {
        sol_current.Rin.push_back(0);
    }
    for(int i=0; i<I; i++)
    {
        sol_current.user_left.push_back(i);
    }

    for(int i=0; i<I; i++)
    {
        sol_current.match.push_back(-1);
    }
    random_pick_ris(ris_table);
    /*cout << "RIS sequence: ";
    for(int i=0; i<K; i++)
    {
        cout << ris_table[i] << " ";
    }
    cout << endl;*/

    for(int k=0; k<K; k++) // k: index for RIS
    {
        if(sol_current.user_left.size() == 0)
        {
            // cout << "all served\n";
            break;
        }

        // update match --> all unserved users assign to RIS[k]
        for(int j=0; j<sol_current.user_left.size(); j++)
        {
            int i = sol_current.user_left[j];
            sol_current.match[i] = ris_table[k];
        }
        rate_distribution(sol_current, ris_table[k], sol_current, 1);
        checkpoint(sol_current, ris_table[k]);
    }
    sol_current.WFI = calculate_WFI(sol_current);

    // initialize the optimal sol to current sol
    Solution sol_opt = sol_current;

    /*--------------------------*/

    // cout << "------------------\nDONE INITIALIZING\n------------------\n\n";

    // cout << T << " " << Tmin << "\n";
    while(T > Tmin)
    {
        for(int l=0; l<L; l++)
        {
            // cout << "current T:" << T;
            // initialize the new sol
            Solution sol_new;

            sol_new.Rin_left = R_bs_max;
            for(int i=0; i<I; i++)
            {
                sol_new.Rin.push_back(0);
            }
            for(int i=0; i<I; i++)
            {
                sol_new.user_left.push_back(i);
            }

            for(int i=0; i<I; i++)
            {
                sol_new.match.push_back(-1);
            }
            random_pick_ris(ris_table);
            /*cout << "RIS sequence: ";
            for(int i=0; i<K; i++)
            {
                cout << ris_table[i] << " ";
            }
            cout << endl;*/

            for(int k=0; k<K; k++) // k: index for RIS
            {
                if(sol_new.user_left.size() == 0)
                    break;
                // update match --> all unserved users assign to RIS[k]
                for(int j=0; j<sol_new.user_left.size(); j++)
                {
                    int i = sol_new.user_left[j];
                    sol_new.match[i] = ris_table[k]; // k;
                }
                rate_distribution(sol_new, ris_table[k], sol_current, 0); // random_rate_distribute(sol_new, ris_table[k]);
                checkpoint(sol_new, ris_table[k]); // check_fidelity_capaticy(sol_new, ris_table[k]);
            }

            // calculate WFI & WFI_diff
            sol_new.WFI = calculate_WFI(sol_new);
            double delta_U = sol_new.WFI - sol_current.WFI;

            // update current ans if WFI_diff > 0
            if(delta_U  > 0) // better answer --> update sol_current
            {
                sol_current = sol_new;
            }
            else
            {
                // Generate a random number r: 0~1
                uniform_real_distribution<double> dist(0.0, 1.0);
                double r = dist(gen);
                if( exp(delta_U/T) > r ) // if r > 0, update sol_ans as well
                {
                    sol_current = sol_new;
                }
            }

            // if WFI_current > WFI_opt --> update opt ans
            if(sol_current.WFI > sol_opt.WFI)
            {
                sol_opt = sol_current;
            }
            // cout << "------------------\nEND OF ONE ITERATION\n------------------\n\n";
        }
        T = T*T_alpha;
    }

    // cout << "------------------\nRESULT\n------------------\n";
    for(int i=0; i<I; i++)
    {
        if(sol_opt.match[i] != -1)
        {
            accept_assign.push_back({i,sol_opt.match[i]});
            int k = sol_opt.match[i];
            R_user[i] = sol_opt.Rin[i];
            // R_user[i][k] = sol_opt.Rin[i];
            // cout << "user_" << i << ": RIS_" << sol_opt.match[i] << "\n";
        }
        // else
            // cout << "user_" << i << ": not served\n";
    }
    // cout << "WFI: " << sol_opt.WFI << "\n";
    // cout << "obj: " << calculate_obj(sol_opt) << "\n";
    // cout << "------------------\n\n";
}

/* --- output result --- */
void output_accept(){
    ofstream out(outfile);
    if(!out.is_open()){
        cout << "Error: Cannot open file data/output/res_greedy_w.txt" << endl;
        exit(1);
    }
    out << "Accepted assignment: " << endl;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        out << "User " << i << " is assigned to RIS " << k << endl;
    }
    out << "Total number of accepted assignment: " << accept_assign.size() << endl;
    double obj = 0;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        obj += w[i] * R_user_max[i];
    }
    out << "Objective value: " << obj << endl;
    double total_power = 0;
    for(auto it = accept_assign.begin(); it != accept_assign.end(); it++){
        auto [i, k] = *it;
        total_power += R_user[i] * (n_pairs[i][k]) / (prob_en[i][k]); // need modification
       //  total_power += R_user_max[i] * (n_pairs[i][k]+1) / (prob_en[i][k] * prob_pur[i][k]);
    }
    out << "Total power usage: " << total_power << endl;
}



/* --- input from dataset --- */
void input_dataset(){
    ifstream in(infile);
    if(!in.is_open()){
        cout << "Error: Cannot open file " << dataset_file << endl;
        exit(1);
    }
    in >> I >> K;
    in >> R_bs_max;
    R_user_max.resize(I);
    w.resize(I);
    prob_en.resize(I, vector<double>(K));
    prob_pur.resize(I, vector<double>(K));
    n_pairs.resize(I, vector<double>(K));

    for(int i = 0; i < I; i++){ in >> w[i] >> R_user_max[i]; }
    for(int i = 0; i < I; i++){
        for(int j = 0; j < K; j++){
            // assumtion: n_pairs[i][j] > 0, prob_pur[i][j] = 1 when we not doing purification
            in >> prob_en[i][j] >> prob_pur[i][j] >> n_pairs[i][j];
        }
    }
    for(int k = 0; k < K; k++){
        int num_served;
        in >> num_served;
        ris_served_user.push_back(vector<int>(num_served));
        for(int i = 0; i < num_served; i++){
            in >> ris_served_user[k][i];
            can_serve[{k, ris_served_user[k][i]}] = 1;
        }
    }
    in.close();
}

int main()
{
    if(argc != 3){
        cout << "Usage: ./greedy_cp <infile> <outfile>" << endl;
        exit(1);
    }
    infile = argv[1];
    outfile = argv[2];
    input_dataset();
    SA();
    output_accept();
    return 0;
}
