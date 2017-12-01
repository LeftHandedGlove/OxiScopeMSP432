
#include <OxiSc_Modules/CC2650_comm.h>

/* ------------------------------------- CC2650_Comm_Init -------------------------------------- */
void CC2650_Comm_Init(void)
{
    /* Initialize any required peripherals */
    GPIO_init();
    UART_init();
    Timer_init();

    /* Construct the unique semaphores */
    uint_fast16_t sem_res;
    sem_res = sem_init(&G_UartAck, 0, 0);
    if (sem_res != 0)
    {
        //Semaphore init failed
        while (1);
    }

    /* Setup the CC2650 reset timer */
    Timer_Params    timer_params;
    Timer_Params_init(&timer_params);
    timer_params.periodUnits = Timer_PERIOD_US;
    timer_params.period = RESET_WAIT_TIME;
    timer_params.timerMode  = Timer_ONESHOT_CALLBACK;
    timer_params.timerCallback = CC2650_Reset_Timer_CallBack;
    G_ResetTimerHandle = Timer_open(Board_TIMER_RESET, &timer_params);
    if (G_ResetTimerHandle == NULL)
    {
        // Timer_open() failed
        while (1);
    }

    /* Construct the various tasks */
    Create_Task(UART_RX_STACKSIZE, UART_RX_PRIORITY, Uart_Rx_Task);
    Create_Task(UART_TX_STACKSIZE, UART_TX_PRIORITY, Uart_Tx_Task);

    /* Initialize the global variables */
    G_ResetCount = 0;
    G_ResettingCC2650 = false;

    /* Open the UART port */
    UART_Params params;
    UART_Params_init(&params);
    params.baudRate = 115200;
    params.dataLength = UART_LEN_8;
    params.parityType = UART_PAR_NONE;
    params.stopBits = UART_STOP_ONE;
    params.readCallback = NULL;
    params.readDataMode = UART_DATA_TEXT;
    params.readEcho = UART_ECHO_OFF;
    params.readMode = UART_MODE_BLOCKING;
    params.readReturnMode = UART_RETURN_NEWLINE;
    params.readTimeout = UART_WAIT_FOREVER;
    params.writeCallback = NULL;
    params.writeDataMode = UART_DATA_BINARY;
    params.writeMode = UART_MODE_BLOCKING;
    params.writeTimeout = UART_WAIT_FOREVER;
    G_Uart_CC2650 = UART_open(Board_UART_CC2650, &params);
    if (G_Uart_CC2650 == NULL)
    {
        /* UART_open failed */
        while (1);
    }

    Display_printf(display, 0, 0, "CC2650 communication initialized");
}

/* --------------------------------------- ROT13_Encrypt --------------------------------------- */
void ROT13_Encrypt(char array[], size_t length)
{
    /* Variables */
    uint16_t i;     // for loop counter

    /* Add 13 to every character in the array */
    for (i = 0; i < length; ++i)
    {
        array[i] += 13;
    }
}

/* --------------------------------------- ROT13_Decrypt --------------------------------------- */
void ROT13_Decrypt(char array[], size_t length)
{
    /* Variables */
    uint16_t i;     // for loop counter

    /* Subtract 13 from every character in the array */
    for (i = 0; i < length; ++i)
    {
        array[i] -= 13;
    }
}

/* ---------------------------------------- Calc_CRC32 ----------------------------------------- */
uint32_t Calc_CRC32(char array[], size_t length)
{
    /* Variables */
    uint16_t i;     // for loop counter

    /* Initialize the CRC32 Hardware with the starting seed */
    CRC32_setSeed(CRC32_SEED, CRC32_MODE);

    /* Stuff every byte, starting with the MSB, through the CRC32 hardware */
    for (i = 0; i < length; i++)
    {
        CRC32_set8BitData(array[i], CRC32_MODE);
    }

    /* return the bit-reversed result XORed with all 1's */
    return (uint32_t)(CRC32_getResultReversed(CRC32_MODE) ^ 0xFFFFFFFF);
}

/* -------------------------------- CC2650_Reset_Timer_CallBack -------------------------------- */
void CC2650_Reset_Timer_CallBack(Timer_Handle time_hand)
{
    /* Variables */
    char mailboxValue;  // Message to be sent through mailbox

    /* Reset everything related to resetting the CC2650 */
    G_ResetCount = 0;
    G_ResettingCC2650 = false;

    /* Check to make sure the CC2650 is paired */
    if (GPIO_read(Board_GPIO_BTPAIR) == BTCHECK_CONN)
    {
        /* Enable interrupts */
        GPIO_enableInt(Board_GPIO_BTPAIR);
        /* Stuff comm mailbox with every possible data msg */
        mailboxValue = 'S';
        mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 0);
        mailboxValue = 'P';
        mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 0);
        mailboxValue = 'B';
        mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 0);
    }
    else
    {
        /* Try again */
        //Reset_CC2650(); //This causes a Hwi exception because of printing inside a callback
    }
}

