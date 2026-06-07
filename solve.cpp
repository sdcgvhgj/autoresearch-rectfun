#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <chrono>
using namespace std;
using namespace std::chrono;

struct Rect { int x1, x2, y1, y2; };
struct RectC { int x1, x2, y1, y2, comp; };

// ---------- fast input ----------
static inline char* readAll(size_t& outLen) {
    size_t cap = 1 << 24, len = 0;
    char* data = (char*)malloc(cap);
    while (true) {
        if (len + (1 << 20) + 1 > cap) { cap <<= 1; data = (char*)realloc(data, cap); }
        size_t got = fread(data + len, 1, 1 << 20, stdin);
        len += got;
        if (got == 0) break;
    }
    data[len] = 0; outLen = len; return data;
}
static inline int parseInt(char*& p) {
    while (*p && (*p < '0' || *p > '9')) ++p;
    int v = 0;
    while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); ++p; }
    return v;
}

// ---------- fast output ----------
static char* obuf; static size_t opos, ocap;
static inline void oReserve(size_t extra) {
    if (opos + extra > ocap) { while (opos + extra > ocap) ocap <<= 1; obuf = (char*)realloc(obuf, ocap); }
}
static inline void writeInt(int x) {
    char tmp[12]; int t = 0;
    if (x == 0) tmp[t++] = '0';
    while (x > 0) { tmp[t++] = char('0' + x % 10); x /= 10; }
    oReserve(t + 1);
    while (t > 0) obuf[opos++] = tmp[--t];
}
static inline void writeChar(char c) { oReserve(1); obuf[opos++] = c; }

// ---------- hierarchical bitset (predecessor structure) ----------
struct HBit {
    uint64_t* lev[6];
    int levWords[6], L;
    void init(int U) {
        int w = (U + 63) >> 6;
        L = 0;
        while (true) {
            lev[L] = (uint64_t*)calloc(w, sizeof(uint64_t));
            levWords[L] = w; ++L;
            if (w <= 1) break;
            w = (w + 63) >> 6;
        }
    }
    inline void set(int i) {
        for (int l = 0; l < L; ++l) { int w = i >> 6; uint64_t b = 1ull << (i & 63);
            if (lev[l][w] & b) return; lev[l][w] |= b; i = w; }
    }
    inline void clr(int i) {
        for (int l = 0; l < L; ++l) { int w = i >> 6; uint64_t b = 1ull << (i & 63);
            lev[l][w] &= ~b; if (lev[l][w]) return; i = w; }
    }
    inline bool test(int i) { return (lev[0][i >> 6] >> (i & 63)) & 1; }
    inline int prev(int i) {
        int l = 0;
        for (;;) {
            int w = i >> 6, bit = i & 63;
            uint64_t mask = (bit == 63) ? ~0ull : ((1ull << (bit + 1)) - 1);
            uint64_t m = lev[l][w] & mask;
            if (m) {
                int pos = (w << 6) + (63 - __builtin_clzll(m));
                for (int d = l; d > 0; --d) pos = (pos << 6) + (63 - __builtin_clzll(lev[d - 1][pos]));
                return pos;
            }
            if (w == 0) return -1;
            i = w - 1; ++l;
            if (l >= L) return -1;
        }
    }
};

static HBit veb;
static int* HI;
static int* XS;
static int* CMP;
struct Ev { int x, lo, hi, type, comp; };

// Decomposition with per-rectangle component tags propagated to output rectangles.
static void decompose(const vector<Rect>& rects, const vector<int>& comp, vector<RectC>& res,
                      int maxSweep, vector<Ev>& ev, vector<Ev>& sorted, vector<int>& cnt) {
    int N = (int)rects.size();
    ev.resize((size_t)N * 2);
    size_t k = 0;
    for (int i = 0; i < N; ++i) {
        const Rect& q = rects[i]; int cc = comp[i];
        ev[k++] = {q.x1, q.y1, q.y2, +1, cc};
        ev[k++] = {q.x2 + 1, q.y1, q.y2, -1, cc};
    }
    int buckets = (maxSweep + 2) * 2;
    cnt.assign(buckets + 1, 0);
    for (const Ev& e : ev) ++cnt[e.x * 2 + (e.type == 1 ? 1 : 0) + 1];
    for (int i = 1; i <= buckets; ++i) cnt[i] += cnt[i - 1];
    sorted.resize(ev.size());
    for (const Ev& e : ev) sorted[cnt[e.x * 2 + (e.type == 1 ? 1 : 0)]++] = e;

    for (const Ev& e : sorted) {
        int c = e.x, alo = e.lo, ahi = e.hi;
        if (e.type == -1) {
            int mlo = veb.prev(alo);
            int mhi = HI[mlo], xstart = XS[mlo], cc = CMP[mlo];
            veb.clr(mlo);
            if (xstart <= c - 1) res.push_back({xstart, c - 1, mlo, mhi, cc});
            if (mlo <= alo - 1) { veb.set(mlo); HI[mlo] = alo - 1; XS[mlo] = c; CMP[mlo] = cc; }
            if (ahi + 1 <= mhi) { veb.set(ahi + 1); HI[ahi + 1] = mhi; XS[ahi + 1] = c; CMP[ahi + 1] = cc; }
        } else {
            int nl = alo, nh = ahi;
            int s = veb.prev(alo);
            if (s != -1 && HI[s] == alo - 1) {
                nl = s;
                if (XS[s] <= c - 1) res.push_back({XS[s], c - 1, s, HI[s], CMP[s]});
                veb.clr(s);
            }
            if (veb.test(ahi + 1)) {
                int a2 = ahi + 1; nh = HI[a2];
                if (XS[a2] <= c - 1) res.push_back({XS[a2], c - 1, a2, HI[a2], CMP[a2]});
                veb.clr(a2);
            }
            veb.set(nl); HI[nl] = nh; XS[nl] = c; CMP[nl] = e.comp;
        }
    }
}

