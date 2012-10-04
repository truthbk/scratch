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

bool Grived::rescan(void)
{

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
        std::unordered_set<std::string>::const_iterator got = dirs.find (dirstr);
        if (got == dirs.end()) 
        {
            //new watch!
            wd = inotify_add_watch( inotfyfd, dirstr.c_str(),
                    IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO );
            if(wd) 
            {
                wds.push_back(wd);
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
}

bool Grived::genEvents(void)
{
    std::map<int, boost::shared_ptr<epoll_event> >::const_iterator cit;

    for (int i=0 ; i<wds.size() ; i++)
    {
        int wd = wds.at(i);
        cit = event_map.find(wd);
        //event not created...
        if(cit == event_map.end())
        {
            boost::shared_ptr<epoll_event> ev_ptr(new epoll_event);
            ev_ptr->events = EPOLLIN; //no need to be edge triggered, right?
            ev_ptr->data.fd = wd;
            epoll_ctl( epollfd, EPOLL_CTL_ADD, wd, ev_ptr );
            event_map.insert(
                    std::pair<int, boost::shared_ptr<epoll_event> >( wd, ev_ptr ));

            events.insert(*ev_ptr); //this copies, but it shouldn't be a problem.

        }
    }
}

int Main( int argc, char **argv )
{
    pid_t pid;
    int inotfyfd;
    int ret;
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
    syncer.genEvents();

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
#define GRIVE_IOTO 2000 //couple seconds before timing out.
        ret = epoll_pwait( 
                syncer.getPollfd(), 
                &(syncer.getEvents()[0]), 
                syncer.getEvents.size(),
                GRIVE_IOTO,
                &origmask);
        if(ret < 0 && errno != EINTR) 
        {
            perror("epoll");
            goto cleanup;
        } else if (shutdown) {
            break;
        } else if(ret == 0) {
            continue;
        } else {
            // TODO:
            // check the event vector, if we have a deleted directory we
            // need to clean up and deregister that from/with epoll_ctl.
            //
            // ----ADD HERE----

            //
            //resync (doesn't matter who triggered inotify event)
            //concerned about infinite inotify triggered loop when syncing.
#ifdef _DEBUG
#define GRIVECMD "grive -v -d -l ~/.grivesync --dry-run"
#else
#define GRIVECMD "grive"
#endif
            //ugliest shit of ALL TIME. This is one ugly hack, must be implemented 
	    //within grive.
            system(GRIVECMD);

	    //We have to rescan for new directories, If unwatched dirs found, add watch.
            syncer.rescan();
            syncer.genEvents();
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
