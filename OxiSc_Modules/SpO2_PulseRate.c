
#include "SpO2_PulseRate.h"

/* ----------------------------------- SpO2_PulseRate_Init ------------------------------------- */
void SpO2_PulseRate_Init(void)
{
    /* Initialize peripherals */
    ADC_init();
    GPIO_init();

    /* Create various tasks */
    Create_Task(PPG_SAMPING_STACKSIZE, PPG_SAMPING_PRIORITY, PPG_Sampling_Task);
    Create_Task(PPG_PROCESS_STACKSIZE, PPG_PROCESS_PRIORITY, PPG_Processing_Task);
    Create_Task(PPG_CALC_STACKSIZE, PPG_CALC_PRIORITY, PPG_Calculate_Task);

    /* Create the various semaphores */
    uint_fast16_t sem_res;
    sem_res = sem_init(&G_PPGSamp, 0, 0);
    sem_res |= sem_init(&G_PPGProcess, 0, 0);
    if (sem_res != 0)
    {
        //Semaphore init failed
        while (1);
    }

    /* Setup Light Attribute structs */
    G_Red.rawSample = G_Ir.rawSample = 0;
    G_Red.ACFilteredSample = G_Ir.ACFilteredSample = 0;
    G_Red.DCFilteredSample = G_Ir.DCFilteredSample = 0;
    G_Red.vpp = G_Ir.vpp = 0;
    G_Red.freq = G_Ir.freq = 0.0;

    Display_printf(display, 0, 0, "SpO2 and PR initialized");
}

/* ------------------------------------ PPG_Sampling_Task -------------------------------------- */
void *PPG_Sampling_Task(void *arg0)
{
    /* Variables */

    /* Open PPG ADC drivers */
    ADC_Params   adc_params;
    ADC_Params_init(&adc_params);
    ADC_Handle redAC_ADC = ADC_open(Board_ADC_ACREDLIGHT, &adc_params);
    ADC_Handle redDC_ADC = ADC_open(Board_ADC_DCREDLIGHT, &adc_params);
    ADC_Handle irAC_ADC = ADC_open(Board_ADC_ACIRLIGHT, &adc_params);
    ADC_Handle irDC_ADC = ADC_open(Board_ADC_DCIRLIGHT, &adc_params);

    /* Loop forever taking samples at the sampling rate */
    while (1)
    {
//        /* Debugging: red laser always on, red channel only */
//        sem_wait(&G_SysHold);
//        GPIO_write(Board_GPIO_REDGATE, GATE_ACTIVE);
//        GPIO_write(Board_GPIO_REDLZR, LASER_ACTIVE);
//        usleep(1000000 / LIGHT_SAMP_RATE);
//        ADC_convert(redAC_ADC, &G_Red.rawSample);
//        sem_post(&G_SysHold);
//        sem_post(&G_PPGSamp);
//        continue;

//        /* Debugging: red laser flashing, red channel only */
//        sem_wait(&G_SysHold);
//        GPIO_write(Board_GPIO_REDGATE, GATE_ACTIVE);
//        GPIO_write(Board_GPIO_REDLZR, LASER_ACTIVE);
//        usleep(500000 / LIGHT_SAMP_RATE);
//        ADC_convert(redAC_ADC, &G_Red.rawSample);
//        GPIO_write(Board_GPIO_REDGATE, GATE_INACTIVE);
//        GPIO_write(Board_GPIO_REDLZR, LASER_INACTIVE);
//        usleep(500000 / LIGHT_SAMP_RATE);
//        sem_post(&G_SysHold);
//        sem_post(&G_PPGSamp);
//        continue;

//        /* Debugging: ir laser flashing, ir channel only */
//        sem_wait(&G_SysHold);
//        GPIO_write(Board_GPIO_IRGATE, GATE_ACTIVE);
//        GPIO_write(Board_GPIO_IRLZR, LASER_ACTIVE);
//        usleep(500000 / LIGHT_SAMP_RATE);
//        ADC_convert(irAC_ADC, &G_Ir.rawSample);
//        GPIO_write(Board_GPIO_IRGATE, GATE_INACTIVE);
//        GPIO_write(Board_GPIO_IRLZR, LASER_INACTIVE);
//        usleep(500000 / LIGHT_SAMP_RATE);
//        sem_post(&G_SysHold);
//        sem_post(&G_PPGSamp);
//        continue;

        /* Make sure system isn't idle */
        sem_wait(&G_SysHold);

        /* Sample Red */
        /* Turn on red laser & analog switch */
        GPIO_write(Board_GPIO_REDGATE, GATE_ACTIVE);
        GPIO_write(Board_GPIO_REDLZR, LASER_ACTIVE);
        /* Wait until the filters have saturated */
        usleep(500000 / LIGHT_SAMP_RATE);
        /* Take red sample */
        ADC_convert(redAC_ADC, &G_Red.rawSample);
        /* Turn off red laser & analog switch */
        GPIO_write(Board_GPIO_REDGATE, GATE_INACTIVE);
        GPIO_write(Board_GPIO_REDLZR, LASER_INACTIVE);

        /* Sample IR */
        /* Turn on IR laser & analog switch */
        GPIO_write(Board_GPIO_IRGATE, GATE_ACTIVE);
        GPIO_write(Board_GPIO_IRLZR, LASER_ACTIVE);
        /* Wait until the filters have saturated */
        usleep(500000 / LIGHT_SAMP_RATE);
        /* Take IR samples */
        ADC_convert(irAC_ADC, &G_Ir.rawSample);
        /* Turn off IR laser & analog switch */
        GPIO_write(Board_GPIO_IRGATE, GATE_INACTIVE);
        GPIO_write(Board_GPIO_IRLZR, LASER_INACTIVE);

        /* Free system */
        sem_post(&G_SysHold);

        /* Once samples have been taken, hand them off to the processing task */
        sem_post(&G_PPGSamp);
    }
}

