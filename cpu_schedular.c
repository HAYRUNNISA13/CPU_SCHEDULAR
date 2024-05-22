#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTALRAM 2048
#define CPU1 512
#define CPU2 (TOTALRAM - CPU1)
#define QUANTUM_HIGH_PRIORITY 8
#define QUANTUM_MEDIUM_PRIORITY 16

// Define the structure for a process
typedef struct Process {
    char name[10];          // Process name
    int arrival_time;       // Arrival time of the process
    int priority;           // Priority of the process
    int burst_time;         // Burst time (execution time) of the process
    int ram_required;       // RAM required by the process
    int cpu_usage;          // CPU usage of the process
    int remaining_time;     // Remaining time to complete the process
    struct Process *next;   // Pointer to the next process in the queue
} Process;

// Define a queue structure for managing processes
typedef struct {
    Process *front;     // Pointer to the front of the queue
    Process *rear;      // Pointer to the rear of the queue
} Queue;

// Function to initialize a queue
void initialize_queue(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
}

// Function to check if a queue is empty
int is_queue_empty(Queue *q) {
    return q->front == NULL;
}

// Function to enqueue a process into the queue
void enqueue(Queue *q, Process *p, FILE *output_file) {
    if (is_queue_empty(q)) {
        // If the queue is empty, set both front and rear to the new process
        q->front = p;
        q->rear = p;
        p->next = NULL;
    } else {
        // Otherwise, append the new process to the rear of the queue
        q->rear->next = p;
        q->rear = p;
        p->next = NULL;
    }
    // Log the enqueue operation
    fprintf(output_file, "Process %s is queued due to insufficient RAM.\n", p->name);
}

// Function to dequeue a process from the queue
Process *dequeue(Queue *q) {
    if (!is_queue_empty(q)) {
        // If the queue is not empty, remove the front process and adjust pointers
        Process *temp = q->front;
        q->front = q->front->next;
        if (q->front == NULL) {
            q->rear = NULL;
        }
        return temp;
    } else {
        // If the queue is empty, return NULL
        return NULL;
    }
}

// Function to release RAM after process completion
void release_ram(Process *p, int *ram_available, FILE *output_file) {
    *ram_available += p->ram_required;
    // Log the release of RAM
    fprintf(output_file, "Process %s releases RAM.\n", p->name);
}

// Function to print the assignment of a process to a CPU
void print_process_assigned(Process *p, int cpu, FILE *output_file) {
    // Log the assignment of the process to a CPU
    fprintf(output_file, "Process %s is assigned to CPU-%d.\n", p->name, cpu);
}

// Function to print completion of a process
void print_process_completed(Process *p, FILE *output_file) {
    // Log the completion of the process
    fprintf(output_file, "Process %s is completed and terminated.\n", p->name);
}

