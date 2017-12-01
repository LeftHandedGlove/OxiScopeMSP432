/*
 * OxiSc_Util.h
 * TODO: description
 *
 */

#ifndef OXISC_UTIL_H_
#define OXISC_UTIL_H_


/* ========================================== INCLUDES ========================================= */
/* Standard Header files */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* For usleep() */
#include <unistd.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>

/* RTOS Header files */
#include <ti/sysbios/BIOS.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Watchdog.h>
#include <ti/drivers/ADC.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/Power.h>

/* Display Header file */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>

/* Board Header file */
#include "Board.h"



/* ======================================= USER SETTINGS ======================================= */

/* ======================================= DEBUG SETTINGS ====================================== */

/* ========================= APPLICATION SPECIFIC DEFINES AND TYPEDEFS ========================= */

/* ========================================== GLOBALS ========================================== */

/* ========================================= FUNCTIONS ========================================= */

/*
 * ======== CreateTask ========
 * Creates the various tasks for the application using POSIX APIs
 * - stackSize:  size of the task's stack
 * - priority:   task priority
 * - taskName:   pointer to task's function
 * - mailboxArg: pointer to the POSIX mailbox, NULL if task doesn't use one
 */
extern void Create_Task(uint32_t stackSize, uint32_t priority, void*(*taskName)(void*));

/*  -------------------------------- Merge ---------------------------------
 *  Merges two arrays from MergeSort into a single sorted array.
 *  Smallest to Largest
 *  -arr[]: pointer to start of arrays to be merged
 *  -l:     left index
 *  -m:     middle index
 *  -r:     right index
 */
extern void Merge(uint16_t arr[], uint16_t l, uint16_t m, uint16_t r);

/*  ------------------------------- MergeSort ------------------------------
 *  Sorts an array from smallest to largest
 *  -arr[]: pointer to array to be sorted
 *  -l:     left index
 *  -r:     right index
 */
extern void Merge_Sort(uint16_t arr[], uint16_t l, uint16_t r);

/*  ---------------------------- BoxAndWhiskAlg ----------------------------
 *  Find the value that best represents an array using
 *  the box-and-whisker averaging algorithm. It takes the
 *  two middle quartiles and averages them, this protects
 *  from outliers.
 *  -array[]:   pointer to array of values to be averaged
 *  -size:      size of the array
 *  -*result:   pointer to where the result will be saved
 */
extern uint32_t Box_Whisk_Avg(uint16_t array[], uint16_t size);

#endif /* OXISC_UTIL_H_ */
