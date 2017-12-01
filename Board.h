/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BOARD_H
#define __BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/ADC.h>
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/SDSPI.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Watchdog.h>

#include "MSP_EXP432P401R.h"

#define LOW     (0)
#define HIGH    (1)
#define LASER_ACTIVE        HIGH
#define LASER_INACTIVE      LOW
#define GATE_ACTIVE         HIGH
#define GATE_INACTIVE       LOW
#define BTCHECK_CONN        HIGH
#define BTCHECK_DISC        LOW
#define LED_ACTIVE          HIGH
#define LED_INACTIVE        LOW

/* ============================================================ */
/* ================= OxiScope Board Settings ================== */
/* ============================================================ */
/* UART */
#define Board_UART_CC2650       MSP_EXP432P401R_UARTA0
#define Board_UART_DEBUG        MSP_EXP432P401R_UARTA1  //Change the display uart interface to this inside MSP_EXP432P401R.c

/* GPIO */
#define Board_GPIO_STARTBTN     MSP_EXP432P401R_GPIO_STARTBTN
#define Board_GPIO_BTPAIR       MSP_EXP432P401R_GPIO_BTPAIR
#define Board_GPIO_CC2650RST    MSP_EXP432P401R_GPIO_CC2650RST
#define Board_GPIO_REDLZR       MSP_EXP432P401R_GPIO_REDLZR
#define Board_GPIO_IRLZR        MSP_EXP432P401R_GPIO_IRLZR
#define Board_GPIO_REDGATE      MSP_EXP432P401R_GPIO_REDGATE
#define Board_GPIO_IRGATE       MSP_EXP432P401R_GPIO_IRGATE
#define Board_GPIO_EXTRA2       MSP_EXP432P401R_GPIO_EXTRA2
#define Board_GPIO_READYLED     MSP_EXP432P401R_GPIO_READYLED

/* ADC */
#define Board_ADC_ACREDLIGHT    MSP_EXP432P401R_ADC0
#define Board_ADC_DCREDLIGHT    MSP_EXP432P401R_ADC1
#define Board_ADC_ACIRLIGHT     MSP_EXP432P401R_ADC3
#define Board_ADC_DCIRLIGHT     MSP_EXP432P401R_ADC2
#define Board_ADC_BATTERY       MSP_EXP432P401R_ADC4

/* Timer */
#define Board_TIMER_LIGHT       MSP_EXP432P401R_TIMER_T32_0
#define Board_TIMER_PULSE       MSP_EXP432P401R_TIMER_TA_1
#define Board_TIMER_DEBOUNCE    MSP_EXP432P401R_TIMER_TA_2
#define Board_TIMER_RESET       MSP_EXP432P401R_TIMER_TA_3

/* Watchdog */
#define Board_WATCHDOG0         MSP_EXP432P401R_WATCHDOG

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
