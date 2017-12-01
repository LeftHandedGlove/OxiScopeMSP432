
#include "OxiSc_Util.h"

/* --------------------------------------- Create_Task ----------------------------------------- */
void Create_Task(uint32_t stackSize, uint32_t priority, void*(*taskName)(void*))
{
    pthread_t           thread;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&pAttrs, detachState);

    pthread_attr_init(&pAttrs);
    priParam.sched_priority = priority;
    pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, stackSize);

    retc |= pthread_create(&thread, &pAttrs, taskName, NULL);
    if (retc != 0)
    {
        /* Failed to create thread */
        while (1);
    }
}

/* ------------------------------------------ Merge -------------------------------------------- */
void Merge(uint16_t arr[], uint16_t l, uint16_t m, uint16_t r)
{
    uint16_t i, j, k;
    uint16_t n1 = m - l + 1;
    uint16_t n2 =  r - m;

    /* create temp arrays */
    uint16_t L[n1], R[n2];

    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];

    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    /* Copy the remaining elements of L[], if there
       are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    /* Copy the remaining elements of R[], if there
       are any */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

/* ---------------------------------------- Merge_Sort ----------------------------------------- */
void Merge_Sort(uint16_t arr[], uint16_t l, uint16_t r)
{
    if (l < r)
    {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l+(r-l)/2;

        // Sort first and second halves
        Merge_Sort(arr, l, m);
        Merge_Sort(arr, m+1, r);

        Merge(arr, l, m, r);
    }
}

/* -------------------------------------- Box_Whisk_Avg ---------------------------------------- */
uint32_t Box_Whisk_Avg(uint16_t array[], uint16_t size)
{
    uint32_t sum = 0;
    uint16_t i;

    //Sort the array
    Merge_Sort(array, 0, (size - 1));

    //Take the average of the two middle quartiles
    for(i = 0; i < (size / 2); i++)
    {
        sum += array[(size / 4 ) + i];
    }
    return (uint32_t)(sum / (size / 2));
}
