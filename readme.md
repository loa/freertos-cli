## FreeRTOS CLI

CLI handler and SCI task for FreeRTOS crafted to work alongside a generated Texas Instruments Halcogen project. Can easily be used in any FreeRTOS environment with modification.


### SCI generated from Halcogen

__Cons:__

- int8_t arrays
- Not "thread safe"
- Ugly _(to much code for simple outputs)_

__Example:__

    int8_t ERROR[] = {'E', 'r', 'r', 'o', 'r', ':', ' ', 'm', 'a', 'i', 'm', '\n', '\r'};
    int8_t NEWLINE[] = {'\n', '\r'};

    printArray(ERROR, sizeof(ERROR));
    printArray(NEWLINE, sizeof(NEWLINE));

__Prefered way:__

    sciDebug("Error: %s\n\r", __FUNCTION__); 

### FreeRTOS CLI

Official FreeRTOS Cli had for me a strange way of working with the cli functions. Cli function got called with both input and output strings in argument which is strait forward but to get multiple rows the cli functions are supposed to return FALSE _(all rows printed?)_. Then the cli function gets called again and again until all rows are printed _(cli function returns true)_. 

__Cons:__

- Not "thread safe"
- Cli functions need to keep track of state _(which line to output in ls?)_
- Complexity in cli functions _(not in cli handler)_

__Example:__

    bool ls(char* input, char* output) {
    	return false;
    }

I didn't want to implement this since I thought it would spread out alot of bugs, issues and complexity in my cli functions since they need to keep track themselves where they are _(which row in file list in ls)_. It also made it impossible to use same CLI functions at the same time from multiple sources _(sci, telnet, etc)_.

### Current state

Simple implementation inspired by FreeRTOS Cli but by using FreeRTOS queues it's "thread safe" and much easier to implement functions that are used from different sources at the same time.

__Pros:__
- "Thread safe"
- Cli functions get called ones each execution
- Complexity is handled in the CLI handler and the SCI task.

#### Example

Cli function get called once, sends all rows to Queue and sends an _'\3' end of message_ char before returning.

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

### Known issues

- Doesn't handle input longer than 80 chars.