#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <rp2040_config.h>

#define vPortSVCHandler         isr_svcall      // Map SVC handler to isr_svcall
#define xPortPendSVHandler      isr_pendsv      // Map PendSV handler to isr_pendsv
#define xPortSysTickHandler     isr_systick     // Map SysTick handler to isr_systick

/*-----------------------------------------------------------
 * Application specific definitions.
 *----------------------------------------------------------*/

/* Scheduler Related */
#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                 1       // Enable tickless idle mode for power saving
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      125000000 // 125MHz for RP2040
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 ) // 1ms tick rate
#define configMAX_PRIORITIES                    32      // Increase max priorities to 32
#define configMINIMAL_STACK_SIZE                ( configSTACK_DEPTH_TYPE ) 128
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/* Synchronization Related */
#define configUSE_MUTEXES                       1       // Enable mutexes
#define configUSE_RECURSIVE_MUTEXES             1       // Enable recursive mutexes
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1       // Enable counting semaphores
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    1       // Enable queue sets
#define configUSE_TIME_SLICING                  1       // Enable time slicing
#define configUSE_NEWLIB_REENTRANT              0

#define configENABLE_BACKWARD_COMPATIBILITY     1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

/* System */
#define configSTACK_DEPTH_TYPE                  uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         0       // Disable static allocation
#define configSUPPORT_DYNAMIC_ALLOCATION        1       // Enable dynamic allocation
#define configTOTAL_HEAP_SIZE                   (146 * 1024) // Set total heap size to 146KB
#define configAPPLICATION_ALLOCATED_HEAP        0

/* Hook function related definitions. */
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           1       // Enable run-time stats generation
#define configUSE_TRACE_FACILITY                1       // Enable trace facility
#define configUSE_STATS_FORMATTING_FUNCTIONS    1       // Enable stats formatting functions

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1       // Enable software timers
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 ) // High priority for timer task
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            256



#if FREE_RTOS_KERNEL_SMP // set by the RP2040 SMP port of FreeRTOS
/* SMP port only */
#define configNUMBER_OF_CORES                   2       // Define number of cores for SMP
#define configTICK_CORE                         0
#define configRUN_MULTIPLE_PRIORITIES           0
#define configUSE_CORE_AFFINITY                 1       // Enable core affinity
#endif

#define configUSE_PASSIVE_IDLE_HOOK             0

/* RP2040 specific */
#define configSUPPORT_PICO_SYNC_INTEROP         1       // Enable Pico SDK sync interop
#define configSUPPORT_PICO_TIME_INTEROP         1       // Enable Pico SDK time interop

#include <assert.h>
/* Define to trap errors during development. */
#define configASSERT(x)                         assert(x) // Use standard assert for configASSERT

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  1
#define INCLUDE_xTaskResumeFromISR              1
#define INCLUDE_xQueueGetMutexHolder            1

/* A header file that defines trace macro can be included here. */

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()

#ifdef __cplusplus
extern "C" {
#endif

uint64_t time_us_64();  // "hardware/timer.h"

#define RUN_TIME_STAT_time_us_64Divider 1000            // Stat granularity is mS
#define portGET_RUN_TIME_COUNTER_VALUE() (time_us_64()/RUN_TIME_STAT_time_us_64Divider)  // Runtime counter in mS

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_CONFIG_H */