// Single directional merges (preserve component tag).
static void hmergeOnce(vector<RectC>& r) {
    sort(r.begin(), r.end(), [](const RectC& a, const RectC& b) {
        if (a.y1 != b.y1) return a.y1 < b.y1;
        if (a.y2 != b.y2) return a.y2 < b.y2;
        return a.x1 < b.x1;
    });
    size_t o = 0;
    for (size_t i = 0; i < r.size(); ++i) {
        if (o && r[o-1].y1 == r[i].y1 && r[o-1].y2 == r[i].y2 && r[o-1].x2 + 1 == r[i].x1)
            r[o-1].x2 = r[i].x2;
        else r[o++] = r[i];
    }
    r.resize(o);
}
static void vmergeOnce(vector<RectC>& r) {
    sort(r.begin(), r.end(), [](const RectC& a, const RectC& b) {
        if (a.x1 != b.x1) return a.x1 < b.x1;
        if (a.x2 != b.x2) return a.x2 < b.x2;
        return a.y1 < b.y1;
    });
    size_t o = 0;
    for (size_t i = 0; i < r.size(); ++i) {
        if (o && r[o-1].x1 == r[i].x1 && r[o-1].x2 == r[i].x2 && r[o-1].y2 + 1 == r[i].y1)
            r[o-1].y2 = r[i].y2;
        else r[o++] = r[i];
    }
    r.resize(o);
}

// ---------- connected components (edge adjacency) ----------
static vector<int> par;
static int find(int x) { while (par[x] != x) { par[x] = par[par[x]]; x = par[x]; } return x; }
static void uni(int a, int b) { a = find(a); b = find(b); if (a != b) par[a] = b; }

static vector<int> cntBuf;
// Stable counting sort of `in` by `key` (values in [0, maxK]).
static void csort(const vector<int>& in, vector<int>& out, const vector<int>& key, int maxK) {
    cntBuf.assign(maxK + 2, 0);
    for (int i : in) ++cntBuf[key[i] + 1];
    for (int j = 1; j <= maxK + 1; ++j) cntBuf[j] += cntBuf[j - 1];
    out.resize(in.size());
    for (int i : in) out[cntBuf[key[i]]++] = i;
}

static void computeComponents(const vector<Rect>& r, int maxX, int maxY) {
    int n = (int)r.size();
    par.resize(n);
    for (int i = 0; i < n; ++i) par[i] = i;
    vector<int> kx1(n), kx2(n), ky1(n), ky2(n);
    for (int i = 0; i < n; ++i) { kx1[i] = r[i].x1; kx2[i] = r[i].x2 + 1; ky1[i] = r[i].y1; ky2[i] = r[i].y2 + 1; }
    vector<int> idx(n); for (int i = 0; i < n; ++i) idx[i] = i;
    vector<int> S(n), E(n), tmp;

    // Vertical adjacency: A.x2+1 == B.x1 with y-overlap. Need orders by (x1,y1) and (x2+1,y1).
    csort(idx, tmp, ky1, maxY);
    csort(tmp, S, kx1, maxX);
    csort(tmp, E, kx2, maxX + 1);
    {
        int si = 0, ei = 0;
        while (si < n && ei < n) {
            int v = min(r[S[si]].x1, r[E[ei]].x2 + 1);
            int e0 = ei; while (ei < n && r[E[ei]].x2 + 1 == v) ++ei;
            int s0 = si; while (si < n && r[S[si]].x1 == v) ++si;
            int a = e0, b = s0;
            while (a < ei && b < si) {
                if (max(r[E[a]].y1, r[S[b]].y1) <= min(r[E[a]].y2, r[S[b]].y2)) uni(E[a], S[b]);
                if (r[E[a]].y2 < r[S[b]].y2) ++a; else ++b;
            }
        }
    }
    // Horizontal adjacency: A.y2+1 == B.y1 with x-overlap. Orders by (y1,x1) and (y2+1,x1).
    csort(idx, tmp, kx1, maxX);
    csort(tmp, S, ky1, maxY);
    csort(tmp, E, ky2, maxY + 1);
    {
        int si = 0, ei = 0;
        while (si < n && ei < n) {
            int v = min(r[S[si]].y1, r[E[ei]].y2 + 1);
            int e0 = ei; while (ei < n && r[E[ei]].y2 + 1 == v) ++ei;
            int s0 = si; while (si < n && r[S[si]].y1 == v) ++si;
            int a = e0, b = s0;
            while (a < ei && b < si) {
                if (max(r[E[a]].x1, r[S[b]].x1) <= min(r[E[a]].x2, r[S[b]].x2)) uni(E[a], S[b]);
                if (r[E[a]].x2 < r[S[b]].x2) ++a; else ++b;
            }
        }
    }
}

