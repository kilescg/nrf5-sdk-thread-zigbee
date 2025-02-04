/*

Copyright (c) 2009-2024 ARM Limited. All rights reserved.

    SPDX-License-Identifier: Apache-2.0

Licensed under the Apache License, Version 2.0 (the License); you may
not use this file except in compliance with the License.
You may obtain a copy of the License at

    www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an AS IS BASIS, WITHOUT
WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

NOTICE: This file has been modified by Nordic Semiconductor ASA.

*/

/* NOTE: Template files (including this one) are application specific and therefore expected to
   be copied into the application project folder prior to its use! */

#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "nrf51_erratas.h"
#include "system_nrf51.h"

/*lint ++flb "Enter library region" */


#define __SYSTEM_CLOCK_DEFAULT      (16000000UL)     /*!< nRF51 devices use a fixed System Clock Frequency of 16MHz */

#if defined ( __CC_ARM )
    uint32_t SystemCoreClock __attribute__((used)) = __SYSTEM_CLOCK_DEFAULT;
#elif defined ( __ICCARM__ )
    __root uint32_t SystemCoreClock = __SYSTEM_CLOCK_DEFAULT;
#elif defined   ( __GNUC__ )
    uint32_t SystemCoreClock __attribute__((used)) = __SYSTEM_CLOCK_DEFAULT;
#endif

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = __SYSTEM_CLOCK_DEFAULT;
}

void SystemInit(void)
{
    /* If desired, switch off the unused RAM to lower consumption by the use of RAMON register.
       It can also be done in the application main() function. */

    /* Prepare the peripherals for use as indicated by the PAN 26 "System: Manual setup is required
       to enable the use of peripherals" found at Product Anomaly document for your device found at
       https://infocenter.nordicsemi.com/index.jsp The side effect of executing these instructions in the devices
       that do not need it is that the new peripherals in the second generation devices (LPCOMP for
       example) will not be available. */
    if (nrf51_errata_26())
    {
        *(uint32_t volatile *)0x40000504ul = 0xC007FFDFul;
        *(uint32_t volatile *)0x40006C18ul = 0x00008000ul;
    }

    /* Disable PROTENSET registers under debug, as indicated by PAN 59 "MPU: Reset value of DISABLEINDEBUG
       register is incorrect" found at Product Anomaly document for your device found at
       https://infocenter.nordicsemi.com/index.jsp There is no side effect of using these instruction if not needed. */
    if (nrf51_errata_59())
    {
        NRF_MPU->DISABLEINDEBUG = MPU_DISABLEINDEBUG_DISABLEINDEBUG_Disabled << MPU_DISABLEINDEBUG_DISABLEINDEBUG_Pos;
    }

    /* Execute the following code to eliminate excessive current in sleep mode with RAM retention in nRF51802 devices,
       as indicated by PAN 76 "System: Excessive current in sleep mode with retention" found at Product Anomaly document
       for your device found at https://infocenter.nordicsemi.com/index.jsp */
    if (nrf51_errata_76()){
        if (*(uint32_t volatile *)0x4006EC00ul != 1ul){
            *(uint32_t volatile *)0x4006EC00ul = 0x9375ul;
            while (*(uint32_t volatile *)0x4006EC00ul != 1ul){
            }
        }
        *(uint32_t volatile *)0x4006EC14ul = 0xC0ul;
    }
}

/*lint --flb "Leave library region" */
