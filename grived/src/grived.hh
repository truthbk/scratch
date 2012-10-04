#ifndef _GRIVED_HH
#define _GRIVED_HH

#include <sys/epoll.h>

class Grived {
    public:
        Grived(string);
        bool rescan(void);
        bool genEvents(void);
        bool getEvents(void);

    private:
        int epollfd;
        string g_dir;
        std::unordered_set<std::string> dirs;
        std::vector< int > wds;

        //should we typedef the type?
        std::map< int, boost::shared_ptr<epoll_event> > event_map;
        std::vector< epoll_event> events;
}

#endif
