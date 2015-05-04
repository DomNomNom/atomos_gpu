
/*******************************\
| A small unitlity to get both  |
| windows and linux times.      |
\*******************************/


#include <stdlib.h> // for NULL

#ifdef _WIN32
#   include <windows.h>
    static DWORD time_ofInit;
    static DWORD time_previous;
    static DWORD time_now;
#else
#   include <sys/time.h>
    static struct timeval time_ofInit;
    static struct timeval time_previous;
    static struct timeval time_now;
#endif

// Figure out time elapsed since last call
float time_dt() {
  time_previous = time_now;

#ifdef _WIN32
    time_now = GetTickCount();
    return (float) (time_now - time_previous) / 1000.0;
#else
    gettimeofday(&time_now, NULL);
    return (
        (float)(time_now.tv_sec  - time_previous.tv_sec ) +
        1.0e-6*(time_now.tv_usec - time_previous.tv_usec)
    );
#endif
}

void time_init() {
#ifdef _WIN32
    time_ofInit = GetTickCount();
    time_previous = time_ofInit;
#else
    gettimeofday (&time_ofInit, NULL);
    time_previous = time_ofInit;
#endif
    time_dt();
}



// the total time since time_init()
float time_sinceInit() {
#ifdef _WIN32
    time_now = GetTickCount();
    return (float) (time_now - time_ofInit) / 1000.0;
#else
    gettimeofday(&time_now, NULL);
    return (
        (float)(time_now.tv_sec  - time_ofInit.tv_sec ) +
        1.0e-6*(time_now.tv_usec - time_ofInit.tv_usec)
    );
#endif
}
