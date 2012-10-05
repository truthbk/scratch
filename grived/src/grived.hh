#ifndef _GRIVED_HH
#define _GRIVED_HH

#include <boost/bimap.hpp>

extern "C" {
        #include <sys/epoll.h>
}

class Grived {
    public:
        Grived(string);
        bool rescan(void);

        epoll_event * getEvents(void);

    private:
        int epollfd;
        string g_dir;

        typedef boost::bimap<int, string> bm_t;
        bm_t wddirmap;
        epoll_event * events;
}

#endif
