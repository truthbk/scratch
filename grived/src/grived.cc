/*
grive_sync: AutoSync with Google Drive
*/

// boost header
#include <boost/exception/all.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <cassert>
#include <cstdlib>
#include <cstdio> 
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/epoll.h>

#define MAX_SUBDIRS 2096

using namespace std;

static volatile int shutdown;

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

void usage(int argc, char **argv) {
	cout << "Usage: " << basename(argv[0]) << "[options]" << endl;
	cout << endl << "\t-d DIR\t: GRIVE's root directory" << endl;
	cout << "\t-h\t: Displays this help message" << endl;
	return;
}

Grived::Grived(string path) :
	g_dir(path)
        , events(NULL) 
{
            //Apparently after 2.6.8 epoll_create() size argument is ignored, just has to
            //be >0.
            epollfd = epoll_create(1); 
            if ( epollfd < 0 ) 
            {
                perror( "epoll_create" );
            }

            //Exception or something....
}

epoll_event * Grived::alloc_events(unsigned int sz)
{
    if(events)
    {
        delete[] events;
    }
    events = new epoll_event[sz](); 

    return events;
}

bool Grived::rescan(void)
{

    bool new_dir = false;
    std::queue< string > scanq;

    //must now recurse and add watches to subdirs....
    //Should be refactored into class methods.
    scanq.push( g_dir );
    while(!scanq.empty()) {
        DIR *dir;
        int wd;

        struct dirent *dp;
        struct stat sb;

        if(!(dir = opendir(scanq.front().c_str()))) {
            //it was likely a file
            continue;
        }

        std::string dirstr(scanq.pop());

        //find dir in set...
        bm_t::right_const_iterator riter = wddirmap.right.find(dirstr);
        if (riter == widdirmap.right.end()) 
        {
            //new watch!
            new_dirs = true;
            wd = inotify_add_watch( inotfyfd, dirstr.c_str(),
                    IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO );
            if(wd) 
            {
                wddirmap.insert(bm_t::value_type(wd, dirstr));
            }
        }

        // breadth first search-like ;-)
        // we do this regardless in case there are new dirs within that subtree.
        while((dp = readdir(dir))) {
            //could use dp->d_type but that's not too portable.
            lstat( dp->d_name, &sb );
            if(sb.st_mode & S_IFDIR) 
            {
                dirs.push( string(dp->d_name) );
            }
        }
    }

    if(new_dirs)
    {
        if(!alloc_events(wddirmap.left.size()))
        {
            //throw exception
        }
    }

    return new_dirs;
}


int Main( int argc, char **argv )
{
    pid_t pid;
    int inotfyfd;
    int n, ret;
    int wd_idx = 0;
    int c, dflag = 0;

    string grivedir;

    sigset_t origmask;
    sigset_t mask;
    struct sigaction sa;

    while((c = getopt(argc, argv, "hd:")) != -1)
    {
        switch(c)
        {
            case 'h':
                usagge(argc, argv);
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
        if(dflag)
        {
            cout << "Specified directory doesn't exist." << endl << endl;
        }
        usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    /* Clone ourselves to make a child */  
    pid = fork(); 
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

    Grived syncer;
    syncer.rescan();

    //install signal handler to shutdown daemon.
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
        bool do_rescan = false;
        bool do_sync = false;

#define GRIVE_IOTO 2000 //couple seconds before timing out.
        epoll_event * events = syncer.getEvents();
        n = epoll_pwait( 
                syncer.getPollfd(), 
                events, 
                syncer.getEvents.size(),
                GRIVE_IOTO,
                &origmask);
        if(n < 0 && errno != EINTR) 
        {
            perror("epoll");
            goto cleanup;
        } else if (shutdown) {
            break;
        } else if(n == 0) {
            continue;
        } else {
            // TODO:
            // check the event vector, if we have a deleted directory we
            // need to clean up and deregister that from/with epoll_ctl.
            //
            // ----ADD HERE----
            for( int i= 0 ; i<n ; i++ )
            {
                //what about EPOLLIN?
                if ( (events[i].events & EPOLLERR) ||
                     (events[i].events & EPOLLHUP))
                {
                    //some sort of error... should we do anything else?
                    continue;
                }

                //resscan.
                //do_rescan = true;
                do_sync = true;
            }

            //
            //resync (doesn't matter who triggered inotify event)
            //concerned about infinite inotify triggered loop when syncing.
#ifdef _DEBUG
#define GRIVECMD "grive -v -d -l ~/.grivesync --dry-run"
#else
#define GRIVECMD "grive"
#endif
            if(do_sync)
            {
                //ugliest shit of ALL TIME. This is one ugly hack, must be implemented 
                //within grive.
                system(GRIVECMD);
            }

            if(do_rescan)
            {
                //We have to rescan for new directories, If unwatched dirs found, add watch.
                syncer.rescan();
            }
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
