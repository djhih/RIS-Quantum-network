#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <algorithm> // use shuffle for randomization?
#include <random>
using namespace std;

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

const double fid_parameter_beta = 0.0044;
const double pro_parameter_alpha = 0.0002;
mt19937 gen(42);

int L = 5; // # of iterations
int I;
int K;
int m_k; // # of users each RIS can serve
int Rin_bs = 100; // initial rate --> double? int?
double w[1000];
double f_th[1000];
double T = 5;
double Tmin = 1;
double T_alpha = 0.6;

struct Vec{
    double x, y;
};

struct R_pos{
    double x, y;
};

struct User{
    int id;
    double performance;
};

Vec base;
vector<Vec> I_pos;
vector<Vec> Ris_pos;
vector<Vec> capacity; // RIS can serve how many users, don't don't whether it is unify or not
int Ris_capacity; // in case it is unify

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

double entangle_prob(double distance)
{
    return exp( -pro_parameter_alpha*distance );
}

double entangle_fid(double distance){
	return 0.5 + 0.5*exp( -fid_parameter_beta*distance )/2 ;
}

double E2E_rate(int i, double Rin_i, double Rpos_x, double Rpos_y)    // RE2E,i --> our R(i,k)
{
    // i: user id
    double distance = sqrt( pow(Rpos_x - base.x, 2) + pow(base.y - I_pos[i].y, 2) )
    + sqrt( pow(Rpos_x - I_pos[i].x, 2) + pow(Rpos_y - I_pos[i].y, 2) );   // dist between base & RIS_k + between RIS_k & user_i
    double p_succ = entangle_prob(distance) * Rin_i;
    return p_succ;
}

double calculate_WFI(Solution& sol)//(vector<double>& Rin, int Rpos_x, int Rpos_y)
{
    double nu, de;  // Numerator, Denominator
    for(int i=0; i<I; i++)
    {
        int k = sol.match[i];
        if(k != -1)
            cout << "user_" << i << " is served by RIS_" << k << "\n";
        else
            cout << "user_" << i << " is not served\n";
        double RE2E_i = E2E_rate(i, sol.Rin[i], Ris_pos[k].x, Ris_pos[k].y);
        nu += RE2E_i;
        de += pow(RE2E_i, 2)/w[i];
    }
    double cal_WFI = pow(nu, 2)/de;
    return cal_WFI;
}

void check_fidelity_capaticy(Solution& sol, int k)
{
    int user_left_num = sol.user_left.size();
    cout << "USER LEFT NUM:" << user_left_num << "\n";
    int loop = user_left_num;
    for (int j = 0; j < loop; j++)
    {
        int i = sol.user_left[j];
        double distance = sqrt( pow(base.x - Ris_pos[k].x, 2) + pow(base.y - Ris_pos[k].y, 2) )
        + sqrt( pow(Ris_pos[k].x - I_pos[i].x, 2) + pow(Ris_pos[k].y - I_pos[i].y, 2) );
        double fidelity = entangle_fid(distance);
        cout << "user_" << i << " | rate:" <<  sol.Rin[i] << " dist:" << distance << " fid:" << fidelity << "\n";

        if(fidelity >= f_th[i] && sol.Rin[i] != 0) // accept --> later have to 1) remove from unserved list, 2) subtract rate, 3) update match
        {
            // cout << "ACCEPT\n\n";
        }
        else // refuse --> remove from match --> set -1
        {
            // cout << "REJECT\n\n";
            sol.match[i] = -1;
            sol.Rin[i] = 0;
            user_left_num--;
        }
    }

    cout << "num of users served by this RIS:" << user_left_num << "\n";
    if(user_left_num > m_k) // exceed capacity --> remove users that performs poorly
    {
        cout << "EXCEED CAPACITY\n";
        vector<User> users;
        for(int j=0; j<user_left_num; j++)
        {
            int i = sol.user_left[j];
            User input;
            input.id = i;
            input.performance = w[i]*E2E_rate(i, sol.Rin[i], Ris_pos[k].x, Ris_pos[k].y); // wi * entangle_prob(i,k) * Rin,i
            users.push_back(input);
            // cout << "user_" << i << " | performance:" << users[i].performance << "\n";
        }
        sort(users.begin(), users.end(), [](const User& a, const User& b) {return a.performance > b.performance;});

        for(int i=0; i<user_left_num; i++)
            cout << "user_" << users[i].id << " | performance:" << users[i].performance << "\n";

        cout << "remove:";
        for(int j=0; j<user_left_num; j++)  // pick the best ones --> 1) remove from unserved list, 2) subtract rate, 3) update match
        {
            int i = users[j].id;
            if(j >= m_k) // remove
            {
                sol.match[i] = -1;
                sol.Rin[i] = 0;
            }
            else
            {
                // cout << "user_" << i << " is served!\n";
                sol.Rin_left -= sol.Rin[i];
                auto it = find(sol.user_left.begin(), sol.user_left.end(), i);
                sol.user_left.erase(it);
            }
        }
    }
    else
    {
        for(int j=0; j<user_left_num; j++)
        {
            int i = sol.user_left[j];
            // cout << "user_" << i << " is served!\n";
            sol.Rin_left -= sol.Rin[i];
            auto it = find(sol.user_left.begin(), sol.user_left.end(), i);
            sol.user_left.erase(it);
        }
    }

    /* cout << "updated match: ";
    for(int i=0; i<I; i++)
        cout << sol.match[i] << " ";
    cout << "\n"; */
}

