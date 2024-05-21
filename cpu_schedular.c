#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define constants for RAM and CPU allocation
#define TOTALRAM 2048             // Total amount of RAM available
#define CPU1 512                  // Amount of RAM allocated to CPU1
#define CPU2 (TOTALRAM - CPU1)    // Amount of RAM allocated to CPU2
#define QUANTUM_HIGH_PRIORITY 8   // Quantum time for high priority processes in round-robin scheduling
#define QUANTUM_MEDIUM_PRIORITY 16 // Quantum time for medium priority processes in round-robin scheduling

// Define a structure to represent a process
typedef struct Process {
    char name[10];          // Process name
    int arrival_time;       // Time at which the process arrives in the system
    int priority;           // Priority level of the process
    int burst_time;         // CPU time required by the process
    int ram_required;       // Amount of RAM required by the process
    int cpu_usage;          // CPU usage of the process (not used in this code)
    int remaining_time;     // Remaining burst time for the process (used for round-robin scheduling)
    struct Process *next;   // Pointer to the next process in the queue
} Process;

// Define a structure for a queue to manage processes
typedef struct {
    Process *front;  // Pointer to the front of the queue
    Process *rear;   // Pointer to the rear of the queue
} Queue;

// Function to initialize a queue
void initialize_queue(Queue *q) {
    q->front = NULL;  // Set front to NULL, indicating the queue is empty
    q->rear = NULL;   // Set rear to NULL, indicating the queue is empty
}

// Function to check if a queue is empty
int is_queue_empty(Queue *q) {
    return q->front == NULL;  // Return 1 (true) if the queue is empty, otherwise return 0 (false)
}

// Function to add a process to the end of a queue
void enqueue(Queue *q, Process *p, FILE *output_file) {
    if (is_queue_empty(q)) {
        // If the queue is empty, set both front and rear to the new process
        q->front = p;
        q->rear = p;
        p->next = NULL;  // The new process points to NULL as it is the only process in the queue
    } else {
        // If the queue is not empty, add the new process to the end and update the rear pointer
        q->rear->next = p;
        q->rear = p;
        p->next = NULL;  // The new process points to NULL as it is now the last process in the queue
    }
    // Log the queuing of the process due to insufficient RAM
    fprintf(output_file, "Process %s is queued due to insufficient RAM.\n", p->name);
}

// Function to remove a process from the front of a queue
Process *dequeue(Queue *q) {
    if (!is_queue_empty(q)) {
        // If the queue is not empty, remove the process from the front
        Process *temp = q->front;  // Temporarily store the front process
        q->front = q->front->next; // Update the front pointer to the next process
        if (q->front == NULL) {
            // If the queue is now empty, update the rear pointer to NULL
            q->rear = NULL;
        }
        return temp;  // Return the removed process
    } else {
        return NULL;  // Return NULL if the queue is empty
    }
}

// Function to release the RAM occupied by a process
void release_ram(Process *p, int *ram_available, FILE *output_file) {
    *ram_available += p->ram_required;  // Add the RAM required by the process back to the available RAM
    fprintf(output_file, "Process %s releases RAM.\n", p->name);  // Log the release of RAM
}

// Function to print the assignment of a process to a CPU
void print_process_assigned(Process *p, int cpu, FILE *output_file) {
    fprintf(output_file, "Process %s is assigned to CPU-%d.\n", p->name, cpu);  // Log the assignment of the process to the specified CPU
}

// Function to print the completion of a process
void print_process_completed(Process *p, FILE *output_file) {
    fprintf(output_file, "Process %s is completed and terminated.\n", p->name);  // Log the completion and termination of the process
}

