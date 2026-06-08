from brute import gen, union_cells, brute_min
from opt import construct_and_count
import random
for (maxc,grid,N) in [(6,6,4000),(8,7,3000),(10,6,3000),(5,8,2000)]:
    bad=0;tested=0;inv=0;maxcells=0
    for seed in range(N):
        r=gen(maxc=maxc,grid=grid,seed=seed*7+1)
        cells=union_cells(r)
        if not cells or len(cells)>17: continue  # brute limit
        bm=brute_min(cells); om=construct_and_count(cells); tested+=1
        if om<0: inv+=1
        if abs(om)!=bm:
            bad+=1
            if bad<=5: print(f"  MISMATCH maxc={maxc} seed={seed} brute={bm} opt={om} rects={r}")
    print(f"maxc={maxc} grid={grid}: tested={tested} mismatches={bad} invalid={inv}")
