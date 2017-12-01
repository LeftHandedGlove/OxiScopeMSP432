
/*  ======================================== main_tirtos.c ========================================
 *  Author: David Delamater
 *  Email:  David.W.Delamater@gmail.com
 *
 *  Version 1.14
 *  "TFW there is nothing in the tube because your sampling sucks"
 */

/*!
 * @file    main_tirtos.c
 * @author  David Delamater <David.W.Delamater@gmail.com>
 * @date    23 Nov 2017
 * @brief   File that gets called when the application starts. Controls when PPGs are taken.
 *
 * The main file that gets called when the application starts. It creates the main task and starts
 * the BIOS. The main task then takes over which initializes the various peripherals used in the
 * application, including a ten minute warm up time for the lasers. Then the system hold loop
 * checks to see if photoplethysmograms (PPGs) are being taken.
 */

/* ========================================== INCLUDES ========================================= */
/* TI & third-party */
#include <stdint.h>                 // Accurate memory use for variables
#include <unistd.h>                 // usleep() and sleep()
#include <pthread.h>                // POSIX thread abstraction
#include <semaphore.h>              // POSIX semaphore abstraction
#include <mqueue.h>                 // POSIX message queue abstraction
#include <ti/sysbios/BIOS.h>        // TI BIOS functions
#include <ti/drivers/GPIO.h>        // Simplelink Family GPIO driver
#include <ti/drivers/Watchdog.h>    // Simplelink Family Watchdog Timer driver
#include <ti/drivers/ADC.h>         // Simplelink Family ADC driver
#include <ti/drivers/UART.h>        // Simplelink Family UART driver
#include <ti/drivers/Timer.h>       // Simplelink Family Timer driver
#include <ti/drivers/Power.h>       // Simplelink Family Power-Mode driver
#include <ti/display/Display.h>     // Debug through terminal message driver

/* Application Specific */
#include "Board.h"                          // Board abstraction file
#include "OxiSc_Modules/CC2650_comm.h"      // CC2650 communication module
#include "OxiSc_Modules/BattMonitor.h"      // Battery monitor module
#include "OxiSc_Modules/SpO2_PulseRate.h"   // Oxygen staturation and pulse rate module
#include "OxiSc_Modules/UserInterface.h"    // User interface module
#include "OxiSc_Modules/OxiSc_Util.h"       // Utility functions

/* ======================================= USER SETTINGS ======================================= */
/*! Stack size for the Main_Task thread. */
#define MAIN_STACKSIZE      1024
/*! Priority for the Main_Task thread. */
#define MAIN_PRIORITY       9
/*! Default time for how long the PPG measurements will be taken. */
#define DEFAULT_LAT         5
/*! WatchDog Timer enable. Don't enable this while debugging. */
//#define WDT_ENABLE

/* ======================================= DEBUG SETTINGS ====================================== */

/* ========================= APPLICATION SPECIFIC DEFINES AND TYPEDEFS ========================= */
/*! Size of the message inside the communication mailbox. */
#define MSG_SIZE sizeof(char)
/*! Maximum number of messages inside the communication mailbox. */
#define MSG_NUM  40

/* ========================================== GLOBALS ========================================== */
/*! Handle for the communication mailbox. */
mqd_t G_CommMesQue;
/*! Handle for the system hold semaphore. It stops PPG samps from being taken. */
sem_t G_SysHold;
/*! Handle for the debugging communication interface. */
Display_Handle display;
/*! How long the PPG samples will be taken in seconds. */
uint32_t G_LightAquisTime = DEFAULT_LAT;
/*! How long until the system stops taken PPG samples in seconds. */
uint32_t G_SysCount = 0;

/* ========================================= FUNCTIONS ========================================= */
/*!
 *  @brief      Constructs the main task and various BIOS objects then starts the BIOS.
 *  @params     None
 *  @return     Exit code
 */
int main(void);

/*!
 *  @brief      Controls the application.
 *  @params     *arg0   Required by TI RTOS, no idea what it is, maybe the task's handle.
 *  @return     None
 *  @note       Could be placed inside the SpO2_PulseRate Module at a later date.
 *
 *  Controls when the application takes PPG samples and resets the WDT.
 */
void *Main_Task(void *arg0);

/* ------------------------------------------- main -------------------------------------------- */
int main(void)
{
    /* Semaphore initialization */
    uint_fast16_t sem_res;
    sem_res = sem_init(&G_SysHold, 0, 0);
    if (sem_res != 0)
    {
        //Semaphore init failed
        while (1);
    }

    /* Construct the communication mailbox */
    struct mq_attr attr;
    attr.mq_maxmsg = MSG_NUM;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_flags = 0;
    G_CommMesQue = mq_open ("Communication", O_RDWR | O_CREAT, 0664, &attr);
    if (G_CommMesQue == (mqd_t)-1)
    {
      /* comm_mq open failed */
      while (1);
    }

    /* Creating tasks */
    Create_Task(MAIN_STACKSIZE, MAIN_PRIORITY, Main_Task);

    /* Let the BIOS take over */
    BIOS_start();

    return (0);
}

/* ----------------------------------------- Main_Task ----------------------------------------- */
void *Main_Task(void *arg0)
{
    /* Initialize peripherals */
    Power_init();
    Watchdog_init();
    Display_init();

    /* Open the display */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL)
    {
        /* Failed to open display driver */
        while (1);
    }

#ifdef WDT_ENABLE

    /* Watchdog Timer Setup */
    Watchdog_Params wdtparams;
    Watchdog_Handle watchdog;
    Watchdog_Params_init(&wdtparams);
    wdtparams.resetMode = Watchdog_RESET_ON;
    wdtparams.debugStallMode = Watchdog_DEBUG_STALL_ON;
    watchdog = Watchdog_open(Board_WATCHDOG, &wdtparams);
    if (watchdog == NULL)
    {
        /* Error opening WDT */
        while (1);
    }

#endif /* WDT_ENABLE */

    Display_printf(display, 0 ,0, "MSP432 Peripherals initialized");

    /* Setup OxiScope Modules */
    //User_Interface_Init();
    SpO2_PulseRate_Init();
    //CC2650_Comm_Init();
    //BattMonitor_Init();
    Display_printf(display, 0, 0, "OxiScope initialized");

    /* Reset the CC2650 to synchronize the board */
    //Reset_CC2650();

    /* Laser driver warm-up time (10 minutes) */
    GPIO_write(Board_GPIO_IRLZR, LASER_ACTIVE);
    GPIO_write(Board_GPIO_REDLZR, LASER_ACTIVE);
    //sleep(600);
    GPIO_write(Board_GPIO_REDLZR, LASER_INACTIVE);
    GPIO_write(Board_GPIO_IRLZR, LASER_INACTIVE);

    /* Loop forever to check whether the application should run */
    while(1)
    {
#ifdef WDT_ENABLE
        /* Reset the WDT so it doesn't reset everything */
        Watchdog_clear(watchdog);
#endif /* WDT_ENABLE */

        /* Debugging: Forcing the system to run or idle */
        G_SysCount = 999;

        /* System is idling */
        if (!G_SysCount)
        {
            sem_wait(&G_SysHold);
        }
        /* System is running */
        else
        {
            if (G_SysCount == G_LightAquisTime)
            {
                Display_printf(display, 0, 0, "Starting PPG sampling");
            }
            G_SysCount--;
            sem_post(&G_SysHold);
            sleep(1);
            if (!G_SysCount)
            {
                Display_printf(display, 0, 0, "Stopping PPG sampling");
            }
        }
    }
}
