#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cstdint>
using namespace std;

struct Rect { int x1, x2, y1, y2; };

// ---------- fast input ----------
static inline char* readAll(size_t& outLen) {
    size_t cap = 1 << 24, len = 0;
    char* data = (char*)malloc(cap);
    while (true) {
        if (len + (1 << 20) > cap) { cap <<= 1; data = (char*)realloc(data, cap); }
        size_t got = fread(data + len, 1, 1 << 20, stdin);
        len += got;
        if (got == 0) break;
    }
    data[len] = 0;
    outLen = len;
    return data;
}

static inline int parseInt(char*& p) {
    while (*p && (*p < '0' || *p > '9')) ++p;
    int v = 0;
    while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); ++p; }
    return v;
}

// ---------- fast output ----------
static char* obuf;
static size_t opos, ocap;
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

int main() {
    size_t len;
    char* data = readAll(len);
    char* p = data;

    int n = parseInt(p);
    vector<Rect> r(n);
    for (int i = 0; i < n; ++i) {
        r[i].x1 = parseInt(p);
        r[i].x2 = parseInt(p);
        r[i].y1 = parseInt(p);
        r[i].y2 = parseInt(p);
    }
    free(data);

    // Iterated identical-edge merge.
    // Vertical merge: same (x1,x2), contiguous in y (prev.y2+1==cur.y1).
    // Horizontal merge: same (y1,y2), contiguous in x (prev.x2+1==cur.x1).
    bool changed = true;
    int rounds = 0;
    while (changed && rounds < 8) {
        changed = false;
        ++rounds;

        // Vertical merge
        sort(r.begin(), r.end(), [](const Rect& a, const Rect& b) {
            if (a.x1 != b.x1) return a.x1 < b.x1;
            if (a.x2 != b.x2) return a.x2 < b.x2;
            return a.y1 < b.y1;
        });
        {
            vector<Rect> out;
            out.reserve(r.size());
            for (const Rect& cur : r) {
                if (!out.empty()) {
                    Rect& last = out.back();
                    if (last.x1 == cur.x1 && last.x2 == cur.x2 && last.y2 + 1 == cur.y1) {
                        last.y2 = cur.y2; changed = true; continue;
                    }
                }
                out.push_back(cur);
            }
            r.swap(out);
        }

        // Horizontal merge
        sort(r.begin(), r.end(), [](const Rect& a, const Rect& b) {
            if (a.y1 != b.y1) return a.y1 < b.y1;
            if (a.y2 != b.y2) return a.y2 < b.y2;
            return a.x1 < b.x1;
        });
        {
            vector<Rect> out;
            out.reserve(r.size());
            for (const Rect& cur : r) {
                if (!out.empty()) {
                    Rect& last = out.back();
                    if (last.y1 == cur.y1 && last.y2 == cur.y2 && last.x2 + 1 == cur.x1) {
                        last.x2 = cur.x2; changed = true; continue;
                    }
                }
                out.push_back(cur);
            }
            r.swap(out);
        }
    }

    // ---------- output ----------
    ocap = 1 << 24; opos = 0; obuf = (char*)malloc(ocap);
    writeInt((int)r.size()); writeChar('\n');
    for (const Rect& rc : r) {
        writeInt(rc.x1); writeChar(' ');
        writeInt(rc.x2); writeChar(' ');
        writeInt(rc.y1); writeChar(' ');
        writeInt(rc.y2); writeChar('\n');
    }
    fwrite(obuf, 1, opos, stdout);
    free(obuf);
    return 0;
}
