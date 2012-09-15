#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <libgen.h>
#include <sched.h>

#ifndef _SPINWAIT
#include <unistd.h>
#endif

volatile int stop = 0;

void signal_handler(int signum)
{
  switch(signum)
  {
    case SIGINT:
      stop = 1;
      break;
    default:
      break;
  }
}


int timeval_subtract (
    struct timeval *result,
    struct timeval *x, 
    struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
   *           tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;

}

void usage(char * name) {
  fprintf(stdout,"usage: %s [-l N] [-n N]\n", basename(name));
  fprintf(stdout,"\n\t-y : Yield CPU instead of sleep. This might not help at all regarding cpu load throttling..");
#ifdef _RANDOMIZE
  fprintf(stdout,"\n\t-r N : Randomize CPU load. N is a factor (0.0-1.0)");
#endif
  fprintf(stdout,"\n\t-l N : Approx. CPU load desired (as a whole number percentage) [0-100].");
  fprintf(stdout,"\n\t-n N : Safety mechanism so the \"infinite\" loop doesn't go on forever.\n");
  return;
}

int main (int argc, char **argv) 
{
  int c;
  uint8_t yield = 0;
#define LOADDEF 100
  uint32_t j;
  uint32_t load = LOADDEF;

#ifdef _RANDOMIZE
  float randomize = 0.0f;
  uint32_t r;
  uint32_t ef_load = LOADDEF;
#endif

#define U_DEFAULT = 1000
  uint64_t counter = 0;
  uint64_t count_max = ULONG_MAX;
  
  struct timeval tv, now, diff;

#ifndef _RANDOMIZE
  while((c = getopt (argc, argv, "hyl:n:")) != -1)
#else
  while((c = getopt (argc, argv, "hyl:n:r:")) != -1)
#endif
  {
    switch(c)
    {
      case 'y':
        yield = 1;
        break;
#ifdef _RANDOMIZE
      case 'r':
        randomize = atof(optarg);
        if( randomize < 0.0f || randomize > 1.0f )
        {
          usage(argv[0]);
          exit(1);
        }
        break;
#endif
      case 'l':
        load = atoi(optarg);
        if( !load || load > 100 ) //unsigned, no need to check negatives...
        {
          usage(argv[0]);
          exit(1);
        }
        break;
      case 'n':
        count_max = atoll(optarg);
        break;
      case 'h':
      case '?':
      default:
          usage(argv[0]);
          exit(1);
          break;
    }
  }

  //install handler
  signal(SIGINT, signal_handler);

  srand(time(NULL));

  //infinite looooooop
  while( !stop && counter < count_max ) //so we don't go on forever... 
  {

    int cont = 1;
    gettimeofday(&tv, NULL);

#ifdef _RANDOMIZE
    if(randomize != 0.0f)
    {
      r = rand();
      if(r%2) 
        ef_load = load + ((float)r/RAND_MAX)*randomize*100;
      else
        ef_load = load - ((float)r/RAND_MAX)*randomize*100;

      if(ef_load > 100)
        ef_load = 100;
      else if (ef_load < 0)
        ef_load = 0;
    }
#endif
    while(cont)
    {
      sqrt(rand()); //load
      gettimeofday(&now, NULL);
      counter++;

#ifndef _RANDOMIZE
      if(load < 100)
#else
      if(ef_load < 100)
#endif
      {
        timeval_subtract( &diff, &now, &tv);
#ifndef _RANDOMIZE
        if(diff.tv_usec > load*10)
#else
        if(diff.tv_usec > ef_load*10)
#endif
        {
          if(!yield)
          {
            usleep((100-load)*10);
            cont = 0;
          } 
          else 
          {
            while(1)
            {
              gettimeofday(&now, NULL);
              timeval_subtract( &diff, &now, &tv);
              if(diff.tv_usec < 1000)
                sched_yield();
              else
              {
                cont = 0;
                break;
              }
            }
          }
        }
      }
    }
  }

  fprintf(stdout, "\nCounter: %ld\n", counter);
  return EXIT_SUCCESS;
}
