/*
 *  ======== SpO2_PulseRate.h ========
 *  Takes light samples, averages them, finds the PPG waveform's
 *  voltage peak-to-peak and frequency for both frequencies,
 *  then uses those to calculate the oxygen saturation and pulse rate.
 *
 *  +++++++++ TODO ++++++++++
 *
 */

#ifndef SPO2_PULSERATE_H_
#define SPO2_PULSERATE_H_

/* ================================ INCLUDES =============================== */
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

/* TI Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/ADC.h>
#include <ti/drivers/Timer.h>

/* Display Header file */
#include <ti/display/Display.h>

/* Board Header file */
#include "Board.h"

/* OxiScope utility functions */
#include "OxiSc_Util.h"

/* ============================= USER SETTINGS ============================= */
/* Frequency of PPG samples */
#define LIGHT_SAMP_RATE         250
/* How many seconds of samples the waveform calculation task will look at */
#define WINDOW_PERIOD           3
/* How many samples are required for the calculation task to recalculate vpp and freq */
#define RECALC_SAMPLES          250
/* Number of samples in a window */
#define WINDOW_SIZE             LIGHT_SAMP_RATE * WINDOW_PERIOD
/* Controls the top of the don't care zone when finding maximums */
#define MAX_WEIGHT      0.7
/* Controls the bottom of the don't care zone when finding minimums */
#define MIN_WEIGHT      0.3

#define AC_FIR_NUM_TAPS         251
#define DC_FIR_NUM_TAPS         501
#define RED_AC_GAIN         5
#define IR_AC_GAIN         5
#define AC_OFFSET       7000

#define PPG_SAMPING_PRIORITY     10
#define PPG_PROCESS_PRIORITY     6
#define PPG_CALC_PRIORITY        5
#define PPG_SAMPING_STACKSIZE    1024
#define PPG_PROCESS_STACKSIZE    16384
#define PPG_CALC_STACKSIZE       8192
/* ============================= DEBUG SETTINGS ============================ */

/* =============== APPLICATION SPECIFIC DEFINES AND TYPEDEFS =============== */
/* Light Attributes Struct */
typedef struct LightAttr
{
    uint16_t rawACSample;
    uint16_t rawDCSample;
    float AC_DC_Inputs[AC_FIR_NUM_TAPS];
    float AC_AC_Inputs[AC_FIR_NUM_TAPS];
    float DC_Inputs[DC_FIR_NUM_TAPS];
    uint16_t ACFilteredSample;
    uint16_t DCFilteredSample;
    uint16_t windowBuffer[RECALC_SAMPLES];
    uint16_t ACFilteredSamples[WINDOW_SIZE];
    uint16_t vpp;
    float freq;
} LightAttr;

/* ================================ GLOBALS ================================ */
/* POSIX Semaphores */
extern sem_t G_SysHold;
sem_t G_PPGSamp;
sem_t G_PPGProcess;

/* POSIX Message Queues */
extern mqd_t G_CommMesQue;

extern Display_Handle display;

uint16_t G_SpO2;
uint16_t G_PulseRate;

LightAttr G_Red;
LightAttr G_Ir;

/* =============================== FUNCTIONS =============================== */
void SpO2_PulseRate_Init(void);
void *PPG_Sampling_Task(void *arg0);
float OxiScope_FIR_Filter(uint16_t input, float inputs[], const float coeffs[], uint16_t numTaps);
void *PPG_Processing_Task(void *arg0);
void Find_Vpp_Freq(LightAttr *light);
void *PPG_Calculate_Task(void *arg0);


#endif /* SPO2_PULSERATE_H_ */