/* --------------------------------------- Reset_CC2650 ---------------------------------------- */
void Reset_CC2650(void)
{
    /* Reset the reset count */
    G_ResetCount = 0;

    /* If it hasn't been reset already, then reset it */
    if (G_ResettingCC2650 == false)
    {
        /* Reset CC2650 */
        GPIO_write(Board_GPIO_CC2650RST, HIGH);
        usleep(10);
        GPIO_write(Board_GPIO_CC2650RST, LOW);
        Display_printf(display, 0, 0, "### Resetting CC2650 ###");

        /* Prevent future resetting */
        G_ResettingCC2650 = true;
        GPIO_disableInt(Board_GPIO_BTPAIR);

        /* Start the timer that will check if the CC2650 has paired */
        Timer_start(G_ResetTimerHandle);

    }
}

/* --------------------------------------- Uart_Rx_Task ---------------------------------------- */
void *Uart_Rx_Task(void *arg0)
{
    /* Variables */
    char mailboxValue;          // Message to be sent through mailbox
    uint32_t CRC32_extract;     // Extracted CRC32 from received message
    uint32_t CRC32_compute;     // Calculated CRC32 from received message


    /* Do UART RX stuff */
    while (1)
    {
        /* Clear the receive buffer */
        memset(G_UartRxBuffer, 0, 8);

        /* Wait for RX message */
        UART_read(G_Uart_CC2650, G_UartRxBuffer, sizeof(G_UartRxBuffer));

        /* Decrypt the message */
        ROT13_Decrypt(G_UartRxBuffer, sizeof(G_UartRxBuffer));

        /* Extract CRC32 */
        CRC32_extract = (G_UartRxBuffer[4] << 24) +
                        (G_UartRxBuffer[5] << 16) +
                        (G_UartRxBuffer[6] << 8) +
                        (G_UartRxBuffer[7] << 0);

        /* Compute CRC32 */
        CRC32_compute = Calc_CRC32(G_UartRxBuffer, 4);

        /* Compare extracted and calculated CRC32s */
        if (CRC32_extract != CRC32_compute)
        {
            mailboxValue = 'N';
            mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 1);
            continue;
        }

        /* Interpret RX message */
        switch (G_UartRxBuffer[0])
        {
            /* Go */
            case 'G':
            {
                /* Reset system counter */
                G_SysCount = G_LightAquisTime;
                /* Free the system */
                sem_post(&G_SysHold);
                /* Send acknowledge */
                mailboxValue = 'K';
                mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 1);
                break;
            }
            /* Light Acquisition Time */
            case 'L':
            {
                /* Set light acquisition time */
                G_LightAquisTime = ((G_UartRxBuffer[1] - '0') * 100) +
                                   ((G_UartRxBuffer[2] - '0') * 10) +
                                   ((G_UartRxBuffer[3] - '0') * 1);
                /* Reset system counter */
                G_SysCount = G_LightAquisTime;
                /* Send acknowledge */
                mailboxValue = 'K';
                mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 1);
                break;
            }
            /* Acknowledge */
            case 'K':
            {
                /* Reset CC2650 reset counters */
                G_ResetCount = 0;
                /* Let the TX task know an acknowledge was received */
                sem_post(&G_UartAck);
                break;
            }
            /* Not Acknowledge */
            case 'N':
            {
                /* Increment CC2650 reset counter */
                G_ResetCount++;
                /* If ready to reset CC2650 */
                if (G_ResetCount >= RESET_COUNTER_LIMIT)
                {
                    Reset_CC2650();
                }
                /* If not then resend previous message */
                else
                {
                    Display_printf(display, 0, 0, "TX Msg: %s", G_UartTxBuffer);
                    UART_write(G_Uart_CC2650, G_UartTxBuffer, sizeof(G_UartTxBuffer));
                }
                break;
            }
            /* Message faked it until it made it */
            default:
            {
                /* Increment reset counter */
                G_ResetCount++;
                /* If ready to reset CC2650 */
                if (G_ResetCount >= RESET_COUNTER_LIMIT)
                {
                    Reset_CC2650();
                }
                /* Send a not acknowledge */
                mailboxValue = 'N';
                mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 1);
                break;
            }
        }
    }
}


