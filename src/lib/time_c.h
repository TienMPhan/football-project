#ifndef TIME_C_H
#define TIME_C_H

#include <sys/time.h>

double getTimeDifferenceInMilliseconds(struct timeval *t1, struct timeval *t2)
{
    return ((t2->tv_sec - t1->tv_sec) * 1000.0) + ((t2->tv_usec - t1->tv_usec) / 1000.0); // (sec to ms) + (us to ms)
}

#endif

/* Example use:
int main()
{
    int i, j = 0;
    double elapsedTime;

    gettimeofday(&t1, NULL);

    for (i = 0; i < 100000; i++)
    {
        j += 3; // do something
    }

    gettimeofday(&t2, NULL);

    elapsedTime = getTimeDifferenceInMilliseconds(&t1, &t2);
    printf("%f", elapsedTime);
}
*/