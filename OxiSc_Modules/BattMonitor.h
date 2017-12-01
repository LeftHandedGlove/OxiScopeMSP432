

/*  ======================================== BattMonitor.h ========================================
 *  Author: David Delamater
 *  Email:  David.W.Delamater@gmail.com
 */

/*!
 * @file    BattMonitor.h
 * @author  David Delamater <David.W.Delamater@gmail.com>
 * @date    23 Nov 2017
 * @brief   Monitors the battery by taking samples and placing them on a best fit line.\
 *
 * Takes battery samples from the voltage divider circuit over a period of time. These samples are
 * averaged and placed on a best fit line in order to find the correct battery level. If the level
 * is new then it gets sent to the communication task via the communication mailbox.
 */

/*  +++++++++ TODO ++++++++++
 *  - Monitor the way the battery discharges to graph the discharge curve
 */

#ifndef BATTMONITOR_H_
#define BATTMONITOR_H_

/* ========================================= INCLUDES ========================================== */
/* TI & third-party */
#include <stdint.h>                 // Accurate memory use for variables
#include <unistd.h>                 // usleep() and sleep()
#include <pthread.h>                // POSIX thread abstraction
#include <semaphore.h>              // POSIX semaphore abstraction
#include <mqueue.h>                 // POSIX message queue abstraction
#include <ti/drivers/ADC.h>         // Simplelink Family ADC driver
#include <ti/drivers/GPIO.h>        // Simplelink Family GPIO driver
#include <ti/display/Display.h>     // Debug through terminal message driver

/* Application Specific */
#include "Board.h"                      // Board abstraction file
#include "OxiSc_Modules/OxiSc_Util.h"   // Utility functions

/* ======================================= USER SETTINGS ======================================= */
/*
 *  VBAT-----VVVV----------VVVV-----
 *            R1     |      R2     |
 *                   |             |
 *                  ADC           GND
 */
/*! The first resistor in the voltage divider circuit in ohms. */
#define BATT_RESISTOR_1     155000
/*! The second resistor in the voltage divider circuit in ohms. */
#define BATT_RESISTOR_2     100000        // R2 in the diagram above in ohms
/*! ADC channel from the board.h file. */
#define BATT_ADC_CHANNEL    Board_ADC_BATTERY
/*! ADC reference voltage. */
#define BATT_VREF           3.3
/*! Max reading for the ADC (14 bits = 16383) */
#define BATT_ADC_MAX        16383
/*! Voltage of the battery at full charge. */
#define BATT_FULL           7.2
/*! Voltage of the battery at low charge. */
#define BATT_LOW            6.2
/*! Time between reported battery readings in seconds. */
#define BATT_SAMP_PERIOD    2
/*! Number of samples per reported reading. */
#define BATT_SAMP_SIZE      50
/*! What increment the battery level will be reported in. (5 = 00.5%) */
#define BATT_HYSTERESIS     5
/*! Priority of the battery monitor task, it is not critical so it can be low. */
#define BATTMON_PRIORITY        1
/*! Stack size of the battery monitor task, it can be small, 1024 performs well. */
#define BATTMON_STACKSIZE       1024

/* ======================================= DEBUG SETTINGS ====================================== */
/*! Controls whether the battery level is being simulated. */
//#define BATTERY_MONITOR_SIMULATION

/* ========================================== GLOBALS ========================================== */
/*! Handle for the communication mailbox. */
extern mqd_t G_CommMesQue;
/*! Handle for the debugging communication interface. */
extern Display_Handle display;
/*! Battery level in 000 = 00.0% format */
uint16_t G_BattLevel;

/* ========================================= FUNCTIONS ========================================= */
/*!
 *  @brief      Initializes the battery monitoring module.
 *  @params     None
 *  @return     None
 *  @note       Only call this once, otherwise the program will crash.
 *
 *  Initializes the battery monitoring module by initializing the required ADC peripheral and
 *  creating the monitoring task.
 */
void BattMonitor_Init(void);

/*!
 *  @brief      Monitors the battery by taking samples and applying them to a best fit line.
 *  @params     *arg0   Required by TI RTOS, no idea what it is, maybe the task's handle
 *  @return     None
 *
 *  NORMAL OPERATION
 *  Takes battery samples from the voltage divider circuit over a period of time. These samples
 *  are averaged and placed on a best fit line in order to find the correct battery level. If
 *  the level is new then it gets sent to the communication task via the communication mailbox.
 *  SIMULATION
 *  Reports a slowly decreasing level every reporting period.
 */
void *Battery_Monitor_Task(void *arg0);

#endif /* BATTMONITOR_H_ */
