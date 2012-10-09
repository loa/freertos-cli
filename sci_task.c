#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_semphr.h"

#include "sci_task.h"

xSemaphoreHandle xSciMUTEX;

xQueueHandle xSciQueue;
xQueueHandle xDebugQueue;

/*
 * Halcogen sciInit() needs to be run first for sciTasks to function
 */
void startSciTasks()
{
	// sciInit();

	xSciMUTEX = xSemaphoreCreateMutex();

	xSciQueue = xQueueCreate(5, 81 * sizeof(uint8_t));
	xDebugQueue = xQueueCreate(5, 81 * sizeof(uint8_t));
 
 	xTaskCreate (vSciDebugTask, (signed char*) "SCIDBG", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
 	xTaskCreate (vSciConsoleTask, (signed char*) "SCICLI", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
}

void vSciConsoleTask(void *pvParameters)
{
	char msg[81], msg_size;
	int endoftext;

	for ( ;; )
	{
		endoftext = 0;

		for (msg_size = 0; msg_size < 81; msg_size++)
		{
			msg[msg_size] = sciReceiveByte(scilinREG);	

			// Remove incase we recieve backspace
			if (msg[msg_size] == 8)
			{
				msg_size -= 2;
			}

			if (msg[msg_size] == '\n')
			{
				msg[msg_size] = '\0';
				break;
			}
		}

		// Run shell
		runShellCommand(&xSciQueue, msg);

		// Get Mutex for SCI
		if (xSemaphoreTake(xSciMUTEX, portMAX_DELAY) == pdTRUE)
		{
			// Send out functions text
			while (endoftext == 0)
			{
				if (xQueueReceive(xSciQueue, &msg, ( portTickType ) 1000) == pdPASS) {
					// Check size of string
					for (msg_size = 0; msg_size < 81; msg_size++)
					{
						if (msg[msg_size] == '\3')
						{
							endoftext = 1;
							msg[msg_size] = '\0';
						}

						if (msg[msg_size] == '\0') break;
					}

					if (msg_size == 0) break;
					
					// Display msg
					sciSend(scilinREG, msg_size, (uint8_t *) msg);
				}
				else
				{
					// ERROR
					ledOn(8);
				}
			}

			xSemaphoreGive(xSciMUTEX);
		}
	}
}

void vSciDebugTask(void *pvParameters)
{
	uint8_t msg[81], msg_size;

	for ( ;; )
	{
		if (xQueueReceive(xDebugQueue, &msg, portMAX_DELAY) == pdPASS)
		{
			// Get Mutex for SCI
			if( xSemaphoreTake(xSciMUTEX, portMAX_DELAY) == pdTRUE)
			{
				// Check size of string
				for (msg_size = 1; msg_size < 81; msg_size++)
				{
					if (msg[msg_size] == '\0') break;
				}

			
				// Display msg
				sciSend(scilinREG, msg_size, msg);

				xSemaphoreGive(xSciMUTEX);
			}
		}
	}
}

void sciPrint(char *format, ...)
{
	char str[81];

	va_list args;
	va_start(args, format);
	
	vsprintf(str, format, args);
	
	va_end(args);

	if(xQueueSendToBack(xSciQueue, str, portMAX_DELAY) != pdPASS)
	{
		// Handle error
	}
}

void sciDebug(char *format, ...)
{
	char str[81];

	va_list args;
	va_start(args, format);

	vsprintf(str, format, args);

	va_end(args);

	if(xQueueSendToBack(xDebugQueue, str, portMAX_DELAY) != pdPASS)
	{
		// Handle error
	}
}
