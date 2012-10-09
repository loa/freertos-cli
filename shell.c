#include "shell.h"

typedef struct xCOMMAND_LIST_ITEM
{
	shellCommand * command;
	struct xCOMMAND_LIST_ITEM * next;
} xCommandListItem;

static void helpCommand(xQueueHandle *outputQueue, char *line);
void echoCommand(xQueueHandle *outputQueue, char *line);

static shellCommand xHelpCommand =
{
	(char *) "help",
	(char *) "help: Lists all available command\n\r",
	&helpCommand
};

static xCommandListItem xRegisteredCommands =
{
	&xHelpCommand,
	NULL
};

void initShell()
{
	shellCommand * cmd = (shellCommand *) pvPortMalloc(sizeof(shellCommand));

	cmd->name = "echo";
	cmd->help = "echo: Echo text\n\r";
	cmd->func = &echoCommand;

	registerShellCommand(cmd);
}

void registerShellCommand(shellCommand * newCommand)
{
	xCommandListItem * item = &xRegisteredCommands;
	xCommandListItem * newItem = (xCommandListItem *) pvPortMalloc(sizeof(xCommandListItem));

	taskENTER_CRITICAL();

	// Get to the last item in list
	while (item->next != NULL)
	{
		item = item->next;
	}

	// Create list item
	newItem->command = newCommand;
	newItem->next = NULL;

	// Add item to list
	item->next = newItem;

	taskEXIT_CRITICAL();
}

void runShellCommand(xQueueHandle *outputQueue, char * line)
{
	xCommandListItem * item = &xRegisteredCommands;
	char * strNotFound = "Command not found, use \"help\" to list available commands.\n\r\3";
	int len;
	while (item != NULL)
	{
		len = strlen(item->command->name);
		// Either line contains only the command or followed by space
		if (strncmp(line, item->command->name, len) == 0 && (len == strlen(line) || line[len] == ' '))
		{
			(*item->command->func)(outputQueue, line);

			break;
		}
		item = item->next;
	}

	if (item == NULL)
	{
		// Command was not found
		if(xQueueSendToBack(*outputQueue, (void *) strNotFound, (portTickType) 100) != pdPASS)
		{
			sciDebug("ERROR %s:%d\n\r", __FUNCTION__, __LINE__);
		}
	}
}

void helpCommand(xQueueHandle *outputQueue, char *line)
{
	char str[81];
	int str_i, help_i, found_end;
	xCommandListItem * item = &xRegisteredCommands;

	// Set last char as NUL
	str[80] = '\0';

	// Print all command names
	while (item != NULL)
	{
		if (strlen(item->command->help) < 81)
		{
			if(xQueueSendToBack(*outputQueue, (void *) item->command->help, (portTickType) 100) != pdPASS)
			{
				sciDebug("ERROR %s:%d\n\r", __FUNCTION__, __LINE__);
			}
		}
		else
		{
			help_i = 0;
			found_end = 0;

			// Help text longer than 81 chars, split it up 
			while (found_end == 0)
			{
				// Get next partition of the string
				for (str_i = 0; str_i < 80; str_i++)
				{
					str[str_i] = item->command->help[help_i++];

					if (str[str_i] == '\0')
					{
						found_end = 1;
						break;
					}
				}

				// Print partition
				if(xQueueSendToBack(*outputQueue, (void *) str, (portTickType) 100) != pdPASS)
				{
					sciDebug("ERROR %s:%d\n\r", __FUNCTION__, __LINE__);
				}
			}
		}

		item = item->next;
	}

	// Send End-of-Text
	sprintf(str, "\3");

	if(xQueueSendToBack(*outputQueue, (void *) str, (portTickType) 100) != pdPASS)
	{
		sciDebug("ERROR %s:%d\n\r", __FUNCTION__, __LINE__);
	}
}

void echoCommand(xQueueHandle *outputQueue, char *line)
{
	char str[81] = "\n\r\3";

	int i = findchar(line, ' ');

	// Incase space after "echo" isnt found or there are no text
	if (i == -1 || line[i+1] == '\0')
	{
		if(xQueueSendToBack(*outputQueue, (void *) str, (portTickType) 100) != pdPASS)
		{
			sciDebug("ERROR %s:%d\n\r", __FUNCTION__, __LINE__);
		}
	}
	else
	{
		sprintf(str, "%s\n\r\3", &line[i+1]);

		if(xQueueSendToBack(*outputQueue, (void *) str, (portTickType) 100) != pdPASS)
		{
			sciDebug("ERROR %s:%d\n\r", __FUNCTION__, __LINE__);
		}
	}
}