// Function to sort an array of processes by their arrival time using bubble sort
void sort_processes_by_arrival(Process *processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j]->arrival_time > processes[j + 1]->arrival_time) {
                // Swap processes[j] and processes[j + 1] if they are out of order
                Process *temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

// Function to sort an array of processes by their burst time using bubble sort
void sort_processes_by_burst_time(Process *processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j]->burst_time > processes[j + 1]->burst_time) {
                // Swap processes[j] and processes[j + 1] if they are out of order
                Process *temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

// Global variable to keep track of the current time in the system
int current_time = 0;

// Function to check if there is enough RAM available for a process
int check_ram_availability(Process *process, int ram_available) {
    return process->ram_required <= ram_available;  // Return true if the process's RAM requirement is less than or equal to available RAM
}

// Function to assign a process to a CPU and update the current time and RAM availability
void assign_process(Process *p, int cpu, int *ram_available, FILE *output_file) {
    *ram_available -= p->ram_required;  // Subtract the RAM required by the process from the available RAM
    print_process_assigned(p, cpu, output_file);  // Log the assignment of the process to the specified CPU
    fprintf(output_file, "Process %s starts at time %d.\n", p->name, current_time);  // Log the start time of the process
    current_time += p->burst_time;  // Update the current time by adding the burst time of the process
    fprintf(output_file, "Process %s completes at time %d.\n", p->name, current_time);  // Log the completion time of the process
    print_process_completed(p, output_file);  // Log the completion and termination of the process
    release_ram(p, ram_available, output_file);  // Release the RAM occupied by the process
}

// Function to handle a process that cannot be assigned due to insufficient RAM
void handle_insufficient_ram(Process *p, Queue *waiting_queue, FILE *output_file) {
    fprintf(output_file, "Process %s could not be assigned due to insufficient RAM.\n", p->name);  // Log the failure to assign the process due to insufficient RAM
    enqueue(waiting_queue, p, output_file);  // Add the process to the waiting queue
}

// Function to assign processes based on RAM availability and CPU allocation
void assign_processes(Process *processes[], int n, Queue *waiting_queue, int *ram_available, FILE *output_file) {
    for (int i = 0; i < n; i++) {
        Process *p = processes[i];  // Get the next process
        if (check_ram_availability(p, *ram_available)) {
            // If there is enough RAM available, assign the process to a CPU based on its priority
            if (p->priority == 0) {
                assign_process(p, 1, ram_available, output_file);  // Assign high priority processes to CPU-1
            } else {
                assign_process(p, 2, ram_available, output_file);  // Assign lower priority processes to CPU-2
            }
        } else {
            handle_insufficient_ram(p, waiting_queue, output_file);  // Handle processes with insufficient RAM
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        // Print usage information if the correct number of arguments is not provided
        printf("Usage: %s input.txt\n", argv[0]);
        return 1;  // Exit with an error code
    }

    FILE *input_file = fopen(argv[1], "r");  // Open the input file for reading
    if (input_file == NULL) {
        // Print an error message if the input file cannot be opened
        printf("Error opening input file.\n");
        return 1;  // Exit with an error code
    }

    Process *processes[200];  // Array to store pointers to processes
    int process_count = 0;    // Counter for the number of processes
    char line[100];           // Buffer for reading lines from the input file

    // Read processes from the input file
    while (fgets(line, sizeof(line), input_file) != NULL) {
        Process *new_process = (Process *)malloc(sizeof(Process));  // Allocate memory for a new process
        if (new_process == NULL) {
            // Print an error message if memory allocation fails
            printf("Memory allocation failed.\n");
            return 1;  // Exit with an error code
        }

        // Parse the input line and populate the process structure
        sscanf(line, "%[^,],%d,%d,%d,%d,%d",
               new_process->name,
               &new_process->arrival_time,
               &new_process->priority,
               &new_process->burst_time,
               &new_process->ram_required,
               &new_process->cpu_usage);

        new_process->remaining_time = new_process->burst_time;  // Initialize remaining time to burst time
        new_process->next = NULL;  // Initialize next pointer to NULL

        processes[process_count++] = new_process;  // Add the new process to the array
    }

    fclose(input_file);  // Close the input file

    FILE *output_file = fopen("output.txt", "w");  // Open the output file for writing
    if (output_file == NULL) {
        // Print an error message if the output file cannot be opened
        printf("Error opening output file.\n");
        return 1;  // Exit with an error code
    }

    int ram_available = TOTALRAM;  // Initialize available RAM to the total amount of RAM
    Queue waiting_queue;  // Declare a queue for processes waiting for RAM
    initialize_queue(&waiting_queue);  // Initialize the waiting queue

    // Example scheduler functions (these need to be implemented)
     fcfs_scheduler(processes, process_count, output_file, &waiting_queue, &ram_available);
     sjf_scheduler(processes, process_count, output_file, &waiting_queue, &ram_available);
     rr_scheduler(processes, process_count, QUANTUM_HIGH_PRIORITY, output_file, &waiting_queue, &ram_available);
     rr_scheduler(processes, process_count, QUANTUM_MEDIUM_PRIORITY, output_file, &waiting_queue, &ram_available);

    fclose(output_file);  // Close the output file

    // Free allocated memory for processes
    for (int i = 0; i < process_count; i++) {
        free(processes[i]);  // Free memory for each process
    }

    return 0;  // Exit successfully
}
