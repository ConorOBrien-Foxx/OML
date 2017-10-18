// number that updates on milliseconds
#ifndef INCLUDE_MS_DELAY
#define INCLUDE_MS_DELAY
#include <inttypes.h>
#include <time.h>
#include <limits.h>
#ifdef __WIN32
    #include <windows.h>
    #define M_OS_WINDOWS
#else
    #include <sys/time.h>
    #define M_OS_SANE
#endif
int ms_delay(void) {
    uint64_t res;
    #ifdef M_OS_WINDOWS
        SYSTEMTIME time;
        GetSystemTime(&time);
        res = (time.wSecond * 1000) + time.wMilliseconds;
    #endif
    #ifdef M_OS_SANE
        struct timeval te;
        gettimeofday(&te, NULL);
        res = te.tv_sec * 1000 + te.tv_usec / 1000;
    #endif
    return res % INT_MAX;
}
#endif
