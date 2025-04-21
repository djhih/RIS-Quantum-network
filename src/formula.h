#include <bits/stdc++.h>
#include <cstdlib>
#include <random>
using namespace std;

double entangle_fidelity(double dis, double beta) {
  return 0.5 + 0.5 * exp(-beta * dis);
}

double swapping_fidelity(double fid1, double fid2) { return fid1 * fid2; }

double purify_fidelity(double fid1, double fid2) {
  return (fid1 * fid2) / ((fid1 * fid2) + ((1 - fid1) * (1 - fid2)));
}

double entangle_success_prob(double dis) { return exp(-0.0002 * dis); }

double purify_success_prob(double fid1, double fid2) {
  return ((fid1 * fid2) + ((1 - fid1) * (1 - fid2)));
}

double ln(double x) { return log(x) / log(exp(1)); }