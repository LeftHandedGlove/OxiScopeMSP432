/*
 * ============================ UserInterface.h ============================
 * Controls all of the user interfaces for the OxiScope application.
 * Currently that only includes a single button but may also include sound
 * or lights in later versions.
 *
 * +++++ TODO +++++
 * - Add stuff to control the LED for beta.
 */

#ifndef USERINTERFACE_H_
#define USERINTERFACE_H_

/* ================================ INCLUDES =============================== */
/* Standard Header files */
#include <stddef.h>
#include <stdint.h>

/* For usleep() */
#include <unistd.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>

/* POSIX Semaphores */
extern sem_t sysHold;

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

/* Display Header file */
#include <ti/display/Display.h>

/* Board Header file */
#include "Board.h"

/* OxiScope utility functions */
#include "OxiSc_Util.h"

/* ============================= USER SETTINGS ============================= */

/* ============================= DEBUG SETTINGS ============================ */

/* =============== APPLICATION SPECIFIC DEFINES AND TYPEDEFS =============== */
#define DB_DONTCARE_MASK    0x03FF

/* ================================ GLOBALS ================================ */
extern uint32_t G_SysCount;
extern uint32_t G_LightAquisTime;
extern sem_t G_SysHold;
extern Display_Handle display;

Timer_Handle G_BtnTimerHandle;

/* =============================== FUNCTIONS =============================== */
/* -------------------------------- UI_Init --------------------------------
 * Initializes all of the User Interface aspects.
 * Sets up the button interrupt and timer.
 */
void User_Interface_Init(void);

/* -------------------------------- ButtonCB -------------------------------
 * Disables the pin interrupt and starts the debouncing timer.
 */
void Go_Button_CallBack(uint_least8_t index);

/* ------------------------------- BtnTimerCB ------------------------------
 * Checks to see if the button has debounced by bit shifting a 16 bit
 * integer over one then adding the current button state. If the button
 * has been held down long enough then the integer should be full of 1's
 * or 0's. At that point, the button has been debounced and processed.
 */
void Button_Debounce_Timer_CallBack(Timer_Handle timer_hand);

#endif /* USERINTERFACE_H_ */
