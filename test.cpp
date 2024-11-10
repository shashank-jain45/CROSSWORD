#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(void)
{
    int x = 10, tid;
    printf("x value outside parallel: %d\n", x);

#pragma omp parallel num_threads(4) shared(x) private(tid)
    {
        int tid = omp_get_thread_num();
        printf("\n 1. Thread [%d] value of x is %d \n", tid, x);
        x = 15;
#pragma omp critical
        {
            printf("\n 2. Thread [%d] value of x is %d \n", tid, x);
            x = x + 1;
            printf("\n 3. Thread [%d] value of x is %d \n", tid, x);
        }
    }
    return 0;
}
