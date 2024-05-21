#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTALRAM 2048
#define CPU1 512
#define CPU2 (TOTALRAM - CPU1)
#define QUANTUM_HIGH_PRIORITY 8
#define QUANTUM_MEDIUM_PRIORITY 16

typedef struct Process {
    char name[10];
    int arrival_time;
    int priority;
    int burst_time;
    int ram_required;
    int cpu_usage;
    int remaining_time;
    struct Process *next;
} Process;

typedef struct {
    Process *front;
    Process *rear;
} Queue;



int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s input.txt\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        printf("Error opening input file.\n");
        return 1;
    }

    Process *processes[200];
    int process_count = 0;
    char line[100];

    while (fgets(line, sizeof(line), input_file) != NULL) {
        Process *new_process = (Process *)malloc(sizeof(Process));
        
        if (new_process == NULL) {
            printf("Memory allocation failed.\n");
            return 1;
        }

        sscanf(line, "%[^,],%d,%d,%d,%d,%d",
               new_process->name,
               &new_process->arrival_time,
               &new_process->priority,
               &new_process->burst_time,
               &new_process->ram_required,
               &new_process->cpu_usage);

        new_process->remaining_time = new_process->burst_time;
        new_process->next = NULL;

        processes[process_count++] = new_process;
    }

    fclose(input_file);

    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Error opening output file.\n");
        return 1;
    }

    int ram_available = TOTALRAM;
    Queue waiting_queue;
    initialize_queue(&waiting_queue);

    fcfs_scheduler(processes, process_count, output_file, &waiting_queue, &ram_available);
    sjf_scheduler(processes, process_count, output_file, &waiting_queue, &ram_available);
    rr_scheduler(processes, process_count, QUANTUM_HIGH_PRIORITY, output_file, &waiting_queue, &ram_available);
    rr_scheduler(processes, process_count, QUANTUM_MEDIUM_PRIORITY, output_file, &waiting_queue, &ram_available);

    fclose(output_file);

    for (int i = 0; i < process_count; i++) {
        free(processes[i]);
    }

    return 0;
}
