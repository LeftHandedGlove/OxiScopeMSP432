
/*  ======================================== CC2650_comm.h ========================================
 *  Author: David Delamater
 *  Email:  David.W.Delamater@gmail.com
 */

/*!
 * @file    CC2650_comm.c
 * @author  David Delamater <David.W.Delamater@gmail.com>
 * @date    23 Nov 2017
 * @brief   Handles sending and receiving messages as well as maintaining the link.
 *
 * Controls the UART interface between the MSP432 and the CC2650. Takes data from various tasks
 * and sends the appropriate message. Reads data from the receive line and performs actions based
 * on those messages. Also monitors the communication line and attempts to fix issues by resetting
 * the CC2650 to reestablish a link.
 */

/*
 *  +++++++++ TODO ++++++++++
 *  - Fix UART timeout issue where it will occasionally not work
 *  - Test resetting the CC2650
 */

#ifndef UART_COMM_H_
#define UART_COMM_H_

/* ========================================== INCLUDES ========================================= */
/* TI & third-party */
#include <stdint.h>     // Accurate memory use for variables
#include <stdio.h>      // for sprintf() to convert numbers to strings
#include <string.h>     // Makes manipulating strings easier
#include <stdbool.h>    // For boolean operators
#include <unistd.h>     // usleep() and sleep()
#include <pthread.h>    // POSIX thread abstraction
#include <semaphore.h>  // POSIX semaphore abstraction
#include <mqueue.h>     // POSIX message queue abstraction
#include <ti/drivers/GPIO.h>    // Simplelink Family GPIO driver
#include <ti/drivers/UART.h>    // Simplelink Family UART driver
#include <ti/drivers/Timer.h>   // Simplelink Family Timer driver
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>  // For CRC32 Hdw drivers
#include <ti/sysbios/knl/Semaphore.h>   // Used for TI RTOS timed semaphores
#include <ti/sysbios/knl/Clock.h>   // Used for TI RTOS timed semaphores
#include <ti/display/Display.h>     // Debug through terminal message driver

/* Application Specific */
#include "Board.h"  // Board abstraction file
#include "OxiSc_Modules/OxiSc_Util.h"   // Utility functions

/* ======================================= USER SETTINGS ======================================= */
/*! Seed used to initialize the CRC32 when calculating a new one. */
#define CRC32_SEED              0xFFFFFFFF
/*! Timeout in mircoseconds when waiting for acknowledges. */
#define UART_TIMEOUT            500000
/*! Waiting time in microseconds between checking for connection after resetting CC2650. */
#define RESET_WAIT_TIME         20000000
/*! Priority of the UART RX task, important so messages are processed before the next message. */
#define UART_RX_PRIORITY        8
/*! Priority of the UART TX task, somewhat important so the mailbox doesn't fill up. */
#define UART_TX_PRIORITY        7
/*! Stack size of the UART RX task, 1024 is fine. */
#define UART_RX_STACKSIZE       1024
/*! Stack size of the UART TX task, 1024 is fine. */
#define UART_TX_STACKSIZE       1024
/*! How many times a timeout or NACK can happen before the CC2650 is reset. */
#define RESET_COUNTER_LIMIT     3

/* ======================================= DEBUG SETTINGS ====================================== */

/* ========================= APPLICATION SPECIFIC DEFINES AND TYPEDEFS ========================= */

/* ========================================== GLOBALS ========================================== */
/*! UART interface handle */
UART_Handle G_Uart_CC2650;
/*! Reset counter for keeping track of when the CC2650 should be reset. */
uint8_t G_ResetCount;
/*! Buffer for the most recent TX message. */
char G_UartTxBuffer[8];
/*! Buffer for the most recent RX message. */
char G_UartRxBuffer[8];
/*! How long until the system stops taken PPG samples in seconds. */
extern uint32_t G_SysCount;
/*! How long the PPG samples will be taken in seconds. */
extern uint32_t G_LightAquisTime;
/*! Oxygen saturation in 000 = 00.0% format. */
extern uint16_t G_SpO2;
/*! Pulse rate in 000 = 000 BPM format. */
extern uint16_t G_PulseRate;
/*! Battery level in 000 = 00.0% format. */
extern uint16_t G_BattLevel;
/*! Handle for the debugging communication interface. */
extern Display_Handle display;
/*! Timer handle for the CC2650 reset timer. */
Timer_Handle G_ResetTimerHandle;
/*! Whether the CC2650 was recently reset. */
bool G_ResettingCC2650;
/*! Handle for the system hold semaphore. It stops PPG samps from being taken. */
extern sem_t G_SysHold;
/*! Handle for the communication mailbox. */
extern mqd_t G_CommMesQue;
/*! Handle for the UART acknowledge semaphore. */
sem_t G_UartAck;

/* ========================================= FUNCTIONS ========================================= */
/*!
 *  @brief      Encrypts a message using ROT13.
 *  @params     array   Message to be encrypted.
 *  @params     length  Length of the message.
 *  @return     None
 *
 *  Encrypts the message by adding 13 to every character in the array.
 */
void ROT13_Encrypt(char array[], size_t length);

/*!
 *  @brief      Decrypts a message using ROT13.
 *  @params     array   Message to be decrypted.
 *  @params     length  Length of the message.
 *  @return     None
 *
 *  Decrypts a message by subtracting 13 from every character in the array.
 */
void ROT13_Decrypt(char array[], size_t length);

/*!
 *  @brief      Calculates the CRC32 of a message.
 *  @params     array   Message used to calculated the CRC32.
 *  @params     length  Length of the array.
 *  @return     CRC32 calculation.
 *
 *  Calculates the CRC32 by initializing the CRC32 hardware with a seed. Then stuffs the message
 *  into the hardware least significant bit first. Afterwards, it retrieves the bit reversed
 *  result and XORs it with all ones and returns it.
 */
uint32_t Calc_CRC32(char array[], size_t length);

/*!
 *  @brief      Checks to see if a connection was established.
 *  @params     time_hand   Timer handle for the timer that caused the callback.
 *  @return     none
 *
 *  The callback for the CC2650 reset timer. It checks to see if a connection was established.
 *  If it was then every possible data message is sent. Otherwise, nothing happens.
 */
void CC2650_Reset_Timer_CallBack(Timer_Handle time_hand);

/*!
 *  @brief      Resets the CC2650.
 *  @params     None
 *  @return     None
 *  @note       Does nothing if the CC2650 was recently reset
 *
 *  Resets the CC2650 is it wasn't recently reset. The reset timer is started and will go
 *  to its specfic callback.
 */
void Reset_CC2650(void);

/*!
 *  @brief      Initializes the CC2650 communication module
 *  @params     None
 *  @return     None
 *  @note       Only call this one, otherwise the program will crash.
 */
void CC2650_Comm_Init(void);

/*!
*  @brief
 *  @params
 *  @return
 *  @note
 */
void *Uart_Rx_Task(void *arg0);

/*!
 *  @brief
 *  @params
 *  @return
 *  @note
 */
void *Uart_Tx_Task(void *arg0);

#endif /* UART_COMM_H_ */
