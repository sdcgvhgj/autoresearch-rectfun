# Reference optimal rectilinear partition via concave-vertex chords (corner-space edges).
from brute import gen, union_cells, brute_min
import collections

def concave_vertices(cells, maxX, maxY):
    cav=[]
    for cx in range(0,maxX+1):
        for cy in range(0,maxY+1):
            bl=(cx,cy) in cells; br=(cx+1,cy) in cells
            tl=(cx,cy+1) in cells; tr=(cx+1,cy+1) in cells
            if bl+br+tl+tr==3: cav.append((cx,cy))
    return cav

def hseg_int(cells,x,cy):  # H-edge (x,cy) interior: cells (x+1,cy)&(x+1,cy+1) inside
    return (x+1,cy) in cells and (x+1,cy+1) in cells
def vseg_int(cells,cx,y):
    return (cx,y+1) in cells and (cx+1,y+1) in cells

def build_chords(cells,cav):
    H=[];V=[]
    byrow={};bycol={}
    for (cx,cy) in cav: byrow.setdefault(cy,[]).append(cx); bycol.setdefault(cx,[]).append(cy)
    for cy,xs in byrow.items():
        xs.sort()
        for i in range(len(xs)-1):
            a,b=xs[i],xs[i+1]
            if all(hseg_int(cells,x,cy) for x in range(a,b)): H.append((a,b,cy))
    for cx,ys in bycol.items():
        ys.sort()
        for i in range(len(ys)-1):
            a,b=ys[i],ys[i+1]
            if all(vseg_int(cells,cx,y) for y in range(a,b)): V.append((cx,a,b))
    return H,V

def cross(h,v):
    a,b,cy=h; cx,c,d=v
    return a<=cx<=b and c<=cy<=d

def solve_chords(H,V):
    adj=[[j for j,v in enumerate(V) if cross(h,v)] for h in H]
    matchV=[-1]*len(V)
    def aug(u,seen):
        for v in adj[u]:
            if not seen[v]:
                seen[v]=True
                if matchV[v]==-1 or aug(matchV[v],seen): matchV[v]=u; return True
        return False
    for u in range(len(H)):
        aug(u,[False]*len(V))
    matchH=[-1]*len(H)
    for v in range(len(V)):
        if matchV[v]!=-1: matchH[matchV[v]]=v
    visH=[False]*len(H); visV=[False]*len(V)
    dq=collections.deque()
    for u in range(len(H)):
        if matchH[u]==-1: visH[u]=True; dq.append(u)
    while dq:
        u=dq.popleft()
        for v in adj[u]:
            if not visV[v]:
                visV[v]=True
                if matchV[v]!=-1 and not visH[matchV[v]]: visH[matchV[v]]=True; dq.append(matchV[v])
    chH=[u for u in range(len(H)) if visH[u]]
    chV=[v for v in range(len(V)) if not visV[v]]
    return chH,chV

def construct_and_count(cells):
    if not cells: return 0
    xs=[x for x,_ in cells]; ys=[y for _,y in cells]
    maxX=max(xs); maxY=max(ys)
    cav=concave_vertices(cells,maxX,maxY)
    H,V=build_chords(cells,cav)
    chH,chV=solve_chords(H,V)
    chosen_h=[H[i] for i in chH]; chosen_v=[V[i] for i in chV]
    Hw=set(); Vw=set()  # H-edge walls (x,cy); V-edge walls (cx,y)
    # boundary
    for (x,y) in cells:
        if (x,y-1) not in cells: Hw.add((x-1,y-1))
        if (x,y+1) not in cells: Hw.add((x-1,y))
        if (x-1,y) not in cells: Vw.add((x-1,y-1))
        if (x+1,y) not in cells: Vw.add((x,y-1))
    for (a,b,cy) in chosen_h:
        for x in range(a,b): Hw.add((x,cy))
    for (cx,c,d) in chosen_v:
        for y in range(c,d): Vw.add((cx,y))
    resolved=set()
    for (a,b,cy) in chosen_h: resolved.add((a,cy)); resolved.add((b,cy))
    for (cx,c,d) in chosen_v: resolved.add((cx,c)); resolved.add((cx,d))
    # vertical wall passes through corner (x,cy) if V-edge (x,cy-1) or (x,cy) in Vw
    def vwall_at(x,cy): return (x,cy-1) in Vw or (x,cy) in Vw
    def hwall_at(x,cy): return (x-1,cy) in Hw or (x,cy) in Hw
    for (cx,cy) in cav:
        if (cx,cy) in resolved: continue
        bl=(cx,cy) in cells;br=(cx+1,cy) in cells;tl=(cx,cy+1) in cells;tr=(cx+1,cy+1) in cells
        # choose horizontal direction that is interior at first step
        drew=False
        # right interior if br and tr inside
        if br and tr:  # extend right
            x=cx
            while hseg_int(cells,x,cy):
                Hw.add((x,cy)); x+=1
                if vwall_at(x,cy): break
            drew=True
        elif bl and tl:  # extend left
            x=cx
            while hseg_int(cells,x-1,cy):
                Hw.add((x-1,cy)); x-=1
                if vwall_at(x,cy): break
            drew=True
        if not drew:
            # vertical extension fallback
            if tl and tr:
                y=cy
                while vseg_int(cells,cx,y):
                    Vw.add((cx,y)); y+=1
                    if hwall_at(cx,y): break
            elif bl and br:
                y=cy
                while vseg_int(cells,cx,y-1):
                    Vw.add((cx,y-1)); y-=1
                    if hwall_at(cx,y): break
    # flood fill
    visited=set(); rects=0; ok=True
    for start in cells:
        if start in visited: continue
        dq=collections.deque([start]); visited.add(start)
        minx=maxx=start[0]; miny=maxy=start[1]; cnt=0
        while dq:
            x,y=dq.popleft(); cnt+=1
            minx=min(minx,x);maxx=max(maxx,x);miny=min(miny,y);maxy=max(maxy,y)
            if (x+1,y) in cells and (x,y-1) not in Vw and (x+1,y) not in visited: visited.add((x+1,y));dq.append((x+1,y))
            if (x-1,y) in cells and (x-1,y-1) not in Vw and (x-1,y) not in visited: visited.add((x-1,y));dq.append((x-1,y))
            if (x,y+1) in cells and (x-1,y) not in Hw and (x,y+1) not in visited: visited.add((x,y+1));dq.append((x,y+1))
            if (x,y-1) in cells and (x-1,y-1) not in Hw and (x,y-1) not in visited: visited.add((x,y-1));dq.append((x,y-1))
        if (maxx-minx+1)*(maxy-miny+1)!=cnt: ok=False
        rects+=1
    return rects if ok else -rects

if __name__=="__main__":
    bad=0;tested=0;invalid=0
    for seed in range(5000):
        r=gen(maxc=4,grid=5,seed=seed)
        cells=union_cells(r)
        if not cells: continue
        bm=brute_min(cells); om=construct_and_count(cells); tested+=1
        if om<0: invalid+=1
        if abs(om)!=bm:
            bad+=1
            if bad<=8: print(f"MISMATCH seed={seed} brute={bm} opt={om} rects={r}")
    print(f"tested={tested} mismatches={bad} invalid={invalid}")
