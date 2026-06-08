// Standalone optimal rectilinear partition via concave-vertex chords, on
// coordinate-compressed cells. Reads "n / x1 x2 y1 y2..." outputs "m / rects".
// For validation against the Python reference + checker. Assumes union fits cap.
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cstring>
#include <array>
#include <functional>
using namespace std;

struct R{int x1,x2,y1,y2;};

int main(){
    int n; if(scanf("%d",&n)!=1) return 0;
    vector<R> rs(n);
    vector<int> xsv, ysv;
    for(int i=0;i<n;i++){
        scanf("%d %d %d %d",&rs[i].x1,&rs[i].x2,&rs[i].y1,&rs[i].y2);
        xsv.push_back(rs[i].x1); xsv.push_back(rs[i].x2+1);
        ysv.push_back(rs[i].y1); ysv.push_back(rs[i].y2+1);
    }
    sort(xsv.begin(),xsv.end()); xsv.erase(unique(xsv.begin(),xsv.end()),xsv.end());
    sort(ysv.begin(),ysv.end()); ysv.erase(unique(ysv.begin(),ysv.end()),ysv.end());
    int nx=xsv.size(), ny=ysv.size();           // grid lines
    int cx=nx-1, cy=ny-1;                        // cells
    // occupancy via 2D diff
    vector<char> occ((size_t)cx*cy,0);
    {
        vector<int> diff((size_t)(cx+1)*(cy+1),0);
        auto xi=[&](int v){return (int)(lower_bound(xsv.begin(),xsv.end(),v)-xsv.begin());};
        auto yi=[&](int v){return (int)(lower_bound(ysv.begin(),ysv.end(),v)-ysv.begin());};
        for(auto&r:rs){
            int a=xi(r.x1), b=xi(r.x2+1), c=yi(r.y1), d=yi(r.y2+1);
            diff[(size_t)a*(cy+1)+c]++; diff[(size_t)b*(cy+1)+c]--;
            diff[(size_t)a*(cy+1)+d]--; diff[(size_t)b*(cy+1)+d]++;
        }
        // proper 2D prefix
        vector<long> ps((size_t)(cx+1)*(cy+1),0);
        for(int i=0;i<=cx;i++)for(int j=0;j<=cy;j++){
            long v=diff[(size_t)i*(cy+1)+j];
            if(i)v+=ps[(size_t)(i-1)*(cy+1)+j];
            if(j)v+=ps[(size_t)i*(cy+1)+j-1];
            if(i&&j)v-=ps[(size_t)(i-1)*(cy+1)+j-1];
            ps[(size_t)i*(cy+1)+j]=v;
            if(i<cx&&j<cy) occ[(size_t)i*cy+j]=(v>0);
        }
    }
    auto inside=[&](int i,int j){return i>=0&&i<cx&&j>=0&&j<cy&&occ[(size_t)i*cy+j];};
    // concave vertices: corner (i,j), 4 cells (i-1,j-1)(i,j-1)(i-1,j)(i,j)
    // bl=(i-1,j-1) br=(i,j-1) tl=(i-1,j) tr=(i,j)
    vector<pair<int,int>> cav;
    for(int i=0;i<=cx;i++)for(int j=0;j<=cy;j++){
        int bl=inside(i-1,j-1),br=inside(i,j-1),tl=inside(i-1,j),tr=inside(i,j);
        if(bl+br+tl+tr==3) cav.push_back({i,j});
    }
    // interior edge tests (corner space): H-edge (x, line j) interior iff cells (x,j-1)&(x,j) inside
    auto hseg=[&](int x,int j){return inside(x,j-1)&&inside(x,j);};
    auto vseg=[&](int i,int y){return inside(i-1,y)&&inside(i,y);};
    // chords: H = (a,b,j) row j between consecutive concave; V = (i,c,d)
    vector<array<int,3>> H,V;
    {
        // group by row j (the cy coordinate) -> xs
        vector<vector<int>> byrow(cy+1), bycol(cx+1);
        for(auto&p:cav){byrow[p.second].push_back(p.first); bycol[p.first].push_back(p.second);}
        for(int j=0;j<=cy;j++){auto&xs=byrow[j]; sort(xs.begin(),xs.end());
            for(size_t t=0;t+1<xs.size();t++){int a=xs[t],b=xs[t+1]; bool ok=true;
                for(int x=a;x<b;x++) if(!hseg(x,j)){ok=false;break;}
                if(ok) H.push_back({{a,b,j}});
            }}
        for(int i=0;i<=cx;i++){auto&ys=bycol[i]; sort(ys.begin(),ys.end());
            for(size_t t=0;t+1<ys.size();t++){int a=ys[t],b=ys[t+1]; bool ok=true;
                for(int y=a;y<b;y++) if(!vseg(i,y)){ok=false;break;}
                if(ok) V.push_back({{i,a,b}});
            }}
    }
    int nh=H.size(), nv=V.size();
    // bipartite: edge if intersect inclusive: a<=cx<=b && c<=cy<=d
    vector<vector<int>> adj(nh);
    for(int h=0;h<nh;h++){int a=H[h][0],b=H[h][1],jj=H[h][2];
        for(int v=0;v<nv;v++){int ii=V[v][0],c=V[v][1],d=V[v][2];
            if(a<=ii&&ii<=b&&c<=jj&&jj<=d) adj[h].push_back(v);}}
    // Kuhn matching
    vector<int> mv(nv,-1), mh(nh,-1);
    vector<char> used;
    function<bool(int)> aug=[&](int u)->bool{
        for(int v:adj[u]) if(!used[v]){used[v]=1;
            if(mv[v]==-1||aug(mv[v])){mv[v]=u;mh[u]=v;return true;}}
        return false;};
    for(int u=0;u<nh;u++){used.assign(nv,0); aug(u);}
    // Konig: MIS = (visited H) + (unvisited V); reachable from unmatched H via alt
    vector<char> visH(nh,0),visV(nv,0);
    vector<int> st;
    for(int u=0;u<nh;u++) if(mh[u]==-1){visH[u]=1;st.push_back(u);}
    while(!st.empty()){int u=st.back();st.pop_back();
        for(int v:adj[u]) if(!visV[v]){visV[v]=1; if(mv[v]!=-1&&!visH[mv[v]]){visH[mv[v]]=1;st.push_back(mv[v]);}}}
    // walls: H-edge (x,j) and V-edge (i,y)
    // store as sets via hash
    vector<char> Hw((size_t)cx*(cy+1),0), Vw((size_t)(cx+1)*cy,0);
    auto setH=[&](int x,int j){ if(x>=0&&x<cx&&j>=0&&j<=cy) Hw[(size_t)x*(cy+1)+j]=1;};
    auto setV=[&](int i,int y){ if(i>=0&&i<=cx&&y>=0&&y<cy) Vw[(size_t)i*cy+y]=1;};
    auto getH=[&](int x,int j){return x>=0&&x<cx&&j>=0&&j<=cy&&Hw[(size_t)x*(cy+1)+j];};
    auto getV=[&](int i,int y){return i>=0&&i<=cx&&y>=0&&y<cy&&Vw[(size_t)i*cy+y];};
    // boundary walls
    for(int i=0;i<cx;i++)for(int j=0;j<cy;j++) if(inside(i,j)){
        if(!inside(i,j-1)) setH(i,j);
        if(!inside(i,j+1)) setH(i,j+1);
        if(!inside(i-1,j)) setV(i,j);
        if(!inside(i+1,j)) setV(i+1,j);
    }
    vector<char> resolved((size_t)(cx+1)*(cy+1),0);
    auto mark=[&](int i,int j){resolved[(size_t)i*(cy+1)+j]=1;};
    auto isres=[&](int i,int j){return resolved[(size_t)i*(cy+1)+j];};
    for(int h=0;h<nh;h++) if(visH[h]){int a=H[h][0],b=H[h][1],j=H[h][2];
        for(int x=a;x<b;x++) setH(x,j); mark(a,j); mark(b,j);}
    for(int v=0;v<nv;v++) if(!visV[v]){int i=V[v][0],c=V[v][1],d=V[v][2];
        for(int y=c;y<d;y++) setV(i,y); mark(i,c); mark(i,d);}
    // vertical wall through corner (x,cy): V-edge (x,cy-1) or (x,cy)
    auto vwallAt=[&](int x,int j){return getV(x,j-1)||getV(x,j);};
    auto hwallAt=[&](int i,int y){return getH(i-1,y)||getH(i,y);};
    for(auto&p:cav){int i=p.first,j=p.second; if(isres(i,j)) continue;
        int bl=inside(i-1,j-1),br=inside(i,j-1),tl=inside(i-1,j),tr=inside(i,j);
        bool drew=false;
        if(br&&tr){int x=i; while(hseg(x,j)){setH(x,j); x++; if(vwallAt(x,j))break;} drew=true;}
        else if(bl&&tl){int x=i; while(hseg(x-1,j)){setH(x-1,j); x--; if(vwallAt(x,j))break;} drew=true;}
        if(!drew){
            if(tl&&tr){int y=j; while(vseg(i,y)){setV(i,y); y++; if(hwallAt(i,y))break;}}
            else if(bl&&br){int y=j; while(vseg(i,y-1)){setV(i,y-1); y--; if(hwallAt(i,y))break;}}
        }
    }
    // flood fill cells -> rectangles
    vector<char> vis((size_t)cx*cy,0);
    vector<array<int,4>> out;
    vector<int> stack;
    for(int si=0;si<cx;si++)for(int sj=0;sj<cy;sj++){
        if(!inside(si,sj)||vis[(size_t)si*cy+sj])continue;
        stack.clear(); stack.push_back(si*cy+sj); vis[(size_t)si*cy+sj]=1;
        int mni=si,mxi=si,mnj=sj,mxj=sj;
        while(!stack.empty()){int cur=stack.back();stack.pop_back();
            int i=cur/cy,j=cur%cy;
            mni=min(mni,i);mxi=max(mxi,i);mnj=min(mnj,j);mxj=max(mxj,j);
            // right (i+1,j): V-edge (i+1,j)
            if(inside(i+1,j)&&!getV(i+1,j)&&!vis[(size_t)(i+1)*cy+j]){vis[(size_t)(i+1)*cy+j]=1;stack.push_back((i+1)*cy+j);}
            if(inside(i-1,j)&&!getV(i,j)&&!vis[(size_t)(i-1)*cy+j]){vis[(size_t)(i-1)*cy+j]=1;stack.push_back((i-1)*cy+j);}
            if(inside(i,j+1)&&!getH(i,j+1)&&!vis[(size_t)i*cy+j+1]){vis[(size_t)i*cy+j+1]=1;stack.push_back(i*cy+j+1);}
            if(inside(i,j-1)&&!getH(i,j)&&!vis[(size_t)i*cy+j-1]){vis[(size_t)i*cy+j-1]=1;stack.push_back(i*cy+j-1);}
        }
        out.push_back({{xsv[mni],xsv[mxi+1]-1,ysv[mnj],ysv[mxj+1]-1}});
    }
    printf("%d\n",(int)out.size());
    for(auto&o:out) printf("%d %d %d %d\n",o[0],o[1],o[2],o[3]);
    return 0;
}
