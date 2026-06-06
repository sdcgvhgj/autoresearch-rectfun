#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <climits>
#include <cstdint>
using namespace std;

struct Rect { int x1, x2, y1, y2; };

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
    inline int prev(int i) { // largest set j <= i, or -1
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
struct Ev { int x, lo, hi, type; };

// Decomposition along the sweep axis; intervals over the second axis (values in
// [1, maxB]). Sweep coords in [1, maxSweep+1].
static void decompose(const vector<Rect>& rects, vector<Rect>& res,
                      int maxSweep, vector<Ev>& ev, vector<Ev>& sorted, vector<int>& cnt) {
    int N = (int)rects.size();
    ev.resize((size_t)N * 2);
    size_t k = 0;
    for (const Rect& r : rects) {
        ev[k++] = {r.x1, r.y1, r.y2, +1};
        ev[k++] = {r.x2 + 1, r.y1, r.y2, -1};
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
            int mhi = HI[mlo], xstart = XS[mlo];
            veb.clr(mlo);
            if (xstart <= c - 1) res.push_back({xstart, c - 1, mlo, mhi});
            if (mlo <= alo - 1) { veb.set(mlo); HI[mlo] = alo - 1; XS[mlo] = c; }
            if (ahi + 1 <= mhi) { veb.set(ahi + 1); HI[ahi + 1] = mhi; XS[ahi + 1] = c; }
        } else {
            int nl = alo, nh = ahi;
            int s = veb.prev(alo);
            if (s != -1 && HI[s] == alo - 1) {
                nl = s;
                if (XS[s] <= c - 1) res.push_back({XS[s], c - 1, s, HI[s]});
                veb.clr(s);
            }
            if (veb.test(ahi + 1)) {
                int a2 = ahi + 1; nh = HI[a2];
                if (XS[a2] <= c - 1) res.push_back({XS[a2], c - 1, a2, HI[a2]});
                veb.clr(a2);
            }
            veb.set(nl); HI[nl] = nh; XS[nl] = c;
        }
    }
}

int main() {
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
    vector<Ev> ev, sorted; vector<int> cnt;

    vector<Rect> vert; vert.reserve(r.size());
    decompose(r, vert, maxX, ev, sorted, cnt);

    vector<Rect> rt(r.size());
    for (size_t i = 0; i < r.size(); ++i) rt[i] = {r[i].y1, r[i].y2, r[i].x1, r[i].x2};
    vector<Rect> horiz; horiz.reserve(r.size());
    decompose(rt, horiz, maxY, ev, sorted, cnt);

    vector<Rect>* best = &vert;
    if (horiz.size() < vert.size()) {
        for (Rect& q : horiz) q = {q.y1, q.y2, q.x1, q.x2};
        best = &horiz;
    }
    // The checker requires m <= n. If a decomposition produced more rectangles
    // than the input (e.g. isolated rectangles), fall back to the input itself.
    if (best->size() > r.size()) best = &r;

    ocap = 1 << 24; opos = 0; obuf = (char*)malloc(ocap);
    writeInt((int)best->size()); writeChar('\n');
    for (const Rect& rc : *best) {
        writeInt(rc.x1); writeChar(' ');
        writeInt(rc.x2); writeChar(' ');
        writeInt(rc.y1); writeChar(' ');
        writeInt(rc.y2); writeChar('\n');
    }
    fwrite(obuf, 1, opos, stdout);
    free(obuf);
    return 0;
}