/* ----------------------------------- OxiScope_FIR_Filter ------------------------------------- */
float OxiScope_FIR_Filter(uint16_t input, float inputs[], const float coeffs[], uint16_t numTaps)
{
    /* Variables */
    float output = 0;
    uint16_t i;

    /* Shift the new value into the input array */
    for (i = 0; i < numTaps - 1; i++)
    {
        /* Shift each index up 1 */
        inputs[numTaps - 1 - i] = inputs[numTaps - 2 - i];
    }
    /* Put the new input into the array */
    inputs[0] = (float)input;

    /* Add together every previous input multiplied by it's coefficient */
    for (i = 0; i < numTaps - 1; i++)
    {
        output += coeffs[i] * inputs[i];
    }

    /* Return the result */
    return output;
}

/* ----------------------------------- PPG_Processing_Task ------------------------------------- */
void *PPG_Processing_Task(void *arg0)
{
    /* Variables */

    /* DC only, 501 taps, 0.1 Hz */
    const float FIR_LPF_0d1Hz_Coeff[501] =
    {
        +0.0002800423f, +0.0002803207f, +0.0002808531f, +0.0002816401f, +0.0002826818f,
        +0.0002839785f, +0.0002855304f, +0.0002873378f, +0.0002894007f, +0.0002917192f,
        +0.0002942934f, +0.0002971232f, +0.0003002087f, +0.0003035497f, +0.0003071461f,
        +0.0003109977f, +0.0003151042f, +0.0003194655f, +0.0003240813f, +0.0003289510f,
        +0.0003340745f, +0.0003394512f, +0.0003450806f, +0.0003509622f, +0.0003570954f,
        +0.0003634796f, +0.0003701142f, +0.0003769984f, +0.0003841316f, +0.0003915128f,
        +0.0003991413f, +0.0004070162f, +0.0004151365f, +0.0004235014f, +0.0004321098f,
        +0.0004409606f, +0.0004500527f, +0.0004593851f, +0.0004689565f, +0.0004787658f,
        +0.0004888115f, +0.0004990926f, +0.0005096075f, +0.0005203550f, +0.0005313336f,
        +0.0005425418f, +0.0005539781f, +0.0005656410f, +0.0005775288f, +0.0005896401f,
        +0.0006019730f, +0.0006145259f, +0.0006272969f, +0.0006402844f, +0.0006534866f,
        +0.0006669015f, +0.0006805272f, +0.0006943619f, +0.0007084034f, +0.0007226499f,
        +0.0007370992f, +0.0007517493f, +0.0007665980f, +0.0007816433f, +0.0007968828f,
        +0.0008123144f, +0.0008279358f, +0.0008437447f, +0.0008597387f, +0.0008759155f,
        +0.0008922727f, +0.0009088079f, +0.0009255186f, +0.0009424023f, +0.0009594564f,
        +0.0009766784f, +0.0009940658f, +0.0010116159f, +0.0010293259f, +0.0010471933f,
        +0.0010652153f, +0.0010833893f, +0.0011017125f, +0.0011201819f, +0.0011387948f,
        +0.0011575484f, +0.0011764398f, +0.0011954661f, +0.0012146244f, +0.0012339116f,
        +0.0012533249f, +0.0012728613f, +0.0012925175f, +0.0013122908f, +0.0013321780f,
        +0.0013521760f, +0.0013722817f, +0.0013924919f, +0.0014128035f, +0.0014332134f,
        +0.0014537182f, +0.0014743150f, +0.0014950002f, +0.0015157709f, +0.0015366237f,
        +0.0015575553f, +0.0015785623f, +0.0015996416f, +0.0016207898f, +0.0016420034f,
        +0.0016632793f, +0.0016846140f, +0.0017060041f, +0.0017274463f, +0.0017489371f,
        +0.0017704731f, +0.0017920510f, +0.0018136672f, +0.0018353184f, +0.0018570010f,
        +0.0018787116f, +0.0019004468f, +0.0019222032f, +0.0019439771f, +0.0019657651f,
        +0.0019875637f, +0.0020093697f, +0.0020311791f, +0.0020529886f, +0.0020747948f,
        +0.0020965943f, +0.0021183833f, +0.0021401586f, +0.0021619163f, +0.0021836534f,
        +0.0022053658f, +0.0022270505f, +0.0022487037f, +0.0022703223f, +0.0022919022f,
        +0.0023134404f, +0.0023349333f, +0.0023563774f, +0.0023777692f, +0.0023991051f,
        +0.0024203819f, +0.0024415960f, +0.0024627440f, +0.0024838224f, +0.0025048279f,
        +0.0025257571f, +0.0025466064f, +0.0025673728f, +0.0025880523f, +0.0026086420f,
        +0.0026291385f, +0.0026495382f, +0.0026698383f, +0.0026900349f, +0.0027101249f,
        +0.0027301053f, +0.0027499725f, +0.0027697235f, +0.0027893549f, +0.0028088635f,
        +0.0028282462f, +0.0028474997f, +0.0028666211f, +0.0028856071f, +0.0029044542f,
        +0.0029231601f, +0.0029417214f, +0.0029601348f, +0.0029783975f, +0.0029965064f,
        +0.0030144588f, +0.0030322515f, +0.0030498817f, +0.0030673463f, +0.0030846426f,
        +0.0031017680f, +0.0031187192f, +0.0031354935f, +0.0031520885f, +0.0031685012f,
        +0.0031847290f, +0.0032007692f, +0.0032166191f, +0.0032322761f, +0.0032477377f,
        +0.0032630013f, +0.0032780643f, +0.0032929245f, +0.0033075791f, +0.0033220260f,
        +0.0033362624f, +0.0033502865f, +0.0033640955f, +0.0033776874f, +0.0033910598f,
        +0.0034042106f, +0.0034171378f, +0.0034298387f, +0.0034423119f, +0.0034545548f,
        +0.0034665656f, +0.0034783424f, +0.0034898832f, +0.0035011859f, +0.0035122489f,
        +0.0035230701f, +0.0035336479f, +0.0035439807f, +0.0035540664f, +0.0035639035f,
        +0.0035734905f, +0.0035828257f, +0.0035919074f, +0.0036007345f, +0.0036093052f,
        +0.0036176182f, +0.0036256721f, +0.0036334656f, +0.0036409975f, +0.0036482662f,
        +0.0036552709f, +0.0036620104f, +0.0036684831f, +0.0036746885f, +0.0036806255f,
        +0.0036862928f, +0.0036916896f, +0.0036968151f, +0.0037016682f, +0.0037062485f,
        +0.0037105551f, +0.0037145871f, +0.0037183438f, +0.0037218248f, +0.0037250298f,
        +0.0037279574f, +0.0037306079f, +0.0037329805f, +0.0037350750f, +0.0037368909f,
        +0.0037384278f, +0.0037396858f, +0.0037406643f, +0.0037413635f, +0.0037417829f,
        +0.0037419228f, +0.0037417829f, +0.0037413635f, +0.0037406643f, +0.0037396858f,
        +0.0037384278f, +0.0037368909f, +0.0037350750f, +0.0037329805f, +0.0037306079f,
        +0.0037279574f, +0.0037250298f, +0.0037218248f, +0.0037183438f, +0.0037145871f,
        +0.0037105551f, +0.0037062485f, +0.0037016682f, +0.0036968151f, +0.0036916896f,
        +0.0036862928f, +0.0036806255f, +0.0036746885f, +0.0036684831f, +0.0036620104f,
        +0.0036552709f, +0.0036482662f, +0.0036409975f, +0.0036334656f, +0.0036256721f,
        +0.0036176182f, +0.0036093052f, +0.0036007345f, +0.0035919074f, +0.0035828257f,
        +0.0035734905f, +0.0035639035f, +0.0035540664f, +0.0035439807f, +0.0035336479f,
        +0.0035230701f, +0.0035122489f, +0.0035011859f, +0.0034898832f, +0.0034783424f,
        +0.0034665656f, +0.0034545548f, +0.0034423119f, +0.0034298387f, +0.0034171378f,
        +0.0034042106f, +0.0033910598f, +0.0033776874f, +0.0033640955f, +0.0033502865f,
        +0.0033362624f, +0.0033220260f, +0.0033075791f, +0.0032929245f, +0.0032780643f,
        +0.0032630013f, +0.0032477377f, +0.0032322761f, +0.0032166191f, +0.0032007692f,
        +0.0031847290f, +0.0031685012f, +0.0031520885f, +0.0031354935f, +0.0031187192f,
        +0.0031017680f, +0.0030846426f, +0.0030673463f, +0.0030498817f, +0.0030322515f,
        +0.0030144588f, +0.0029965064f, +0.0029783975f, +0.0029601348f, +0.0029417214f,
        +0.0029231601f, +0.0029044542f, +0.0028856071f, +0.0028666211f, +0.0028474997f,
        +0.0028282462f, +0.0028088635f, +0.0027893549f, +0.0027697235f, +0.0027499725f,
        +0.0027301053f, +0.0027101249f, +0.0026900349f, +0.0026698383f, +0.0026495382f,
        +0.0026291385f, +0.0026086420f, +0.0025880523f, +0.0025673728f, +0.0025466064f,
        +0.0025257571f, +0.0025048279f, +0.0024838224f, +0.0024627440f, +0.0024415960f,
        +0.0024203819f, +0.0023991051f, +0.0023777692f, +0.0023563774f, +0.0023349333f,
        +0.0023134404f, +0.0022919022f, +0.0022703223f, +0.0022487037f, +0.0022270505f,
        +0.0022053658f, +0.0021836534f, +0.0021619163f, +0.0021401586f, +0.0021183833f,
        +0.0020965943f, +0.0020747948f, +0.0020529886f, +0.0020311791f, +0.0020093697f,
        +0.0019875637f, +0.0019657651f, +0.0019439771f, +0.0019222032f, +0.0019004468f,
        +0.0018787116f, +0.0018570010f, +0.0018353184f, +0.0018136672f, +0.0017920510f,
        +0.0017704731f, +0.0017489371f, +0.0017274463f, +0.0017060041f, +0.0016846140f,
        +0.0016632793f, +0.0016420034f, +0.0016207898f, +0.0015996416f, +0.0015785623f,
        +0.0015575553f, +0.0015366237f, +0.0015157709f, +0.0014950002f, +0.0014743150f,
        +0.0014537182f, +0.0014332134f, +0.0014128035f, +0.0013924919f, +0.0013722817f,
        +0.0013521760f, +0.0013321780f, +0.0013122908f, +0.0012925175f, +0.0012728613f,
        +0.0012533249f, +0.0012339116f, +0.0012146244f, +0.0011954661f, +0.0011764398f,
        +0.0011575484f, +0.0011387948f, +0.0011201819f, +0.0011017125f, +0.0010833893f,
        +0.0010652153f, +0.0010471933f, +0.0010293259f, +0.0010116159f, +0.0009940658f,
        +0.0009766784f, +0.0009594564f, +0.0009424023f, +0.0009255186f, +0.0009088079f,
        +0.0008922727f, +0.0008759155f, +0.0008597387f, +0.0008437447f, +0.0008279358f,
        +0.0008123144f, +0.0007968828f, +0.0007816433f, +0.0007665980f, +0.0007517493f,
        +0.0007370992f, +0.0007226499f, +0.0007084034f, +0.0006943619f, +0.0006805272f,
        +0.0006669015f, +0.0006534866f, +0.0006402844f, +0.0006272969f, +0.0006145259f,
        +0.0006019730f, +0.0005896401f, +0.0005775288f, +0.0005656410f, +0.0005539781f,
        +0.0005425418f, +0.0005313336f, +0.0005203550f, +0.0005096075f, +0.0004990926f,
        +0.0004888115f, +0.0004787658f, +0.0004689565f, +0.0004593851f, +0.0004500527f,
        +0.0004409606f, +0.0004321098f, +0.0004235014f, +0.0004151365f, +0.0004070162f,
        +0.0003991413f, +0.0003915128f, +0.0003841316f, +0.0003769984f, +0.0003701142f,
        +0.0003634796f, +0.0003570954f, +0.0003509622f, +0.0003450806f, +0.0003394512f,
        +0.0003340745f, +0.0003289510f, +0.0003240813f, +0.0003194655f, +0.0003151042f,
        +0.0003109977f, +0.0003071461f, +0.0003035497f, +0.0003002087f, +0.0002971232f,
        +0.0002942934f, +0.0002917192f, +0.0002894007f, +0.0002873378f, +0.0002855304f,
        +0.0002839785f, +0.0002826818f, +0.0002816401f, +0.0002808531f, +0.0002803207f,
        +0.0002800423f
    };

    /* AC with DC, 251 taps, 4.5Hz */
    const float FIR_LPF_4d5Hz_Coeff[251]    = {
        +0.0002036836f, +0.0002043849f, +0.0002031877f, +0.0002000097f, +0.0001947444f,
        +0.0001872638f, +0.0001774227f, +0.0001650644f, +0.0001500270f, +0.0001321523f,
        +0.0001112943f, +0.0000873291f, +0.0000601653f, +0.0000297547f, -0.0000038977f,
        -0.0000407231f, -0.0000805796f, -0.0001232441f, -0.0001684047f, -0.0002156557f,
        -0.0002644937f, -0.0003143161f, -0.0003644223f, -0.0004140169f, -0.0004622154f,
        -0.0005080535f, -0.0005504980f, -0.0005884612f, -0.0006208172f, -0.0006464211f,
        -0.0006641299f, -0.0006728255f, -0.0006714395f, -0.0006589781f, -0.0006345488f,
        -0.0005973868f, -0.0005468808f, -0.0004825981f, -0.0004043087f, -0.0003120069f,
        -0.0002059305f, -0.0000865768f, +0.0000452845f, +0.0001886029f, +0.0003420431f,
        +0.0005039854f, +0.0006725317f, +0.0008455166f, +0.0010205236f, +0.0011949077f,
        +0.0013658224f, +0.0015302524f, +0.0016850514f, +0.0018269846f, +0.0019527754f,
        +0.0020591554f, +0.0021429183f, +0.0022009753f, +0.0022304130f, +0.0022285508f,
        +0.0021930002f, +0.0021217205f, +0.0020130749f, +0.0018658825f, +0.0016794659f,
        +0.0014536951f, +0.0011890256f, +0.0008865288f, +0.0005479166f, +0.0001755570f,
        -0.0002275188f, -0.0006576179f, -0.0011103970f, -0.0015808833f, -0.0020635051f,
        -0.0025521328f, -0.0030401282f, -0.0035204038f, -0.0039854911f, -0.0044276170f,
        -0.0048387866f, -0.0052108727f, -0.0055357134f, -0.0058052088f, -0.0060114274f,
        -0.0061467066f, -0.0062037623f, -0.0061757895f, -0.0060565677f, -0.0058405558f,
        -0.0055229869f, -0.0050999545f, -0.0045684921f, -0.0039266390f, -0.0031735047f,
        -0.0023093119f, -0.0013354352f, -0.0002544223f, +0.0009299962f, +0.0022129121f,
        +0.0035882583f, +0.0050488426f, +0.0065863947f, +0.0081916284f, +0.0098543148f,
        +0.0115633709f, +0.0133069577f, +0.0150725907f, +0.0168472566f, +0.0186175425f,
        +0.0203697737f, +0.0220901426f, +0.0237648617f, +0.0253802985f, +0.0269231275f,
        +0.0283804704f, +0.0297400281f, +0.0309902281f, +0.0321203358f, +0.0331205837f,
        +0.0339822695f, +0.0346978642f, +0.0352610908f, +0.0356669836f, +0.0359119624f,
        +0.0359938666f, +0.0359119624f, +0.0356669836f, +0.0352610908f, +0.0346978642f,
        +0.0339822695f, +0.0331205837f, +0.0321203358f, +0.0309902281f, +0.0297400281f,
        +0.0283804704f, +0.0269231275f, +0.0253802985f, +0.0237648617f, +0.0220901426f,
        +0.0203697737f, +0.0186175425f, +0.0168472566f, +0.0150725907f, +0.0133069577f,
        +0.0115633709f, +0.0098543148f, +0.0081916284f, +0.0065863947f, +0.0050488426f,
        +0.0035882583f, +0.0022129121f, +0.0009299962f, -0.0002544223f, -0.0013354352f,
        -0.0023093119f, -0.0031735047f, -0.0039266390f, -0.0045684921f, -0.0050999545f,
        -0.0055229869f, -0.0058405558f, -0.0060565677f, -0.0061757895f, -0.0062037623f,
        -0.0061467066f, -0.0060114274f, -0.0058052088f, -0.0055357134f, -0.0052108727f,
        -0.0048387866f, -0.0044276170f, -0.0039854911f, -0.0035204038f, -0.0030401282f,
        -0.0025521328f, -0.0020635051f, -0.0015808833f, -0.0011103970f, -0.0006576179f,
        -0.0002275188f, +0.0001755570f, +0.0005479166f, +0.0008865288f, +0.0011890256f,
        +0.0014536951f, +0.0016794659f, +0.0018658825f, +0.0020130749f, +0.0021217205f,
        +0.0021930002f, +0.0022285508f, +0.0022304130f, +0.0022009753f, +0.0021429183f,
        +0.0020591554f, +0.0019527754f, +0.0018269846f, +0.0016850514f, +0.0015302524f,
        +0.0013658224f, +0.0011949077f, +0.0010205236f, +0.0008455166f, +0.0006725317f,
        +0.0005039854f, +0.0003420431f, +0.0001886029f, +0.0000452845f, -0.0000865768f,
        -0.0002059305f, -0.0003120069f, -0.0004043087f, -0.0004825981f, -0.0005468808f,
        -0.0005973868f, -0.0006345488f, -0.0006589781f, -0.0006714395f, -0.0006728255f,
        -0.0006641299f, -0.0006464211f, -0.0006208172f, -0.0005884612f, -0.0005504980f,
        -0.0005080535f, -0.0004622154f, -0.0004140169f, -0.0003644223f, -0.0003143161f,
        -0.0002644937f, -0.0002156557f, -0.0001684047f, -0.0001232441f, -0.0000805796f,
        -0.0000407231f, -0.0000038977f, +0.0000297547f, +0.0000601653f, +0.0000873291f,
        +0.0001112943f, +0.0001321523f, +0.0001500270f, +0.0001650644f, +0.0001774227f,
        +0.0001872638f, +0.0001947444f, +0.0002000097f, +0.0002031877f, +0.0002043849f,
        +0.0002036836f
    };

    /* Loop forever processing incoming samples */
    float acFIRTemp, dcFIRTemp;
    uint16_t recalcCount = 0;
    uint16_t i;
    while (1)
    {
        /* Wait for the new samples semaphore */
        sem_wait(&G_PPGSamp);

        /* Push raw red sample through the FIR filters */
        dcFIRTemp = OxiScope_FIR_Filter(G_Red.rawSample, G_Red.DCInputs, FIR_LPF_0d1Hz_Coeff, DC_FIR_NUM_TAPS);
        acFIRTemp = OxiScope_FIR_Filter(G_Red.rawSample, G_Red.ACInputs, FIR_LPF_4d5Hz_Coeff, AC_FIR_NUM_TAPS);
        acFIRTemp -= dcFIRTemp;
        acFIRTemp *= RED_AC_GAIN;
        acFIRTemp += AC_OFFSET;
        G_Red.DCFilteredSample = (uint16_t)dcFIRTemp;
        G_Red.ACFilteredSample = (uint16_t)acFIRTemp;
        /* Push raw ir sample through the FIR filters */
        dcFIRTemp = OxiScope_FIR_Filter(G_Ir.rawSample, G_Ir.DCInputs, FIR_LPF_0d1Hz_Coeff, DC_FIR_NUM_TAPS);
        acFIRTemp = OxiScope_FIR_Filter(G_Ir.rawSample, G_Ir.ACInputs, FIR_LPF_4d5Hz_Coeff, AC_FIR_NUM_TAPS);
        acFIRTemp -= dcFIRTemp;
        acFIRTemp *= IR_AC_GAIN;
        acFIRTemp += AC_OFFSET;
        G_Ir.DCFilteredSample = (uint16_t)dcFIRTemp;
        G_Ir.ACFilteredSample = (uint16_t)acFIRTemp;

        /* Stuff sample into window buffer */
        G_Red.windowBuffer[recalcCount] = G_Red.ACFilteredSample;
        G_Ir.windowBuffer[recalcCount] = G_Ir.ACFilteredSample;
        recalcCount++;

        /* Serial Port Plotter lines. ONLY USE ONE AT A TIME! */
        /* Raw red AC sample, red AC filtered sample, red DC filtered sample */
        Display_printf(display, 0, 0, "$%d %d %d;", G_Red.rawSample, G_Red.ACFilteredSample, G_Red.DCFilteredSample);
        /* Raw ir AC sample, ir AC filtered sample, ir DC filtered sample */
        Display_printf(display, 0, 0, "$%d %d %d;", G_Ir.rawSample, G_Ir.ACFilteredSample, G_Ir.DCFilteredSample);
        /* red AC filtered sample, ir AC filtered sample */
        Display_printf(display, 0, 0, "$%d %d;", G_Red.ACFilteredSample, G_Ir.ACFilteredSample);


        /* If the required number of samples to recalculate has been reached */
        if (recalcCount >= RECALC_SAMPLES)
        {
            recalcCount = 0;
            /* Stuff the window samples into the previous filtered samples array */
            /* Shift previous samples over */
            for (i = 0; i < WINDOW_SIZE - RECALC_SAMPLES; i++)
            {
                G_Red.ACFilteredSamples[WINDOW_SIZE - 1 - i] =
                G_Red.ACFilteredSamples[WINDOW_SIZE - 1 - RECALC_SAMPLES - i];
                G_Ir.ACFilteredSamples[WINDOW_SIZE - 1 - i] =
                G_Ir.ACFilteredSamples[WINDOW_SIZE - 1 - RECALC_SAMPLES - i];
            }
            /* Put the new samples in the big array */
            for (i = 0; i < RECALC_SAMPLES; i++)
            {
                G_Red.ACFilteredSamples[i] = G_Red.windowBuffer[i];
                G_Ir.ACFilteredSamples[i] = G_Ir.windowBuffer[i];
            }

            /* Send them to see if new waveform chars have been found */
            sem_post(&G_PPGProcess);
        }
    }
}

