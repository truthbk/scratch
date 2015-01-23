#ifndef _CACHE_H
#define _CACHE_H

#include <unordered_map>
#include <list>

//We could also generalize the key, std::string for now.
template <typename S = std::string,typename T>
class Cache {
    public:
        //constructor
        explicit Cache<S,T>(int n)
            : sz(n) { //empty 
        }

        bool get_cache(S s, T& t) {
            bool exists = has_key(s);
            if(!exists) {
                return false;
            }
            cache_list::iterator it(hash_hm[s]);
            t = *it;
            return true;
        }
        bool put_cache(S s, T& t) {
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

        T get_cache_copy(S s) {
            T cached;

            if(get_cache(s, cached)) {
                return cached;

            }
            //throw an exception or something...
            //think this over.
        }

    protected:
        struct Cachable {
            public:
                S key;
                T el;

                Cachable(S s, T t) 
                    : key(s) 
                    , el(t) {  };
        }

        //standard should be LRU
        bool touch(S s) {
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
        bool evict() {
            if(cache.empty()) {
                return false;
            }

            Cachable c(cache.back());
            cache.pop_back(); 
            cache_hm.erase(c.key); // Gotta fix this.
            return true;
        }
        bool cache_full(){
            return (cache.size()==sz);
        }
        bool has_key(S s){
            return (cache_hm.find(s) != cache_hm.end());
        }

    private:
        const uint32_t sz;

        typedef std::list<Cachable> cache_list;
        cache_list cache;
        std::unordered_map<S, cache_list::iterator> cache_hm;
}

#endif //_CACHE_H