void random_rate_distribute(Solution& sol, int k)
{
    cout << "\nrandom_rate_distribute!\n";
    cout << "RIS_" << k << ":" << endl;

    cout << sol.Rin_left << " rate left\n" << sol.user_left.size() << " users left\n";

    uniform_int_distribution<> dis(0, sol.Rin_left);

    vector<int> energy_cuts;
    for (int i = 0; i < sol.user_left.size() - 1; i++)
    {
        energy_cuts.push_back(dis(gen));
    }

    sort(energy_cuts.begin(), energy_cuts.end());

    energy_cuts.insert(energy_cuts.begin(), 0);
    energy_cuts.push_back(sol.Rin_left);


    int user_left_num = sol.user_left.size();
    vector<int> energy(user_left_num);
    for (int i = 0; i < user_left_num; i++)
    {
        energy[i] = energy_cuts[i + 1] - energy_cuts[i];
        sol.Rin[sol.user_left[i]] = energy[i];
    }

    /* for (int i = 0; i < user_left_num; i++)
    {
        // cout << energy[i] << " ";
        // sol.Rin[sol.user_left[i]] = energy[i];
        cout << "user" << sol.user_left[i] << ": " << sol.Rin[sol.user_left[i]] << "  ";
    }
    cout << endl;*/
}

void random_pick_ris(int table[])
{
    cout << "\nrandom_pick_ris!\n";

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
            obj = obj + w[i]*E2E_rate(i, sol.Rin[i], Ris_pos[k].x, Ris_pos[k].y);
        // cout << "E2E_rate: " << E2E_rate(i, sol.Rin[i], Ris_pos[k].x, Ris_pos[k].y) << " | weight: " << w[i] << "\n";
    }
    return obj;
}

