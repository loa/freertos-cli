#ifndef SCI_TASK_H
#define SCI_TASK_H

#include <stdarg.h>

void startSciTasks();
void vSciConsoleTask(void *pvParameters);
void vSciDebugTask(void *pvParameters);

void sciPrint(char *text, ...);
void sciDebug(char *text, ...);

#endif