/* --------------------------------------- Uart_Tx_Task ---------------------------------------- */
void *Uart_Tx_Task(void *arg0)
{
    /* Variables */
    char mailboxValue;      // Message to be sent through mailbox
    uint16_t j;             // For loop counter
    char sprintfBuf[3];     // Buffer used by sprintf to transform numbers into strings
    uint32_t CRC32Holder;   // Holder for CRC32 calculations

    /* Send messages when told to */
    while (1)
    {
        /* Wait for message queue from other threads */
        mq_receive(G_CommMesQue, (char *)&mailboxValue, sizeof(mailboxValue), NULL);

        /* Check Bluetooth connection */
        if (GPIO_read(Board_GPIO_BTPAIR) == BTCHECK_DISC)
        {
            Display_printf(display, 0, 0, "BLE Down: Clearing Mailbox Value: %c", mailboxValue);
            Reset_CC2650();
            continue;
        }

        /* Build messages based on mailbox value */
        /* Add ID */
        G_UartTxBuffer[0] = mailboxValue;
        switch (mailboxValue)
        {
            /* Oxygen saturation */
            case 'S':
            {
                /* Convert SpO2 to string */
                sprintf(sprintfBuf, "%d", G_SpO2);
                /* Copy number string into payload */
                strcpy(G_UartTxBuffer + (4 - strlen(sprintfBuf)), sprintfBuf);
                /* Pad with zeros */
                for (j = strlen(sprintfBuf); j < 3; j++)
                {
                    G_UartTxBuffer[3-j] = '0';
                }
                break;
            }

            /* Pulse rate */
            case 'P':
            {
                /* Convert pulse rate to string */
                sprintf(sprintfBuf, "%d", G_PulseRate);
                /* Copy number string into payload */
                strcpy(G_UartTxBuffer + (4 - strlen(sprintfBuf)), sprintfBuf);
                /* Pad with zeros */
                for (j = strlen(sprintfBuf); j < 3; j++)
                {
                    G_UartTxBuffer[3-j] = '0';
                }
                break;
            }

            /* Battery level */
            case 'B':
            {
                /* Convert battery level to string */
                sprintf(sprintfBuf, "%d", G_BattLevel);
                /* Copy number string into payload */
                strcpy(G_UartTxBuffer + (4 - strlen(sprintfBuf)), sprintfBuf);
                /* Pad with zeros */
                for (j = strlen(sprintfBuf); j < 3; j++)
                {
                    G_UartTxBuffer[3-j] = '0';
                }
                break;
            }

            /* Acknowledge */
            case 'K':
            {
                /* Fill with Ks */
                G_UartTxBuffer[1] = 'K';
                G_UartTxBuffer[2] = 'K';
                G_UartTxBuffer[3] = 'K';
                break;
            }

            /* Non-acknowledge */
            case 'N':
            {
                /* Fill with Ns */
                G_UartTxBuffer[1] = 'N';
                G_UartTxBuffer[2] = 'N';
                G_UartTxBuffer[3] = 'N';
                break;
            }
        }

        /* Add CRC32 to message */
        CRC32Holder = Calc_CRC32(G_UartTxBuffer, 4);
        G_UartTxBuffer[4] = (char)((CRC32Holder & 0xFF000000) >> 24);
        G_UartTxBuffer[5] = (char)((CRC32Holder & 0x00FF0000) >> 16);
        G_UartTxBuffer[6] = (char)((CRC32Holder & 0x0000FF00) >> 8);
        G_UartTxBuffer[7] = (char)((CRC32Holder & 0x000000FF) >> 0);

        /* Encrypt the message */
        ROT13_Encrypt(G_UartTxBuffer, sizeof(G_UartTxBuffer));

        /* Send message */
        UART_write(G_Uart_CC2650, G_UartTxBuffer, sizeof(G_UartTxBuffer));

        /* Special case to skip waiting for acknowledgments when sending acknowledgments */
        if ((mailboxValue == 'K') || (mailboxValue == 'N'))
        {
            continue;
        }

        /* Wait for acknowledgment semaphore from Rx */
        while (1)
        {
            /* Timeout */
            /* Hacky way of using TI timed semaphore with POSIX semaphore */
            if (Semaphore_pend(Semaphore_handle(&(G_UartAck.sem)), UART_TIMEOUT / Clock_tickPeriod)
                == FALSE)
            {
                /* Increment reset counter */
                G_ResetCount++;
                /* If time to reset the CC2650 */
                if (G_ResetCount >= RESET_COUNTER_LIMIT)
                {
                    Reset_CC2650();
                    /* Break from while loop */
                    break;
                }
                /* If not then resend the previous message */
                else
                {
                    UART_write(G_Uart_CC2650, G_UartTxBuffer, sizeof(G_UartTxBuffer));
                }
            }
            /* No Timeout */
            else
            {
                /* Reset the reset count */
                G_ResetCount = 0;
                break;
            }
        }

    }
}