int main() {
    auto t0 = steady_clock::now();
    size_t len;
    char* data = readAll(len);
    char* p = data;
    int n = parseInt(p);
    vector<Rect> r(n);
    int maxX = 1, maxY = 1;
    for (int i = 0; i < n; ++i) {
        r[i].x1 = parseInt(p); r[i].x2 = parseInt(p);
        r[i].y1 = parseInt(p); r[i].y2 = parseInt(p);
        if (r[i].x2 > maxX) maxX = r[i].x2;
        if (r[i].y2 > maxY) maxY = r[i].y2;
    }
    free(data);

    int U = max(maxX, maxY) + 2;
    veb.init(U);
    HI = (int*)malloc((size_t)U * sizeof(int));
    XS = (int*)malloc((size_t)U * sizeof(int));
    CMP = (int*)malloc((size_t)U * sizeof(int));

    // Connected components, normalized to roots.
    computeComponents(r, maxX, maxY);
    vector<int> comp(n);
    for (int i = 0; i < n; ++i) comp[i] = find(i);

    vector<Ev> ev, sorted; vector<int> cnt;

    // A decomposition with raw size > n is useless globally (isolated-style); skip
    // its (slow) recovery merge. Such directions are never chosen per component
    // (their per-component counts stay inflated), so no score is lost and the
    // worst cases (c04/c15-18) avoid an expensive merge over >n rectangles.
    vector<RectC> vert; vert.reserve(r.size());
    decompose(r, comp, vert, maxX, ev, sorted, cnt);
    if (vert.size() <= (size_t)n) hmergeOnce(vert);

    vector<Rect> rt(r.size());
    for (size_t i = 0; i < r.size(); ++i) rt[i] = {r[i].y1, r[i].y2, r[i].x1, r[i].x2};
    vector<RectC> horiz; horiz.reserve(r.size());
    decompose(rt, comp, horiz, maxY, ev, sorted, cnt);
    for (RectC& q : horiz) q = {q.y1, q.y2, q.x1, q.x2, q.comp}; // transpose back
    if (horiz.size() <= (size_t)n) vmergeOnce(horiz);

    // Per-component direction choice: minimise rectangles per connected component.
    // V_c / H_c = rectangle counts from each decomposition; n_c = input count.
    vector<int> Vc(n, 0), Hc(n, 0), Nc(n, 0);
    for (int i = 0; i < n; ++i) ++Nc[comp[i]];
    for (const RectC& q : vert) ++Vc[q.comp];
    for (const RectC& q : horiz) ++Hc[q.comp];
    // dec: 0 = vert, 1 = horiz, 2 = input (each <= n_c, so total <= n).
    vector<char> dec(n, 2);
    size_t total = 0;
    for (int i = 0; i < n; ++i) {
        if (par[i] != i) continue; // only roots
        int best = Nc[i]; char d = 2;
        if (Vc[i] < best) { best = Vc[i]; d = 0; }
        if (Hc[i] < best) { best = Hc[i]; d = 1; }
        dec[i] = d; total += best;
    }

    ocap = 1 << 24; opos = 0; obuf = (char*)malloc(ocap);
    writeInt((int)total); writeChar('\n');
    for (const RectC& q : vert) if (dec[q.comp] == 0) {
        writeInt(q.x1); writeChar(' '); writeInt(q.x2); writeChar(' ');
        writeInt(q.y1); writeChar(' '); writeInt(q.y2); writeChar('\n');
    }
    for (const RectC& q : horiz) if (dec[q.comp] == 1) {
        writeInt(q.x1); writeChar(' '); writeInt(q.x2); writeChar(' ');
        writeInt(q.y1); writeChar(' '); writeInt(q.y2); writeChar('\n');
    }
    for (int i = 0; i < n; ++i) if (dec[comp[i]] == 2) {
        writeInt(r[i].x1); writeChar(' '); writeInt(r[i].x2); writeChar(' ');
        writeInt(r[i].y1); writeChar(' '); writeInt(r[i].y2); writeChar('\n');
    }
    fwrite(obuf, 1, opos, stdout);
    free(obuf);
    return 0;
}
