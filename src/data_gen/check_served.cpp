#include<bits/stdc++.h>
#include<random>

using namespace std;

/* This code is used to check if the user can be served by the RIS
    1. I: # of users
    2. K: # of RISs
    3. loc_user[|I|]: location of user i
    4. loc_ris[|K|]: location of RIS k
    5. dis[i][k]: distance between user i and RIS k
    6. dis_bs[k]: distance between RIS k and BS
    7. dis_tot[i][k]: distance between user i and BS through RIS k
*/

struct position{
    pair<double, double>loc;
    pair<double, double>vec;
    double x(){ return loc.ff;}
    double y(){ return loc.ss;}
    void upd(double x, double y){
        loc.ff = x;
        loc.ss = y;
    }
} user[10], bs;

struct RIS {
    pair<double, double>loc;
    pair<double, double>horizontal;
    pair<double, double>vertical;
    double theta;
    double x(){ return loc.ff;}
    double y(){ return loc.ss;}
    void upd(double x, double y){
        loc.ff = x;
        loc.ss = y;
    }
} ris[10];

double dot(pair<double, double> a, pair<double, double> b){
    return a.ff * b.ff + a.ss * b.ss;
}

double get_angle(pair<double, double> a, pair<double, double> b){
    double angle = acos(dot(a, b) / (sqrt(a.ff * a.ff + a.ss * a.ss) * sqrt(b.ff * b.ff + b.ss * b.ss)));
    return angle;
}

int I, K;

int main(){
    cin >> I >> K;
    // assume the BS is at (0, 0)
    bs.loc = {0, 0};
    bs.vec = {0, 0};

    // gen random ris position
    for(int i = 0; i < K; i++){
        // gen random theta
        // ris[i].theta = (double)(rand() % 360) * M_PI / 180.0;
        double x, y, theta;
        cin >> x >> y >> theta;
        ris[i].upd(x, y);
        ris[i].theta = (double) theta * M_PI / 180.0;

        double xx = ris[i].y() * (1.0 / tan(ris[i].theta));
        xx += x;
        cout << "(xx, 0): " << xx << '\n';

        ris[i].horizontal = {xx - x, -y};
        ris[i].vertical = {y, xx - x}; // rotate hor。izontal vector 90 degree
        // ris[i].vertical = {-y, x-xx};

        cout << "horizontal: " << ris[i].horizontal.ff << ' ' << ris[i].horizontal.ss << endl;
        cout << "vertical: " << ris[i].vertical.ff << ' ' << ris[i].vertical.ss << endl;
    }

    for(int i=0; i<I; i++){
        double x, y;
        cin >> x >> y;
        user[i].upd(x, y);
    }

    for(int k=0; k<K; k++){
        // count ris_k can server which user
        pair<double, double>bs_ris = {bs.x() - ris[k].x(), bs.y() - ris[k].y()};
        double angle_bs = get_angle(bs_ris, ris[k].vertical);
        for(int i=0; i<I; i++){
            // check if user[i] and bs are on the same side of the RIS
            pair<double, double> u_ris = {user[i].x() - ris[k].x(), user[i].y() - ris[k].y()};
                
            
            double angle_user = get_angle(u_ris, ris[k].vertical);
            cout << "angle_user: " << angle_user * 180.0 / M_PI << endl;
            cout << "angle_bs: " << angle_bs * 180.0 / M_PI << endl;
            cout << "u_ris: " << u_ris.ff << ' ' << u_ris.ss << endl;
            cout << "bs_ris: " << bs_ris.ff << ' ' << bs_ris.ss << endl;
            if(angle_user < (M_PI / 2.0) && angle_bs < (M_PI / 2.0)){
                cout << "User " << i << " at " << user[i].x() << ' ' << user[i].y() << " can be served by RIS " << k << endl;
            } else {
                // cout << "User " << i << " cannot be served by RIS " << k << endl;
            }
            cout << '\n';
        }
    }
}