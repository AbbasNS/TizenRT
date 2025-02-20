/*
 * Copyright (C) 2020 Realtek Semiconductor Corp.  All Rights Reserved.
 */

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <tinyara/config.h>
#include <tinyara/mpu.h>
#include <tinyara/mm/heap_regioninfo.h>
/* Scheduler includes. */
#ifndef CONFIG_PLATFORM_TIZENRT_OS
#include "FreeRTOS.h"
#include "task.h"     /* RTOS task related API prototypes. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */
#endif

#include "ameba_soc.h"

#ifndef CONFIG_PLATFORM_TIZENRT_OS
extern int main(void);
extern void arm_gic_set_CUTVersion(uint32_t CUTVersion);
#endif

#if defined (__GNUC__)
/* Add This for C++ support to avoid compile error */
void _init(void) {}
#endif

#ifdef CONFIG_PLATFORM_TIZENRT_OS
extern unsigned int _sidle_stack;
extern unsigned int _psram_heap_start;
extern unsigned int __PsramStackLimit;
//void NS_ENTRY BOOT_IMG3(void);
//void app_init_psram(void);

#define IDLE_STACK ((uintptr_t)&_sidle_stack + CONFIG_IDLETHREAD_STACKSIZE - 4)
#define PSRAM_HEAP_BASE ((uintptr_t)&_psram_heap_start)
#define PSRAM_HEAP_LIMIT ((uintptr_t)&__PsramStackLimit)

// const uintptr_t g_idle_topstack = IDLE_STACK;

void os_heap_init(void){
	kregionx_start[0] = (void *)PSRAM_HEAP_BASE;
	kregionx_size[0] = (size_t)(PSRAM_HEAP_LIMIT - PSRAM_HEAP_BASE);
#if CONFIG_KMM_REGIONS >= 2
#if CONFIG_KMM_REGIONS == 3
	kregionx_start[1] = (void *)PSRAM_HEAP_BASE;
	kregionx_size[1] = (size_t)kregionx_start[1] + kregionx_size[1] - PSRAM_HEAP_BASE;

	kregionx_start[2] = (void *)kregionx_start[2];
	kregionx_size[2] = (size_t)PSRAM_HEAP_LIMIT - (size_t)kregionx_start[2];
#elif CONFIG_KMM_REGIONS > 3
#error "Need to check here for heap."
#else
	kregionx_start[1] = (void *)PSRAM_HEAP_BASE;
	kregionx_size[1] = (size_t)(PSRAM_HEAP_LIMIT - PSRAM_HEAP_BASE);
#endif
#endif
}
#endif

/*
 * Starts all the other tasks, then starts the scheduler.
 */
void app_start(void)
{
#ifndef CONFIG_PLATFORM_TIZENRT_OS
	/* GIC need know current Cut Version */
	arm_gic_set_CUTVersion(SYSCFG_RLVersion());
#endif

	RTK_LOG_MASK(LEVEL_ERROR, 0xFFFFFFFF);

	os_heap_init();

#ifndef CONFIG_PLATFORM_TIZENRT_OS
#if defined (__GNUC__)
	extern void __libc_init_array(void);
	/* Add This for C++ support */
	__libc_init_array();
#endif
#endif

#ifndef CONFIG_PLATFORM_TIZENRT_OS
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();
#endif

	/* Get flash_init_para info for AP */
	_memcpy((void *)&flash_init_para, (const void *)HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_LSYS_FLASH_PARA_ADDR), sizeof(FLASH_InitTypeDef));

	flash_layout_init();

#ifndef CONFIG_PLATFORM_TIZENRT_OS
	main();
#endif
#if defined( __ICCARM__ )
	__iar_data_init3();
#endif
}
