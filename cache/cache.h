#include <unordered_map>
#include <list>

//We could also generalize the key, std::string for now.
template <typename T>
class Cache {
    public:
        void Cache(int n);
        bool get_cache(std::string s, T& t);
        bool put_cache(std::string s, T& t);
        bool get_cache_copy(std::string s, T& t);

    protected:
        struct Cachable {
            public:
                std::string key;
                T el;

                Cachable(std::string s, T t) 
                    : key(s) 
                    , el(t) {  };
        };
        virtual bool touch(std::string s);
        virtual bool touch(cache_list::iterator it);
        virtual bool evict();
        bool cache_full();
        bool has_key(std::string s);

    private:
        const uint32_t sz;

        typedef std::list<Cachable> cache_list;
        cache_list cache;
        std::unordered_map<std::string, cache_list::iterator> cache_hm;
}
