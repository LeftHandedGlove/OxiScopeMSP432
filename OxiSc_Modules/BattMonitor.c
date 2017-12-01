
#include "BattMonitor.h"

/* ------------------------------------- BattMonitor_Init -------------------------------------- */
void BattMonitor_Init(void)
{
    /* Initialize peripherals */
    ADC_init();

    /* Create task */
    Create_Task(BATTMON_STACKSIZE, BATTMON_PRIORITY, Battery_Monitor_Task);

    Display_printf(display, 0, 0, "Battery monitor initialized");
}

#ifndef BATTERY_MONITOR_SIMULATION
/* ----------------------------------- Battery_Monitor_Task ------------------------------------ */
void *Battery_Monitor_Task(void *arg0)
{
    /* Variables */
    char mailboxValue;          // Message sent through the mailbox
    uint16_t curRead = 1;       // Current reading counter
    uint32_t result = 0;        // Result of best fit line
    uint32_t battAverage = 0;   // Average of battery readings
    uint16_t battSample;        // Current battery sample

    /* Open the battery ADC Driver */
    ADC_Handle   batt_ADC;
    ADC_Params   params;
    ADC_Params_init(&params);
    batt_ADC = ADC_open(BATT_ADC_CHANNEL, &params);

    /* Sampling and calculation constants */
    const uint32_t res1 = BATT_RESISTOR_1;
    const uint32_t res2 = BATT_RESISTOR_2;
    const float vref = BATT_VREF;
    const uint32_t adcMax = BATT_ADC_MAX;
    const float full = BATT_FULL;
    const float low = BATT_LOW;
    const uint8_t hyst = BATT_HYSTERESIS;
    const uint32_t sleepTime = (1000000 * BATT_SAMP_PERIOD) / BATT_SAMP_SIZE;
    const uint32_t battFullAdc = (full * ((float)res2 / (res1 + res2)) * ((float)adcMax / vref));
    const uint32_t battLowAdc = (low * ((float)res2 / (res1 + res2)) * ((float)adcMax / vref));

    /* Monitor the battery level forever */
    while (1)
    {
        /* Take sample */
        ADC_convert(batt_ADC, &battSample);
        battAverage += battSample;
        curRead++;

        Display_printf(display, 0, 0, "Batt Samp:%d", battSample);

        /* If all samples are taken */
        if (curRead >= BATT_SAMP_SIZE)
        {
            /* Average them */
            battAverage /= BATT_SAMP_SIZE;
            Display_printf(display, 0, 0, "Batt Avg:%d", battAverage);

            /* Place the value on the best fit line */
            if(battAverage >= battFullAdc)
            {
                result = 999;   //99.9%
            }
            else if(battAverage <= battLowAdc)
            {
                result = 0;     //00.0%
            }
            else
            {
                /* Best fit line equation */
                result = (1000 * (battAverage - battLowAdc)) / (battFullAdc - battLowAdc);
                result = result - (result % hyst);
            }

            /* If its new then send it */
            if (result != G_BattLevel)
            {
                Display_printf(display, 0, 0, "New Batt:%d", result);
                G_BattLevel = result;
                mailboxValue = 'B';
                mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 0);
            }

            /* Reset variables */
            battAverage = 0;
            curRead = 1;
        }

        /* Sleep in between samples */
        usleep(sleepTime);
    }
}

#else /* BATTERY_MONITOR_SIMULATION */
/* ----------------------------------- Battery_Monitor_Task ------------------------------------ */
void *Battery_Monitor_Task(void *arg0)
{
    /* Communication POSIX mailbox variables */
    char mailboxValue;

    /* Setting the battery level to slowly drain */
    G_BattLevel = 999;
    while (1)
    {
        /* Send the battery level */
        mailboxValue = 'B';
        mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 0);

        /* Simulate draining the battery */
        G_BattLevel -= 10;
        if (G_BattLevel <= 30)
        {
            G_BattLevel = 999;
        }

        /* Wait for the next reading */
        sleep(BATT_SAMP_PERIOD);
    }
}

#endif /* BATTERY_MONITOR_SIMULATION */