// Function to sort processes by arrival time
void sort_processes_by_arrival(Process *processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j]->arrival_time > processes[j + 1]->arrival_time) {
                // Swap processes if they are out of order based on arrival time
                Process *temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

// Function to sort processes by burst time
void sort_processes_by_burst_time(Process *processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j]->burst_time > processes[j + 1]->burst_time) {
                // Swap processes if they are out of order based on burst time
                Process *temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

// Current time for CPU1 and CPU2
int current_time_cpu1 = 0;
int current_time_cpu2 = 0;

// Function to check RAM availability for a process
int check_ram_availability(Process *process, int ram_available) {
    return process->ram_required <= ram_available;
}

// Function to assign a process to a CPU
void assign_process(Process *p, int cpu, int *ram_available, FILE *output_file) {
    *ram_available -= p->ram_required;

    // Determine the start time of the process
    int start_time = (cpu == 1) ? current_time_cpu1 : current_time_cpu2;

    // Print assignment and completion times of the process
    fprintf(output_file, "Process %s is assigned to CPU-%d.\n", p->name, cpu);
    fprintf(output_file, "Process %s starts at time %d on CPU-%d.\n", p->name, start_time, cpu);
    if (cpu == 1) {
        current_time_cpu1 = start_time + p->burst_time;
    } else {
        current_time_cpu2 = start_time + p->burst_time;
    }
    fprintf(output_file, "Process %s completes at time %d.\n", p->name, start_time + p->burst_time);
    print_process_completed(p, output_file);
    release_ram(p, ram_available, output_file);
}

// Function to handle insufficient RAM for a process
void handle_insufficient_ram(Process *p, Queue *waiting_queue, FILE *output_file) {
    // Log that the process could not be assigned due to insufficient RAM
    fprintf(output_file, "Process %s could not be assigned due to insufficient RAM.\n", p->name);
    // Enqueue the process into the waiting queue
    enqueue(waiting_queue, p, output_file);
}

// Function to assign processes to CPUs based on RAM availability and priority
void assign_processes(Process *processes[], int n, Queue *waiting_queue, int *ram_available, FILE *output_file) {
    int cpu1_ram = CPU1;
    int cpu2_ram = CPU2;

    for (int i = 0; i < n; i++) {
        Process *p = processes[i];
        if (check_ram_availability(p, *ram_available)) {
            if (p->priority == 0) {
                if (cpu1_ram >= p->ram_required) {
                    // If there is enough RAM on CPU1, assign the process to CPU1
                    assign_process(p, 1, &cpu1_ram, output_file);
                    cpu1_ram -= p->ram_required;
                } else if (cpu2_ram >= p->ram_required) {
                    // If there is enough RAM on CPU2, assign the process to CPU2
                    assign_process(p, 2, &cpu2_ram, output_file);
                    cpu2_ram -= p->ram_required;
                } else {
                    // If there is insufficient RAM on both CPUs, handle the situation
                    handle_insufficient_ram(p, waiting_queue, output_file);
                }
            } else {
                // For non-priority-0 processes, assign them to CPU2
                assign_process(p, 2, &cpu2_ram, output_file);
                cpu2_ram -= p->ram_required;
            }
        } else {
            // If there is insufficient RAM for the process, handle it
            handle_insufficient_ram(p, waiting_queue, output_file);
        }
    }
}

// Function to perform FCFS scheduling for priority 0 processes on CPU1
void fcfs_scheduler(Process *processes[], int n, FILE *output_file, Queue *waiting_queue, int *ram_available) {
    printf("CPU-1 que1(priority-0)(FCFS):");
    for (int i = 0; i < n; i++) {
        if (processes[i]->priority == 0) {
            // Assign processes with priority 0 to CPU1
            assign_processes(&processes[i], 1, waiting_queue, ram_available, output_file);
            printf("%s", processes[i]->name); // Print process name
            if (i < n - 1 && processes[i + 1]->priority == 0) {
                printf("-");
            }
        }
    }
    printf("\n");
}

void sjf_scheduler(Process *processes[], int n, FILE *output_file, Queue *waiting_queue, int *ram_available)
{
    // Print the Scheduler Information
    printf("CPU-2 que2(priority-1) (Sjf):");

    // Initialize Variables
    Process *priority1_processes[200]; // Array to store high priority processes
    int priority1_count = 0; // Count of high priority processes

    // Loop Through Processes to Select High Priority Processes
    for (int i = 0; i < n; i++)
    {
        if (processes[i]->priority == 1)
        {
            priority1_processes[priority1_count++] = processes[i];
        }
    }

    // Execute the SJF Algorithm
    int current_time = 0; // Current time
    int completed = 0; // Count of completed high priority processes

    while (completed < priority1_count)
    {
        int index = -1; // Index of the process with the shortest burst time

        // Find the process with the shortest burst time among the high priority processes
        for (int i = 0; i < priority1_count; i++)
        {
            if (priority1_processes[i]->arrival_time <= current_time &&
                priority1_processes[i]->remaining_time > 0)
            {
                if (index == -1 || priority1_processes[i]->burst_time < priority1_processes[index]->burst_time)
                {
                    index = i;
                }
            }
        }

        // Assign Process to CPU and Update Time
        if (index != -1)
        {
            Process *p = priority1_processes[index];
            if (check_ram_availability(p, *ram_available))
            {
                int start_time = current_time;
                current_time += p->burst_time;
                p->remaining_time = 0;
                *ram_available -= p->ram_required;

                fprintf(output_file, "Process %s is assigned to CPU-2.\n", p->name);
                fprintf(output_file, "Process %s starts at time %d on CPU-2.\n", p->name, start_time);
                fprintf(output_file, "Process %s completes at time %d.\n", p->name, current_time);
                print_process_completed(p, output_file);
                release_ram(p, ram_available, output_file);

                completed++;
                printf("-%s", p->name);
            }
            else
            {
                handle_insufficient_ram(p, waiting_queue, output_file);
            }
        }
        else
        {
            current_time++; // No suitable process found, increment current time
        }
    }

    // Print New Line
    printf("\n");
}

// Function to perform Round Robin scheduling for priority 2 and 3 processes on CPU2
void rr_scheduler(Process *processes[], int n, int quantum, FILE *output_file, Queue *waiting_queue, int *ram_available) {
    printf("CPU-2 que3(priority-2) (RR-q8):");
    int *remaining_burst = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        remaining_burst[i] = processes[i]->burst_time;
    }

    while (1) {
        int all_completed = 1;

        for (int i = 0; i < n; i++) {
            if (remaining_burst[i] > 0 && (processes[i]->priority == 2 || processes[i]->priority == 3)) {
                all_completed = 0;

                if (remaining_burst[i] <= quantum) {
                    // If the remaining burst time is less than or equal to the quantum, assign and complete the process
                    if ((quantum == 8 && processes[i]->priority == 2) ||
                        (quantum == 16 && processes[i]->priority == 3)) {
                        assign_processes(&processes[i], 2, waiting_queue, ram_available, output_file);
                        remaining_burst[i] = 0;
                        release_ram(processes[i], ram_available, output_file);
                        printf("%s", processes[i]->name); // Print process name
                        if (i < n - 1 && (processes[i + 1]->priority == 2 || processes[i + 1]->priority == 3)) {
                            printf("-");
                        }
                    }
                    remaining_burst[i] = 0;
                } else {
                    // If the remaining burst time is greater than the quantum, assign and enqueue the process again
                    if ((quantum == 8 && processes[i]->priority == 2) ||
                        (quantum == 16 && processes[i]->priority == 3)) {
                        assign_processes(&processes[i], 2, waiting_queue, ram_available, output_file);
                        enqueue(waiting_queue, processes[i], output_file);
                        fprintf(output_file, "Process %s ran until the defined quantum time and is queued again because the process is not completed.\n", processes[i]->name);
                        printf("%s-", processes[i]->name); // Print process name
                    }
                }
                remaining_burst[i] -= quantum;
            }
        }

        if (all_completed) {
            break;
        }
    }

    free(remaining_burst);
    printf("\n");
}

// Main function
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

    // Read processes from input file and store them in an array
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

    // Assign processes to CPUs
    assign_processes(processes, process_count, &waiting_queue, &ram_available, output_file);

    // Perform scheduling algorithms
    fcfs_scheduler(processes, process_count, output_file, &waiting_queue, &ram_available);
    sjf_scheduler(processes, process_count, output_file, &waiting_queue, &ram_available);
    rr_scheduler(processes, process_count, QUANTUM_HIGH_PRIORITY, output_file, &waiting_queue, &ram_available);
    rr_scheduler(processes, process_count, QUANTUM_MEDIUM_PRIORITY, output_file, &waiting_queue, &ram_available);

    fclose(output_file);

    // Free memory allocated for processes
    for (int i = 0; i < process_count; i++) {
        free(processes[i]);
    }

    return 0;
}


