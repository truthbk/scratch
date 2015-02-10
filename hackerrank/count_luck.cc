#include <cmath>
#include <cstdio>
#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <algorithm>
using namespace std;

struct pairhash {
public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const
  {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};

class Maze {
    private:
    char ** arr;
    int x_sz,y_sz;
    pair<int,int> exit;
    pair<int,int> start;    
    
    public:
    Maze(int x, int y)·
        : x_sz(x)
        , y_sz(y){
            arr = new char*[x_sz];
            for(int i=0 ; i<x_sz ; i++) {
                arr[i] = new char[y_sz];
            }
        }
    ~Maze() {
        for(int i=0 ; i<x_sz ; i++) {
            delete arr[i];
        }
        delete arr;
    }
    void read_maze() {
        for(int i=0 ; i<x_sz ; i++) {
            for(int j=0 ; j<y_sz ; j++) {                
                cin >> arr[i][j];
                if(arr[i][j]=='*') {
                    exit = pair<int,int>(i,j);
                } else if(arr[i][j] =='M') {
                    start = pair<int,int>(i,j);
                }
            }
        }
    }
    vector<pair<int,int> > find_path() {
        unordered_map<pair<int,int>, bool, pairhash> visited;
        queue< vector<pair<int,int> > > bfs_queue;
        pair<int,int> pos = start;
        vector<pair<int,int> > v;
        v.push_back(pos);
        bfs_queue.push(v);        
                
        while(bfs_queue.size()) {
            vector<pair<int,int> > path(bfs_queue.front());
            bfs_queue.pop();
            if(path.back() == exit) {                
                return path;
            }
            //push neighbors into queue (if we haven't pushed them in yet)
            int x = path.back().first;
            int y = path.back().second;                        
            
            if(x<x_sz-1 && (arr[x+1][y]=='.' || arr[x+1][y]=='*')) {
                pair<int,int> step(x+1,y);
                if(visited.find(step) == visited.end() ) {
                    //new copy
                    vector<pair<int,int> > newpath(path);
                    newpath.push_back(step);
                    bfs_queue.push(newpath);                    
                }
            }
            if(x>0 && (arr[x-1][y]=='.' || arr[x-1][y]=='*')) {
                pair<int,int> step(x-1,y);
                if(visited.find(step) == visited.end() ) {
                    //new copy
                    vector<pair<int,int> > newpath(path);
                    newpath.push_back(step);
                    bfs_queue.push(newpath);                    
                }
            }
            if(y<y_sz-1 && (arr[x][y+1]=='.' || arr[x][y+1]=='*')) {
                pair<int,int> step(x,y+1);
                if(visited.find(step) == visited.end() ) {
                    //new copy
                    vector<pair<int,int> > newpath(path);
                    newpath.push_back(step);
                    bfs_queue.push(newpath);                    
                }
            }
            if(y>0 && (arr[x][y-1]=='.' || arr[x][y-1]=='*')) {
                pair<int,int> step(x,y-1);
                if(path.back() != step) {
                    //new copy
                    vector<pair<int,int> > newpath(path);
                    newpath.push_back(step);
                    bfs_queue.push(newpath);                    
                }
            }
            visited.insert(pair< pair<int,int>, bool>(path.back(),true));
        }
        
        //If we get here, there was no path to the exit.
        return vector<pair<int,int> >();
    }
    int path_forks(vector<pair<int,int> > path) {
        int forks = 0;
        vector<pair<int,int> >::iterator it(path.begin());
        for(; it != path.end() ; it++) {
            int x = it->first;
            int y = it->second;                        
            
            int options = 0;
            
            if(arr[x][y] != '*') {
            
                if(x<x_sz-1 && (arr[x+1][y]=='.' || arr[x+1][y]=='*')) {
                    options++;
                    if(arr[x][y] != 'M') {
                        arr[x][y] = 'p';
                    }
                }
                if(x>0 && (arr[x-1][y]=='.' || arr[x-1][y]=='*')) {
                    options++;
                    if(arr[x][y] != 'M') {
                        arr[x][y] = 'p';
                    }
                }
                if(y<y_sz-1 && (arr[x][y+1]=='.' || arr[x][y+1]=='*')) {
                    options++;
                    if(arr[x][y] != 'M') {
                        arr[x][y] = 'p';
                    }
                }
                if(y>0 && (arr[x][y-1]=='.' || arr[x][y-1]=='*')) {
                    options++;
                    if(arr[x][y] != 'M') {
                        arr[x][y] = 'p';
                    }
                }
                if(options>1) {
                    forks++;
                }
            }
        }
        //unmark path
        vector<pair<int,int> >::iterator it2(path.begin());
        for( ; it2 != path.end() ; it2++) {
            int x = it2->first;
            int y = it2->second;       
            
            if(arr[x][y] != 'M' && arr[x][y] != '*') {
                arr[x][y] = '.';
            }
        }
        return forks;
    }
};

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */   
    int iter;
    cin >> iter;
    for(int i=0 ; i<iter ; i++) {
        int x,y,k;
        cin >> x;
        cin >> y;
        Maze m(x,y);
        m.read_maze();
        vector<pair<int,int> > path(m.find_path());
        cin >> k;      
        cout << ((k == m.path_forks(path)) ? "Impressed" : "Oops!") << endl;                
    }
    return 0;
}

