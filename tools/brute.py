import sys, random
from functools import lru_cache

def union_cells(rects):
    s=set()
    for x1,x2,y1,y2 in rects:
        for x in range(x1,x2+1):
            for y in range(y1,y2+1):
                s.add((x,y))
    return s

def brute_min(cells):
    cells=frozenset(cells)
    from functools import lru_cache
    memo={}
    def rec(rem):
        if not rem: return 0
        if rem in memo: return memo[rem]
        # pick lexicographically smallest uncovered cell as top-left
        c=min(rem)  # (x,y) smallest x then y
        x0,y0=c
        best=10**9
        # try all rectangles with top-left corner (x0,y0) fully inside rem
        maxw=0
        # extend width while (x0+w,y0) in rem
        x=x0
        while (x,y0) in rem:
            x+=1
        wlim=x-1
        for x2 in range(x0,wlim+1):
            # for this width, extend height while full row inside
            y=y0
            ok=True
            while ok:
                if all((xx,y) in rem for xx in range(x0,x2+1)):
                    y+=1
                else:
                    break
            hlim=y-1
            for y2 in range(y0,hlim+1):
                newrem=rem-frozenset((xx,yy) for xx in range(x0,x2+1) for yy in range(y0,y2+1))
                r=1+rec(newrem)
                if r<best: best=r
        memo[rem]=best
        return best
    return rec(cells)

def gen(maxc=4, grid=5, seed=0):
    random.seed(seed)
    rects=[]
    occ=set()
    for _ in range(random.randint(1,maxc)):
        for _try in range(20):
            x1=random.randint(1,grid); x2=random.randint(x1,grid)
            y1=random.randint(1,grid); y2=random.randint(y1,grid)
            cells=set((x,y) for x in range(x1,x2+1) for y in range(y1,y2+1))
            if cells & occ: continue
            occ|=cells; rects.append((x1,x2,y1,y2)); break
    return rects

if __name__=="__main__":
    # demo
    for seed in range(5):
        r=gen(seed=seed)
        c=union_cells(r)
        print(seed, "rects",r,"cells",len(c),"min",brute_min(c) if c else 0)
