#include<iostream>
#include<fstream>
#include<random>
#include<ctime>
#include<cmath>
#include<limits>
#include"../formula.h"
#include <stdexcept>
#define ff first
#define ss second
using namespace std;

/* This code is used to generate test cases, containing the following data:
    1. I: # of users
    2. K: # of RISs
    3. R_bs_max: max rate of BS
    4. R_user_max[|I|]: max rate of users
    5. w[i]: weight of user i
    6. fid[|I|][|K|]: fidelity of user i to RIS k
    7. prob_en[|I|][|K|]: probability of user i create entanglement with RIS k
    8. prob_pur[|I|][|K|]: probability of user i purify entanglement with RIS k
    9. n[|I|][|K|]: # of entanglement pairs for user i purify with RIS k
    10. loc_x[|I|]: x coordinate of user i
    11. loc_y[|I|]: y coordinate of user i
    12. loc_x[|K|]: x coordinate of RIS k
    13. loc_y[|K|]: y coordinate of RIS k
    14. dis[|I|][|K|]: dis(i, k)
    15. dis_bs[|k|]: dis(k, bs)
    16. dis_tot[|I|][|K|]: dis(i, k) + dis(k, bs)

output: 
    1. I
    2. K
    3. R_bs_max
    4. R_user_max[|I|]
    5. w[i]
    6. prob_en[|I|][|K|]
    7. prob_pur[|I|][|K|]
    8. n[|I|][|K|]

constraints:
    1. dis(user, RIS) <= 20
    2. uesr can be served by RIS while dis(user, RIS) <= 20 and angle(user-RIS, bs-RIS) <= 180
    3. all user can be served by at least one RIS
    4. all RIS can serve at least one user

steps:
    1. argv contains the number of users, RISs, BS power, fidelity threshold, avg load
    2. generate random RIS locations and RIS theta
    3. generate random user locations, and check if they can be served by at least one RIS
    4. generate random user max rate, weight, and distance to RIS

Default setting: 
    1. RIS 10
    2. User 100
    3. Random User, RIS Location
    4. User-RIS 20
    5. weight 1-5 float

Plot:
    X Label: # Users, # RISs, dis(user, ris), BS power, Fidelity threshold, 平均負載(RIS 能服務的 user 數量)
    Y Label : obj, # Users

Note:
    1. n pair = 1 means 1 pair of entanglement and use 0 pair to purify
*/

int I = 100; // number of users
int K = 10; // number of RISs
const double beta = 0.00438471;
double R_bs_max = 100; // max rate of BS
double fidelity_threshold = 0.85;

struct purify_table {
    double dis;
    double fid_en;
    double prob_en;
    double prob_pur;
    vector<double> fid_pur_times;

    purify_table() : dis(0), fid_en(0), prob_en(1), prob_pur(1), fid_pur_times(vector<double>()) {}
    purify_table(double dis, double fid_en, double prob_en, double prob_pur, double prob_cur = 1)
        : dis(dis), fid_en(fid_en), prob_en(prob_en), prob_pur(prob_pur), fid_pur_times(vector<double>()) {}
};

class Position {
public:
    Position() : loc(0.0, 0.0) {}
    Position(double x, double y) : loc(x, y) {}

    double x() const { return loc.first; }
    double y() const { return loc.second; }
    void update(double x, double y) {
        loc.first = x;
        loc.second = y;
    }

protected:
    pair<double, double> loc;
};

class RIS : public Position {
public:
    RIS() : Position(), theta(0.0), dis_bs(0.0) {}
    RIS(double x, double y, double t) : Position(x, y), theta(t), dis_bs(0.0) {}

    double theta;
    double dis_bs;
    double angle_bs;
    double angle_user;
    vector<double> n_pairs;
    vector<double> dis_user;
    vector<int> served_user;
    pair<double, double> horizontal;
    pair<double, double> vertical;
};

class User : public Position {
public:
    User() : Position(), R_user_max(0.0), w(0.0) {}
    User(double x, double y, double rate, double weight) : Position(x, y), R_user_max(rate), w(weight) {}

    double R_user_max;
    double w;
    vector<double> n_pairs;
    vector<double> dis_ris;
};

double dot(pair<double, double> a, pair<double, double> b){
    return a.ff * b.ff + a.ss * b.ss;
}

double get_angle(pair<double, double> a, pair<double, double> b){
    double angle = acos(dot(a, b) / (sqrt(a.ff * a.ff + a.ss * a.ss) * sqrt(b.ff * b.ff + b.ss * b.ss)));
    return angle;
}

bool check_served(double angle_user, double angle_bs, double dis){
    return (angle_user < (M_PI / 2.0) && angle_bs < (M_PI / 2.0) && dis <= 20);
} 