/* -------------------------------------- Find_Vpp_Freq ---------------------------------------- */
void Find_Vpp_Freq(LightAttr *light)
{
    /* Variables */
    uint16_t globalMax, globalMin;
    uint16_t maxCutoff, minCutoff;
    bool maxLocked, minLocked;
    uint16_t localPointTemp, localPointIndexTemp;
    uint16_t localPoints[40], localPointsIndex[40], localPointsCount;
    uint16_t i;

    //Display_printf(display, 0, 0, "----------------------------------------");

    /* Sweep for global max and global min */
    globalMax = 0;
    globalMin = 65535;
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        if (light->ACFilteredSamples[i] > globalMax)
        {
            globalMax = light->ACFilteredSamples[i];
        }
        if (light->ACFilteredSamples[i] < globalMin)
        {
            globalMin = light->ACFilteredSamples[i];
        }
    }

    /* Calculate the max and min cutoff points */
    maxCutoff = (((float)globalMax - (float)globalMin) * MAX_WEIGHT) + (float)globalMin;
    minCutoff = (((float)globalMax - (float)globalMin) * MIN_WEIGHT) + (float)globalMin;

    //Display_printf(display, 0, 0, "GMax:%d, GMin:%d, MaxCut:%d, MinCut:%d",
                   //globalMax, globalMin, maxCutoff, minCutoff);

    /* Sweep through all samples to find local max and local mins */
    localPointsCount = 0;
    localPointIndexTemp = 0;
    minLocked = false;
    maxLocked = false;
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        /* When above max cutoff, find local max */
        if (light->ACFilteredSamples[i] > maxCutoff)
        {
            /* lock in the previous minimum,
             * prevent a new min from being locked in,
             * free up the max to be locked */
            if (!minLocked)
            {
                //Display_printf(display, 0, 0, "Passing maxCut, locking min: %d at %d",
                               //localPointTemp, localPointIndexTemp);

                localPoints[localPointsCount] = localPointTemp;
                localPointsIndex[localPointsCount] = localPointIndexTemp;
                localPointsCount++;
                minLocked = true;
                maxLocked = false;
                localPointTemp = 0;
            }

            /* If maxes aren't locked then find the local maximum */
            if (!maxLocked)
            {
                if (light->ACFilteredSamples[i] > localPointTemp)
                {
                    localPointTemp = light->ACFilteredSamples[i];
                    localPointIndexTemp = i;
                }
            }
        }
        /* When below min cutoff, find local min */
        if (light->ACFilteredSamples[i] < minCutoff)
        {
            /* lock in the previous maximum,
             * prevent a new max from being locked in,
             * free up the min to be locked*/
            if (!maxLocked)
            {
                //Display_printf(display, 0, 0, "Passing minCut, locking max: %d at %d",
                               //localPointTemp, localPointIndexTemp);

                localPoints[localPointsCount] = localPointTemp;
                localPointsIndex[localPointsCount] = localPointIndexTemp;
                localPointsCount++;
                minLocked = false;
                maxLocked = true;
                localPointTemp = 65355;
            }

            /* If mins aren't locked then find the local minimum */
            if (!minLocked)
            {
                if (light->ACFilteredSamples[i] < localPointTemp)
                {
                    localPointTemp = light->ACFilteredSamples[i];
                    localPointIndexTemp = i;
                }
            }
        }
    }

    //Display_printf(display, 0, 0, "LocalPoints:%d", localPointsCount);

    /* Use max and min values to find the Vpp */
    light->vpp = 0;

    for (i = 1; i < localPointsCount; i++)
    {
        /* If a max occurs first */
        if (localPoints[0] > localPoints[1])
        {
            /* If on an even iteration */
            if (i % 2)
            {
                light->vpp += localPoints[i] - localPoints[i + 1];
            }
            /* If on an odd iteration */
            else
            {
                light->vpp += localPoints[i + 1] - localPoints[i];
            }
        }
        /* If a min occurs first */
        else
        {
            /* If on an even iteration */
            if (i % 2)
            {
                light->vpp += localPoints[i + 1] - localPoints[i];
            }
            /* If on an odd iteration */
            else
            {
                light->vpp += localPoints[i] - localPoints[i + 1];
            }
        }
    }
    /* deal with divide by zero */
    if (localPointsCount)
    {
        light->vpp /= (localPointsCount - 1);
    }
    else
    {
        //Display_printf(display, 0, 0, "No points found!");
    }

    /* Use the difference between max indexes to find the frequency */
    light->freq = 0.0;
    uint8_t startPoint;
    uint8_t freqCount = 0;
    if (localPoints[0] > localPoints[1])
    {
        startPoint = 1;
    }
    else
    {
        startPoint = 2;
    }
    for (i = startPoint; i < (localPointsCount - 2); i += 2)
    {
        light->freq += localPointsIndex[i + 2] - localPointsIndex[i];
        freqCount++;
    }
    /* Deal with divide by zero */
    if (freqCount)
    {
        light->freq /= freqCount;
        light->freq = LIGHT_SAMP_RATE / light->freq;
    }
    else
    {
        //Display_printf(display, 0, 0, "Not enough maxes found!");
    }

    //Display_printf(display, 0, 0, "Vpp:%d, Freq:%f", light->vpp, light->freq);
}

