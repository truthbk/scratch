/*
grive_sync: AutoSync with Google Drive
*/

// boost header
#include <boost/exception/all.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/epoll.h>

#define MAX_SUBDIRS 2096

using namespace std;

static int shutdown;

static void sig_handler(int signo) {
    switch(signo)
    {
        case SIGINT:
        case SIGTERM:
            shutdown = 1;
            break;
        default:
            break;
    }
}

int Main( int argc, char **argv )
{
    pid_t pid;
    int inotfyfd, epollfd;
    int ret;
    int wd_idx = 0;
    int c, dflag = 0;

    string grivedir;
    std::queue<boost::shared_ptr<string>> dirs;
    std::vector<int> wds;

    struct epoll_event event;
    std::vector<shared_ptr<epoll_event>> events;

    sigset_t origmask;
    sigset_t mask;
    struct sigaction sa;

    while((c = getopt(argc, argv, "hd:")) != -1)
    {
        switch(c)
        {
            case 'h':
                break;
            case 'd':
                dflag = 1;
                grivedir = optarg ;
                break;
        }
    }

    if(!dflag || !boost::filesystem::exists(grivedir))
    {
        //show help
        exit(EXIT_FAILURE);
    }

    /* Clone ourselves to make a child */  
    pid = fork(); 

    /* If the pid is less than zero,
     * something went wrong when forking */
    if (pid < 0) 
    {
        exit(EXIT_FAILURE);
    }

    /* parent. */
    if (pid > 0) 
    {
        exit(EXIT_SUCCESS);
    }

    /* child */
    /* Set the umask to zero */
    umask(0);

    pid_t sid;

    /* Try to create our own process group */
    sid = setsid();
    if (sid < 0) 
    {
        syslog(LOG_ERR, "Could not create process group\n");
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir(grivedir.c_str())) < 0) 
    {
        syslog(LOG_ERR, "Could not change working directory to: %s\n", grivedir.c_str());
        exit(EXIT_FAILURE);
    }

    /* Close the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* daemon logic */
    inotfyfd = inotify_init();
    if ( inotfyfd < 0 ) 
    {
        perror( "inotify_init" );
    }

    //must not recurse and add watches to subdirs....
    dirs.push( boost::shared_ptr<string>(new string(argv[1])) );
    while(!myqueue.empty()) {
        DIR *dir;
        int wd;

        struct dirent *dp;
        struct stat sb;

        if(!(dir = opendir(myqueue.front().c_str()))) {
            //it was likely a file
            continue;
        }

        wd = inotify_add_watch( inotfyfd, myqueue.front().c_str(),
                IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO );
        if(wd) 
        {
            wds.push_back(wd);
        }

        while((dp = readdir(dir))) {
            //could use dp->d_type but that's not too portable.
            lstat( dp->d_name, &sb );
            if(sb.st_mode & S_IFDIR) 
            {
                dirs.push( boost::shared_ptr<string>(new string(dp->d_name)) );
            }
        }
    }

    epollfd = epoll_create(wds.size()); 
    if ( epollfd < 0 ) 
    {
        perror( "epoll_create" );
        ret = errno;
        goto cleanup;
    }

    event.events = EPOLLIN; //no need to be edge triggered, right?
    typedef std::vector<int>::const_iterator vit;
    for( vit it = wds.begin() ; it != wds.end() ; ++it )
    {
        event.data.fd = *it;
        epoll_ctl( epollfd, EPOLL_CTL_ADD, *it, &event );
        boost::shared_ptr<epoll_event> ev_ptr(new epoll_event);
        events.push_back(*ev_ptr);
    }

    memset( &sa, 0, sizeof(struct sigaction) );
    sa.sa_handler = sig_handler;
    sa.sa_flags = 0;
    if ( sigaction( SIGTERM, &sa, 0 ))
    {
        perror("sigaction");
        ret = errno;
        goto cleanup;
    }
    if ( sigaction( SIGINT, &sa, 0 ))
    {
        perror("sigaction");
        ret = errno;
        goto cleanup;
    }

    //mask signals...
    sigemptyset(&mask);
    sigaddset( &mask, SIGINT );
    sigaddset( &mask, SIGTERM );

    if( sigprocmask(SIG_BLOCK, &mask, &origmask) < 0 )
    {
        perror("sigprocmask");
        ret = errno;
        goto cleanup;
    }

    //we're now watching the entire gdrive subtree.
    while(!shutdown) {
#define GRIVE_IOTO 2000 //couple seconds before timing out.
        ret = epoll_pwait(epollfd, &events[0], event.size(), GRIVE_IOTO, &origmask);
        if(ret < 0 && errno != EINTR) 
        {
            perror("epoll");
            goto cleanup;
        } else if (shutdown) {
            break;
        } else if(ret == 0) {
            continue;
        } else {
            //resync (doesn't matter who triggered inotify event)
            //concerned about infinite inotify triggered loop when syncing.
#ifdef _DEBUG
#define GRIVECMD "grive -v -d -l ~/.grivesync --dry-run"
#else
#define GRIVECMD "grive"
#endif
            system(GRIVECMD);
        }
    }
    ret = EXIT_SUCCESS;

cleanup:
    while(!events.empty()) 
    {
        // boost::shared_ptr<epoll_event> aux;
        // aux = events.back();

        events.pop_back();
    }
    //in theory these two could be wrapped into one loop
    //because they should contain the same number of
    //entries....
    while(!dirs.empty()) 
    {
        dirs.pop();
    }
    while(!wds.empty())
    {
        int wd = wds.back();
        wds.pop_back();
        inotify_rm_watch(inotifyfd, wd);
    }

    close(inotfyfd);

    exit(ret);
}

int main( int argc, char **argv )
{
    try
    {
        return Main( argc, argv ) ;
    }
    catch ( Exception& e )
    {
        Log( "exception: %1%", boost::diagnostic_information(e), log::critical ) ;
    }
    catch ( std::exception& e )
    {
        Log( "exception: %1%", e.what(), log::critical ) ;
    }
    catch ( ... )
    {
        Log( "unexpected exception", log::critical ) ;
    }
    return -1 ;
}
