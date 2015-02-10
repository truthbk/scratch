#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
using namespace std;


class MyArray {
    private:
        int ** arr;
        int x_sz,y_sz;
        multimap<int, pair<int,int> > coordmap;
    public:
        MyArray(int x, int y) 
         : x_sz(x)
         , y_sz(y) 
           {
            arr = new int*[x_sz];
            for(int i=0 ; i<x_sz ; i++) {
                arr[i] = new int[y_sz];
            }
        }
        ~MyArray() {
            for(int i=0 ; i<x_sz ; i++) {
                delete arr[i];
            }
            delete arr;
        }
        void populate() {
            for(int i=0 ; i<x_sz ; i++) {
                for(int j=0 ; j<y_sz ; j++) {
                    char digit;
                    cin >> digit;
                    arr[i][j] = static_cast<int>(digit-'0');   
                    pair<int,int> coord(i,j);
                    coordmap.insert(pair<int, pair<int,int> >(arr[i][j], coord));
                }
            }
        }
        bool has_pattern(int **pat,int x, int y) {
            pair < multimap<int, pair<int,int> >::iterator, 
                   multimap<int, pair<int,int> >::iterator> candidates;
            candidates = coordmap.equal_range(pat[0][0]);
            bool match = false;
            for (multimap<int,pair<int,int> >::iterator it=candidates.first; 
                 it!=candidates.second && !match; ++it) {
                int s_x = it->second.first;
                int s_y = it->second.second;

                bool bounds_x = ((s_x + x )<=x_sz);
                bool bounds_y = ((s_y + y )<=y_sz);
                if(bounds_x && bounds_y) {
                    bool mismatch = false;
                    bool worthit = (arr[s_x][s_y] == pat[0][0]);
                    worthit = worthit && (arr[s_x][s_y] == pat[0][0]);
                    worthit = worthit && (arr[s_x][s_y+y-1] == pat[0][y-1]);
                    worthit = worthit && (arr[s_x+x-1][s_y] == pat[x-1][0]);
                    worthit = worthit && (arr[s_x+x-1][s_y+y-1] == pat[x-1][y-1]);
                    if(!worthit) {
                        mismatch = true;
                    } else {
                        //cout << "Exploring: " << s_x << ", " << s_y << endl;
                        for( int i=0; i<x && !mismatch ; i++) {
                            for( int j=0; j<y && !mismatch ; j++) {
                                if(arr[s_x+i][s_y+j] != pat[i][j]) {
                                    mismatch = true;
                                }
                            }
                        }
                    }
                    match = mismatch ? false : true ;
                }
            }
            return match;
        }
};

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */   
    int iter;
    cin >> iter;
    for(int i = 0 ; i<iter ; i++) {
        int x,y,x_pat,y_pat;
        int **pat;
        cin >> x;
        cin >> y;
        MyArray array(x,y);
        array.populate();
        cin >> x_pat;
        cin >> y_pat;        
        pat = new int*[x_pat];
        for(int j=0 ; j<x_pat ; j++) {
            pat[j] = new int[y_pat];
        }
        for(int j=0 ; j<x_pat ; j++) {
            for(int k=0 ; k<y_pat ; k++) {
                char digit;
                cin >> digit;
                pat[j][k] = static_cast<int>(digit-'0');
            }
        }
        cout << ((array.has_pattern(pat,x_pat,y_pat)) ? "YES" : "NO") << endl;
    }
    return 0;
}





