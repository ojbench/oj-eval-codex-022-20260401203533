#include <bits/stdc++.h>
using namespace std;

// Interactive oracle provided by the judge
int query(int x, int y, int z);

namespace GuessInternal {
    const int MOD = 998244353;
    const int BASE = 233;

    // Safe helper to compute q(x,y,k)
    inline int q3(int x, int y, int k) {
        return query(x, y, k);
    }

    // Find indices of global min and max values using two-phase selection.
    pair<int,int> find_extremes(int n) {
        auto verify = [&](int a, int b) -> bool {
            int first_k = -1;
            for (int i = 1; i <= n; ++i) if (i != a && i != b) { first_k = i; break; }
            if (first_k == -1) return true;
            int S = q3(a, b, first_k);
            int checks = 0;
            for (int i = 1; i <= n; ++i) {
                if (i == a || i == b) continue;
                int v = q3(a, b, i);
                if (v != S) return false;
                if (++checks >= 32) break;
            }
            return true;
        };

        auto attempt = [&](int a, int b) -> pair<int,int> {
            // c: argmax_k q(a,b,k)
            int c = -1, cv = INT_MIN;
            for (int k = 1; k <= n; ++k) if (k != a && k != b) {
                int v = q3(a, b, k);
                if (v > cv) { cv = v; c = k; }
            }
            if (c == -1) return make_pair(a, b);
            // d: argmin_k q(a,c,k)
            int d = -1, dv = INT_MAX;
            for (int k = 1; k <= n; ++k) if (k != a && k != c) {
                int v = q3(a, c, k);
                if (v < dv) { dv = v; d = k; }
            }
            if (d != -1 && verify(d, c)) return make_pair(d, c); // (min,max)
            // try using b with c
            d = -1, dv = INT_MAX;
            for (int k = 1; k <= n; ++k) if (k != b && k != c) {
                int v = q3(b, c, k);
                if (v < dv) { dv = v; d = k; }
            }
            if (d != -1 && verify(d, c)) return make_pair(d, c);
            // alternatively, find new c' with (a,d)
            if (d != -1) {
                int c2 = -1, c2v = INT_MIN;
                for (int k = 1; k <= n; ++k) if (k != a && k != d) {
                    int v = q3(a, d, k);
                    if (v > c2v) { c2v = v; c2 = k; }
                }
                if (c2 != -1 && verify(d, c2)) return make_pair(d, c2);
            }
            return make_pair(a, b);
        };

        // Try a few attempts
        vector<pair<int,int>> seeds;
        seeds.emplace_back(1, 2);
        if (n >= 3) seeds.emplace_back(1, 3);
        if (n >= 4) seeds.emplace_back(2, 3);
        std::mt19937 rng(912367);
        for (int t = 0; t < 3 && (int)seeds.size() < 6; ++t) {
            int a = uniform_int_distribution<int>(1, n)(rng);
            int b = uniform_int_distribution<int>(1, n)(rng);
            while (b == a) b = uniform_int_distribution<int>(1, n)(rng);
            seeds.emplace_back(a, b);
        }
        for (auto [a, b] : seeds) {
            auto p = attempt(a, b);
            if (verify(p.first, p.second)) return p;
        }
        // Fallback: last attempt from 1,2
        return attempt(1, 2);
    }

    long long mod_pow_base(int n) {
        // Not used directly; we'll iteratively build hash.
        return 0;
    }
}

int guess(int n, int Taskid) {
    using namespace GuessInternal;
    vector<long long> A(n + 1, 0);

    // Step 1: find indices of global min and max
    auto [imin, imax] = find_extremes(n);

    // Step 2: compute S = A_min + A_max using any third t
    int t = 1;
    if (t == imin || t == imax) t = (t % n) + 1;
    if (t == imin || t == imax) t = (t % n) + 1;
    int S = q3(imin, imax, t);

    // Step 3: pick three non-extreme distinct indices a,b,c
    vector<int> pool;
    pool.reserve(n);
    for (int i = 1; i <= n; ++i) if (i != imin && i != imax) pool.push_back(i);
    // n >= 5 by constraints, so pool has at least 3
    int a = pool.size() >= 1 ? pool[0] : -1;
    int b = pool.size() >= 2 ? pool[1] : -1;
    int c = pool.size() >= 3 ? pool[2] : -1;

    auto pair_sum = [&](int i, int j) -> long long {
        // Ai + Aj = q(imin,i,j) + q(imax,i,j) - S
        int v1 = q3(imin, i, j);
        int v2 = q3(imax, i, j);
        return (long long)v1 + (long long)v2 - S;
    };

    long long sab = pair_sum(a, b);
    long long sac = pair_sum(a, c);
    long long sbc = pair_sum(b, c);

    // Solve for A[a], A[b], A[c]
    long long Aa = (sab + sac - sbc) / 2;
    long long Ab = sab - Aa;
    long long Ac = sac - Aa;
    A[a] = Aa; A[b] = Ab; A[c] = Ac;

    // Step 4: determine which of (imin, imax) is global min vs max and compute their values.
    // Use the property for any two non-extreme a,b:
    // - If e is min: q(e,a,b) = A_e + max(Aa,Ab)
    // - If e is max: q(e,a,b) = A_e + min(Aa,Ab)
    int qe1ab = q3(imin, a, b);
    long long mx_ab = max(Aa, Ab);
    long long mn_ab = min(Aa, Ab);
    long long cand_e1_min = (long long)qe1ab - mx_ab;
    long long cand_e1_max = (long long)qe1ab - mn_ab;
    long long min_known = min({Aa, Ab, Ac});
    long long max_known = max({Aa, Ab, Ac});
    bool e1_is_min = cand_e1_min < min_known;
    long long val_e1, val_e2;
    if (e1_is_min) {
        val_e1 = cand_e1_min; // A_min
        val_e2 = (long long)S - val_e1; // A_max
    } else {
        val_e1 = cand_e1_max; // A_max
        val_e2 = (long long)S - val_e1; // A_min
    }
    A[imin] = val_e1;
    A[imax] = val_e2;

    // Step 5: fill the remaining values using sums with anchor a
    for (int i = 1; i <= n; ++i) {
        if (i == a || i == b || i == c || i == imin || i == imax) continue;
        long long sia = pair_sum(i, a); // Ai + Aa
        A[i] = sia - Aa;
    }

    // Compute and return hash
    long long ret = 0;
    for (int i = n; i >= 1; --i) {
        ret = (ret + (A[i] % MOD + MOD) % MOD) % MOD;
        ret = (ret * BASE) % MOD;
    }
    // debug (stderr) - safe for local testing
    // fprintf(stderr, "imin=%d imax=%d S=%d a=%d b=%d c=%d ret=%lld\n", imin, imax, S, a, b, c, ret);
    return (int)(ret % MOD);
}
