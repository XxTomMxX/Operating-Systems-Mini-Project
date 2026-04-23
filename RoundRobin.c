#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "cpu.h"

#define MAX_TASKS 100   // using fixed-size arrays to track RR data

struct node* head = NULL;
int nextTid = 0;

int current_time = 0;
int waiting[MAX_TASKS];
int burst[MAX_TASKS];
int remaining[MAX_TASKS];
int turnaround[MAX_TASKS];
int response[MAX_TASKS];
int completion[MAX_TASKS];
float ratio[MAX_TASKS];
int started[MAX_TASKS];
int queue[MAX_TASKS];
Task* taskList[MAX_TASKS];
int total_waiting = 0;
int total_turnaround = 0;
int busy_time = 0;      // tracking total CPU busy time
int idle_time = 0;      // tracking total CPU idle time

void add(char* name, int arrivalTime, int burstTime) {
    Task* newTask = (Task*)malloc(sizeof(Task));

    newTask->name = name;
    newTask->tid = nextTid++;
    newTask->arrivalTime = arrivalTime;
    newTask->burst = burstTime;
    insert(&head, newTask);   // adding each task into the linked list first
}

void sortTasks(Task* tasks[], int count)
{
    int i;
    int j;
    Task* temp;

    // using nested for loops to sort tasks by arrival time, then by name if tied
    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (tasks[j]->arrivalTime > tasks[j + 1]->arrivalTime) {
                temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
            else if (tasks[j]->arrivalTime == tasks[j + 1]->arrivalTime) {
                if (strcmp(tasks[j]->name, tasks[j + 1]->name) > 0) {
                    temp = tasks[j];
                    tasks[j] = tasks[j + 1];
                    tasks[j + 1] = temp;
                }
            }
        }
    }
}

void schedule()
{
    Task* current;
    struct node* temp;
    int quantum;
    int finished = 0;
    int i;
    int count = 0;
    int front = 0;
    int rear = 0;
    int next_arrival = 0;
    float avg_waiting;
    float avg_turnaround;
    float throughput;
    float cpu_utilization;
    float active_throughput;
    float passive_throughput;

    printf("Enter quantum time: ");   // taking quantum from the user for Round Robin
    scanf("%d", &quantum);

    if (quantum <= 0) {   // using an if statement to reject bad quantum input
        printf("Quantum time must be greater than 0.\n");
        return;
    }

    traverse(head);

    // resetting totals in case the function is ever run again
    total_waiting = 0;
    total_turnaround = 0;
    busy_time = 0;
    idle_time = 0;

    // using a while loop to move tasks from the linked list into an array
    temp = head;
    while (temp != NULL) {
        taskList[count] = temp->task;
        count++;
        temp = temp->next;
    }

    sortTasks(taskList, count);   // sorting first so RR starts in the correct order

    // using a for loop to initialize all arrays before scheduling starts
    for (i = 0; i < count; i++) {
        burst[taskList[i]->tid] = taskList[i]->burst;
        remaining[taskList[i]->tid] = taskList[i]->burst;
        waiting[taskList[i]->tid] = 0;
        turnaround[taskList[i]->tid] = 0;
        response[taskList[i]->tid] = 0;
        completion[taskList[i]->tid] = 0;
        ratio[taskList[i]->tid] = 0.0;
        started[taskList[i]->tid] = 0;
    }

    current_time = taskList[0]->arrivalTime;

    // using a while loop to load the first ready tasks into the queue
    while (next_arrival < count && taskList[next_arrival]->arrivalTime <= current_time) {
        queue[rear] = taskList[next_arrival]->tid;
        rear++;
        next_arrival++;
    }

    // main RR loop: keep cycling until every task is finished
    while (finished < count) {
        if (front == rear) {   // using an if statement to jump forward if the CPU is idle
            idle_time += taskList[next_arrival]->arrivalTime - current_time;
            current_time = taskList[next_arrival]->arrivalTime;

            while (next_arrival < count && taskList[next_arrival]->arrivalTime <= current_time) {
                queue[rear] = taskList[next_arrival]->tid;
                rear++;
                next_arrival++;
            }
        }

        current = taskList[queue[front]];   // taking the next task from the RR queue
        front++;

        if (started[current->tid] == 0) {   // using an if statement so response time is only set once
            response[current->tid] = current_time - current->arrivalTime;
            started[current->tid] = 1;
        }

        printf("Running %s at time %d\n", current->name, current_time);

        // using if/else so a task either gets one quantum or finishes completely
        if (remaining[current->tid] > quantum) {
            run(current, quantum);
            current_time += quantum;
            busy_time += quantum;
            remaining[current->tid] = remaining[current->tid] - quantum;
        }
        else {
            run(current, remaining[current->tid]);
            current_time += remaining[current->tid];
            busy_time += remaining[current->tid];
            remaining[current->tid] = 0;
        }

        // using another while loop to add tasks that arrived during the last run
        while (next_arrival < count && taskList[next_arrival]->arrivalTime <= current_time) {
            queue[rear] = taskList[next_arrival]->tid;
            rear++;
            next_arrival++;
        }

        // using if/else to either requeue the task or calculate its final metrics
        if (remaining[current->tid] > 0) {
            queue[rear] = current->tid;
            rear++;
        }
        else {
            completion[current->tid] = current_time;
            turnaround[current->tid] = completion[current->tid] - current->arrivalTime;
            waiting[current->tid] = turnaround[current->tid] - burst[current->tid];
            ratio[current->tid] = (float)(waiting[current->tid] + burst[current->tid]) / burst[current->tid];

            total_waiting += waiting[current->tid];
            total_turnaround += turnaround[current->tid];
            finished++;
        }
    }

    printf("\nTask\tArr\tBurst\tWait\tTurn\tResp\tRatio");   // printing the required process metrics table
    for (i = 0; i < count; i++) {
        current = taskList[i];
        printf("\n%s\t%d\t%d\t%d\t%d\t%d\t%.2f",
            current->name,
            current->arrivalTime,
            burst[current->tid],
            waiting[current->tid],
            turnaround[current->tid],
            response[current->tid],
            ratio[current->tid]);
    }

    avg_waiting = (float)total_waiting / count;
    avg_turnaround = (float)total_turnaround / count;
    throughput = (float)count / current_time;
    cpu_utilization = ((float)busy_time / current_time) * 100;
    active_throughput = (float)count / busy_time;
    passive_throughput = (float)count / current_time;

    // printing the average values plus the extra CPU stats
    printf("\n\nAverage Waiting Time: %.2f", avg_waiting);
    printf("\nAverage Turnaround Time: %.2f", avg_turnaround);
    printf("\nThroughput: %.6f tasks/unit time", throughput);
    printf("\nCPU Busy Time: %d", busy_time);
    printf("\nCPU Idle Time: %d", idle_time);
    printf("\nCPU Utilization: %.2f%%", cpu_utilization);
    printf("\nActive Throughput: %.6f tasks/unit busy time", active_throughput);
    printf("\nPassive Throughput: %.6f tasks/unit total time\n", passive_throughput);
}
