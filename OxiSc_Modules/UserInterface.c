
#include "UserInterface.h"

/* ----------------------------------- User_Interface_Init ------------------------------------- */
void User_Interface_Init(void)
{
    /* Setup start button */
    GPIO_setCallback(Board_GPIO_STARTBTN, Go_Button_CallBack);
    GPIO_enableInt(Board_GPIO_STARTBTN);

    /* Setup debounce timer for buttons */
    Timer_Params    timer_params;
    Timer_Params_init(&timer_params);
    timer_params.periodUnits = Timer_PERIOD_US;
    timer_params.period = 5000;
    timer_params.timerMode  = Timer_CONTINUOUS_CALLBACK;
    timer_params.timerCallback = Button_Debounce_Timer_CallBack;
    G_BtnTimerHandle = Timer_open(Board_TIMER_DEBOUNCE, &timer_params);
    if (G_BtnTimerHandle == NULL)
    {
        // Timer_open() failed
        while (1);
    }

    Display_printf(display, 0, 0, "User interface initialized");
}

/* ------------------------------------ Go_Button_CallBack ------------------------------------- */
void Go_Button_CallBack(uint_least8_t index)
{
    /* Disable button interrupt */
    GPIO_disableInt(index);

    /* Start debounce timer */
    Timer_start(G_BtnTimerHandle);
}

/* ------------------------------ Button_Debounce_Timer_CallBack ------------------------------- */
void Button_Debounce_Timer_CallBack(Timer_Handle timer_hand)
{
    /* Used for exiting */
    static uint8_t exitCount = 0;

    /* Masks used for debouncing */
    static uint16_t startBtnMask = 0xAA;

    /* Check button's state */
    startBtnMask = (startBtnMask << 1) | GPIO_read(Board_GPIO_STARTBTN);

    /* Check to see if button has debounced */
    startBtnMask = startBtnMask & DB_DONTCARE_MASK;
    if (startBtnMask == 0x0000)
    {
        /* Reset counters */
        startBtnMask = 0xAA;
        exitCount = 0;

        /* Stop timer */
        Timer_stop(G_BtnTimerHandle);

        /* Enable button interrupt */
        GPIO_enableInt(Board_GPIO_STARTBTN);

        /* Free the program to take samples */
        G_SysCount = G_LightAquisTime;
        sem_post(&G_SysHold);
    }

    /* Exit in case button wasn't actually pressed */
    exitCount++;
    if (exitCount >= 10)
    {
        /* Reset counters */
        startBtnMask = 0xAA;
        exitCount = 0;

        /* Stop timer */
        Timer_stop(G_BtnTimerHandle);

        /* Enable button interrupt */
        GPIO_enableInt(Board_GPIO_STARTBTN);
    }
}