/* ------------------------------------ PPG_Calculate_Task ------------------------------------- */
void *PPG_Calculate_Task(void *arg0)
{
    /* Variables */
    char mailboxValue;      // Message to be sent through the mailbox
    float ROR_Red;          // DC Normalized Red Vpp
    float ROR_IR;           // DC Normalized Infrared Vpp
    uint32_t result;        // holder for results before being validated

    while (1)
    {
        /* Wait for processed samples */
        sem_wait(&G_PPGProcess);

        /* Find the voltage peak to peak and freuqency of both light sources */
        Find_Vpp_Freq(&G_Red);
        Find_Vpp_Freq(&G_Ir);

        /* Deal with divide by zero scenarios */
        if (G_Red.vpp < 1) { G_Red.vpp = 1; }
        if (G_Red.DCFilteredSample <  1) { G_Red.DCFilteredSample = 1; }
        if (G_Ir.vpp < 1) { G_Ir.vpp = 1; }
        if (G_Ir.DCFilteredSample < 1) { G_Ir.DCFilteredSample = 1; }

        /* Calculate oxygen saturation */
        ROR_Red = ((float)G_Red.vpp) / (G_Red.DCFilteredSample);
        ROR_IR = ((float)G_Ir.vpp) / (G_Ir.DCFilteredSample);
        result = (uint32_t)((11000 - (2500 * (ROR_Red / ROR_IR))) / 11);
        /* Acceptable? (Not outside normal 99.9% bounds) */
        if (result <= 999)
        {
            if (result != G_SpO2)
            {
                G_SpO2 = result;
                Display_printf(display, 0, 0, "New SpO2: %d", G_SpO2);
                mailboxValue = 'S';
                //mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 0);
            }
        }
        else
        {
            //Display_printf(display, 0, 0, "!!!Bad SpO2 Value!!!");
        }

        /* Calculate Pulse Rate */
        /* simplified *60/2 with truncation rounding */
        result = (uint32_t)(((G_Red.freq + G_Ir.freq) * 30) + 0.5);
        /* Acceptable? (no larger than 240 BPM) */
        if (result < 240)
        {
            if (result != G_PulseRate)
            {
                G_PulseRate = result;
                Display_printf(display, 0, 0, "New Pulse Rate: %d", G_PulseRate);
                mailboxValue = 'P';
                //mq_send(G_CommMesQue, (char*)&mailboxValue, sizeof(mailboxValue), 0);
            }
        }
        else
        {
            //Display_printf(display, 0, 0, "!!!Bad Pulse Rate Value!!!");
        }
    }
}
