#ifndef _GRIVED_HH
#define _GRIVED_HH

#include <sys/epoll.h>

class Grived {
    public:
        Grived(string);
	int rescan(void);
	std::queue<boost::shared_ptr<string>>& getDirs(void);
	std::vector<boost::shared_ptr<epoll_event>>& getEvents(void);
        std::vector<int>& getWds(void);
    private:
	string g_dir;
        std::queue<boost::shared_ptr<string>> dirs;
        std::vector<boost::shared_ptr<epoll_event>> events;
        std::vector<int> wds;
}

#endif
