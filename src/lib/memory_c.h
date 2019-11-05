#ifndef MEMORY_C_H
#define MEMORY_C_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
    uint32_t virtualMem;
    uint32_t physicalMem;
} processMem_t;

int parseLine(char *line)
{
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char *p = line;
    while (*p < '0' || *p > '9')
        p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
}

void getProcessMemory(processMem_t *processMem)
{
    FILE *file = fopen("/proc/self/status", "r");
    char line[128];

    while (fgets(line, 128, file) != NULL)
    {
        //printf("%s", line);
        if (strncmp(line, "VmSize:", 7) == 0)
        {
            processMem->virtualMem = parseLine(line);
        }

        if (strncmp(line, "VmRSS:", 6) == 0)
        {
            processMem->physicalMem = parseLine(line);
        }
    }
    fclose(file);
}

#endif

/* Example use:
int main()
{
	int i;
	processMem_t myMem;

	getProcessMemory(&myMem);

	printf("Memory: vMem %u KB, pMem %u KB\n", myMem.virtualMem, myMem.physicalMem);
}
*/