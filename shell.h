#ifndef SHELL_H
#define SHELL_H

#include "FreeRTOS.h"
#include "os_queue.h"

typedef void (*shellFunction) (xQueueHandle *outputQueue, char *args);

typedef struct SHELL_COMMAND
{
	char * name;
	char * help;
	shellFunction func;
} shellCommand;

void initShell();
void registerShellCommand();
void runShellCommand(xQueueHandle *outputQueue, char * line);

#endif