void SA()
{
    int ris_table[K]; // pick RIS sequence

    // initialize the current sol
    Solution sol_current;
    sol_current.Rin_left = Rin_bs;
    for(int i=0; i<I; i++)
    {
        sol_current.Rin.push_back(0);
    }
    for(int i=0; i<I; i++)
    {
        sol_current.user_left.push_back(i);
    }
    /*cout << "\nwho's left? ";
    for(int i=0; i<I; i++)
    {
        cout << sol_current.user_left[i] << " ";
    }*/

    // int user_left_num = sol_current.user_left.size();
    // cout << "\nHow many users left? " << sol_current.user_left.size();

    for(int i=0; i<I; i++)
    {
        sol_current.match.push_back(-1);
    }
    random_pick_ris(ris_table);
    cout << "RIS sequence: ";
    for(int i=0; i<K; i++)
    {
        cout << ris_table[i] << " ";
    }
    cout << endl;

    for(int k=0; k<K; k++) // k: index for RIS
    {
        if(sol_current.user_left.size() == 0)
            break;
        // update match --> all unserved users assign to RIS[k]
        for(int j=0; j<sol_current.user_left.size(); j++)
        {
            int i = sol_current.user_left[j];
            sol_current.match[i] = k;
        }
        random_rate_distribute(sol_current, ris_table[k]);
        check_fidelity_capaticy(sol_current, k);
    }
    sol_current.WFI = calculate_WFI(sol_current);
    cout << "current sol WFI:" << sol_current.WFI << "\n\n";

    // initialize the optimal sol to current sol
    Solution sol_opt = sol_current;

    /*--------------------------*/

    cout << "------------------\nDONE INITIALIZING\n------------------\n\n";

    // cout << T << " " << Tmin << "\n";
    while(T > Tmin)
    {
        for(int l=0; l<L; l++)
        {
            // cout << "current T:" << T;
            // initialize the new sol
            Solution sol_new;

            sol_new.Rin_left = Rin_bs;
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
            cout << "RIS sequence: ";
            for(int i=0; i<K; i++)
            {
                cout << ris_table[i] << " ";
            }
            cout << endl;

            for(int k=0; k<K; k++) // k: index for RIS
            {
                if(sol_new.user_left.size() == 0)
                    break;
                // update match --> all unserved users assign to RIS[k]
                for(int j=0; j<sol_new.user_left.size(); j++)
                {
                    int i = sol_new.user_left[j];
                    sol_new.match[i] = k;
                }
                random_rate_distribute(sol_new, ris_table[k]);
                check_fidelity_capaticy(sol_new, k);
            }

            // calculate WFI & WFI_diff
            sol_new.WFI = calculate_WFI(sol_new);
            // cout << "new sol WFI:" << sol_new.WFI << "\n\n";
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
                // cout << "r:" << r << "\n";
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
            cout << "------------------\nEND OF ONE ITERATION\n------------------\n\n";
        }
        T = T*T_alpha;
    }

    cout << "------------------\nRESULT\n------------------\n\n";
    for(int i=0; i<I; i++)
    {
        if(sol_opt.match[i] != -1)
            cout << "user_" << i << ": RIS_" << sol_opt.match[i] << "\n";
        else
            cout << "user_" << i << ": not served\n";
    }
    cout << "WFI: " << sol_opt.WFI << "\n";
    cout << "obj: " << calculate_obj(sol_opt) << "\n";
}


int main()
{
    /* test input start */
    K = 3, I = 6, m_k = 2;
    base.x = 0;
    base.y = 0;

    Vec pos;
    pos.x = 0;
    pos.y = 10;
    Ris_pos.push_back(pos);
    pos.x = 50;
    pos.y = 3;
    Ris_pos.push_back(pos);
    pos.x = 78;
    pos.y = 44;
    Ris_pos.push_back(pos);

    pos.x = 7;
    pos.y = 90;
    I_pos.push_back(pos);
    pos.x = 0;
    pos.y = 99;
    I_pos.push_back(pos);
    pos.x = 30;
    pos.y = 45;
    I_pos.push_back(pos);
    pos.x = 2;
    pos.y = 2;
    I_pos.push_back(pos);
    pos.x = 5;
    pos.y = 89;
    I_pos.push_back(pos);
    pos.x = 35;
    pos.y = 6;
    I_pos.push_back(pos);

    for (int i = 0; i < I; i++)
    {
        //cin >> f_th[i];
        f_th[i] = 0.65;
    }
    for (int i = 0; i < I; i++)
    {
        //cin >> w[i];
        w[i] = 0.5;
    }
    /* test input end */

    for (int i = 0; i < K; i++)
    {
        cout << "RIS_" << i << ":(" << Ris_pos[i].x << "," << Ris_pos[i].y << ")\n";
    }
    for (int i = 0; i < I; i++)
    {
        cout << "user_" << i << ":(" << I_pos[i].x << "," << I_pos[i].y << ")\n";
    }

    SA();
}
