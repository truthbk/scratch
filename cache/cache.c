#include "cache.h"


void Cache::Cache(int n)
    : sz(n) {
}


bool Cache::get_cache(std::string s, T& t) {
    bool exists = has_key(s);
    if(!exists) {
        return false;
    }
    cache_list::iterator it(hash_hm[s]);
    t = *it;
    return true;
}
bool Cache::put_cache(std::string s, T& t) {
    bool exists = has_key(s);
    //refresh
    if(exists) {
        cache_list::iterator it(cache_hm[s]);
        Cachable c(s, t);
        *it = c;
        return true;
    } else {
        if(cache_full()) {
            evict();
        }
        Cachable c(s, t);
        push_front(c);
        cache_hm[s] = cache.begin();
    }
}

T Cache::get_cache_copy(std::string s) {
    T cached;

    if(get_cache(s, cached)) {
        return cached;

    }
    //throw an exception or something...
    //think this over.
}

//standard should be LRU
bool Cache::touch(std::string s) {
    bool exists =  has_key(s);
    if(!exists) {
        return false;
    }

    cache_list::iterator it(cache_hm[s]);
    Cachable c(*it);
    cache.erase(it);
    cache.push_front(c);
    cache_hm[s] = cache.begin();

    return true;
}
//standard should be LRU
bool Cache::evict() {
    if(cache.empty()) {
        return false;
    }

    Cachable c(cache.back());
    cache.pop_back(); 
    cache_hm.erase(c.key); // Gotta fix this.
    return true;
}
bool Cache::cache_full(){
    return (cache.size()==sz);
}
bool Cache::has_key(std::string s){
    return (cache_hm.find(s) != cache_hm.end());
}