double generate_price_ratio(std::default_random_engine& rng) {
    std::normal_distribution<> dist(1.0, 1.5); // 平均值1.0，標準差1.5
    double val;
    do {
        val = dist(rng);
    } while (val < 1.0 || val > 3.0); // 只取右半部在 [1, 3]
    return val;
}

int required_purification_rounds(double F0, double F_th) {
    if (F0 <= 0.0 || F0 >= 1.0 || F_th <= 0.0 || F_th >= 1.0) {
        throw std::invalid_argument("F0 and F_th must be in the open interval (0, 1).");
    }

    double log_F0_ratio = std::log(F0 / (1.0 - F0));
    double log_Fth_ratio = std::log(F_th / (1.0 - F_th));

    if (log_F0_ratio <= 0.0) {
        throw std::runtime_error("F0 is too low to reach the threshold through purification.");
    }

    // Compute the minimum n satisfying the inequality
    double n_real = log_Fth_ratio / log_F0_ratio - 1.0;

    // Round up to next integer, since n must be an integer
    int n = static_cast<int>(std::ceil(n_real));
    return n;
}


int main(int argc, char *argv[]){
    if(argc != 8){
        cout << "Usage: ./gen <datasetfile> <I> <K> <R_bs_max> <fidelity_threshold> <avg_load> <seed>" << endl;
        exit(1);
    }
    string dataset_file = argv[1];
    I = atoi(argv[2]);
    K = atoi(argv[3]);
    R_bs_max = atof(argv[4]);
    fidelity_threshold = atof(argv[5]);
    double avg_load = atof(argv[6]);
    int seed = atoi(argv[7]);

    ofstream out(dataset_file);
    if(!out.is_open()){
        cout << "Error: Cannot open file raw/dataset.txt" << endl;
        exit(1);
    }
    
    random_device rd;
    // default_random_engine gen(rd());
    default_random_engine gen(seed);   // fixed seed for reproducibility
    uniform_int_distribution<> dist_int(0, 100);
    uniform_real_distribution<> dist_real(0, 100);
    uniform_int_distribution<> dist_int2(0, 600);

    vector<User> users(I);
    vector<RIS> riss(K);
    vector<vector<purify_table>> data_i_k(I, vector<purify_table>(K));

    // Generate BS location
    Position bs(0, 0);

    out << I << " " << K << " " << R_bs_max << endl;
    set<pair<double, double>> user_set;
    set<pair<double, double>> ris_set;
    
    // Generate RIS locations and angles
    for(int k = 0; k < K; k++){
        int x = dist_int2(gen), y = dist_int2(gen);
        do {
            uniform_real_distribution<> angle_dist(0, 2 * M_PI);
            double theta = angle_dist(gen);

            // Use uniform distribution for x and y coordinates
            x = dist_int2(gen);
            y = dist_int2(gen);
            riss[k] = RIS(x, y, theta);
            
            double xx = riss[k].y() * (1.0 / tan(riss[k].theta));
            xx += x;
            riss[k].horizontal = {xx - x, -y};
            riss[k].vertical = {y, xx - x};

            pair<double, double>bs_ris = {bs.x() - riss[k].x(), bs.y() - riss[k].y()};
            riss[k].angle_bs = get_angle(bs_ris, riss[k].vertical);
        } while((riss[k].angle_bs >= (M_PI / 2.0)) || (x == 0 && y == 0) || ris_set.count({x, y}) > 0);
        
        // Calculate distance from RIS to BS
        riss[k].dis_bs = sqrt(pow(riss[k].x(), 2) + pow(riss[k].y(), 2));
        riss[k].dis_user.resize(I);
        ris_set.insert({x, y});
    }
    
    // Generate user locations
    for(int i = 0; i < I; i++){
        users[i].dis_ris.resize(K);
        users[i].n_pairs.resize(K);
        
        int x = dist_int2(gen), y = dist_int2(gen);
        bool served = false;

        while((x == 0 && y == 0) || !served || user_set.count({x, y}) > 0){
            for(int k = 0; k < K; k++){
                pair<double, double> u_ris = {x - riss[k].x(), y - riss[k].y()};
                double dis_user_risk = sqrt(pow(x - riss[k].x(), 2) + pow(y - riss[k].y(), 2));
                double angle_user = get_angle(u_ris, riss[k].vertical);
                if(check_served(angle_user, riss[k].angle_bs, dis_user_risk)){
                    served = true;
                    // cout << "User " << i << " is served by RIS " << k << " dis= " << dis_user_risk << endl;
                    // cout << "User location: (" << x << ", " << y << ")" << endl;
                    // cout << "RIS location: (" << riss[k].x() << ", " << riss[k].y() << ")" << endl;
                    // cout << "Angle: " << angle_user << endl;
                    // cout << "Distance: " << dis << endl;
                    break;
                }
            }
            if(served) break;
            x = dist_int2(gen);
            y = dist_int2(gen);
        }
        users[i].update(x, y);
        user_set.insert({x, y});

        for(int k = 0; k < K; k++){
            double distance = sqrt(pow(users[i].x() - riss[k].x(), 2) + pow(users[i].y() - riss[k].y(), 2));
            users[i].dis_ris[k] = distance;
            riss[k].dis_user[i] = distance;
            data_i_k[i][k] = purify_table(distance, 1, 1, 1);
        }
    }

    
    
    // Generate entanglement data
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){ 
            double distance = users[i].dis_ris[k];
            data_i_k[i][k].fid_en = entangle_fidelity(distance, beta);
            data_i_k[i][k].prob_en = entangle_success_prob(distance);
            // cout << "fid_en: " << data_i_k[i][k].fid_en << " prob_en: " << data_i_k[i][k].prob_en << endl;
            
            // Initialize first value as entanglement fidelity
            data_i_k[i][k].fid_pur_times.push_back(data_i_k[i][k].fid_en);
            
            // Try purifying until fidelity exceeds threshold
            for(int t = 0; t < 30; t++){
                if(data_i_k[i][k].fid_pur_times.back() >= fidelity_threshold){
                    users[i].n_pairs[k] = t + 1; // ! we set n_pairs[k] = t + 1
                    break;
                }
                double purify_fid = purify_fidelity(data_i_k[i][k].fid_pur_times.back(), data_i_k[i][k].fid_en);  
                data_i_k[i][k].prob_pur *= data_i_k[i][k].prob_en;
                data_i_k[i][k].prob_pur *= purify_success_prob(data_i_k[i][k].fid_pur_times.back(), data_i_k[i][k].fid_en);
                data_i_k[i][k].fid_pur_times.push_back(purify_fid);
            }
        }
    }
    
    // Generate random max rates and weights for users
    for(int i = 0; i < I; i++){
        double user_bs_dis = sqrt(pow(users[i].x(), 2) + pow(users[i].y(), 2));
        double user_en_fid = entangle_fidelity(user_bs_dis, beta);
        double user_bs_purification_times = required_purification_rounds(user_en_fid, fidelity_threshold);

        // mean 2.0, standard deviation 0.5
        uniform_int_distribution<> user_max_dist(1e4, 2e4);
        users[i].w = generate_price_ratio(gen) * user_bs_purification_times;
        users[i].R_user_max = user_max_dist(gen);
        out << users[i].w << ' ' << users[i].R_user_max << "\n";
    }

    // Output entanglement probability data
    for(int i = 0; i < I; i++){
        for(int k = 0; k < K; k++){
            if(data_i_k[i][k].fid_en > fidelity_threshold && data_i_k[i][k].prob_pur == 0){
                data_i_k[i][k].prob_pur = 1; // ! if not purified, set prob_pur to 1
            }
            out << data_i_k[i][k].prob_en << " " << data_i_k[i][k].prob_pur << " " << users[i].n_pairs[k] << "\n";
        }
    }

    for(int k=0; k<K; k++){
        for(int i=0; i<I; i++){
            // get angle between user and RIS
            pair<double, double> u_ris = {users[i].x() - riss[k].x(), users[i].y() - riss[k].y()};
            double angle_user = get_angle(u_ris, riss[k].vertical);
            if(check_served(angle_user, riss[k].angle_bs, users[i].dis_ris[k])){
                riss[k].served_user.push_back(i);
                // cout << "User " << i << " is served by RIS " << k << " dis= " << users[i].dis_ris[k] << endl;
            } 
        }
    }

    // output user served by RIS
    for(int k=0; k<K; k++){
        out << riss[k].served_user.size() << " ";
        for(int i=0; i<riss[k].served_user.size(); i++){
            out << riss[k].served_user[i] << " ";
        }
        out << "\n";
    }

    out.close();

    // print all served user
    // for(int k=0; k<K; k++){
    //     cout << "RIS " << k << " can serve user: ";
    //     for(int i=0; i<riss[k].served_user.size(); i++){
    //         cout << riss[k].served_user[i] << " ";
    //     }
    //     cout << endl;
    // }

    // output location
    string loc_file = "data/output/loc_" + dataset_file;
    ofstream out_loc(loc_file);
    if(!out_loc.is_open()){
        cout << "Error: Cannot open file " << loc_file << endl;
        exit(1);
    }
    for(int i=0; i<I; i++){
        out_loc << "user " << users[i].x() << " " << users[i].y() << "\n";
    }
    out << "\n\n";
    for(int k=0; k<K; k++){
        out_loc << "ris " << riss[k].x() << " " << riss[k].y() << "\n";
    }
    return 0;
}