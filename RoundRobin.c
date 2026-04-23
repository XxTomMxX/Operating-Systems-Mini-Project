#include <stdlib.h>
#include <stdio.h>
#include <string.h>  

#include "task.h"
#include "list.h"
#include "cpu.h"

#define MAX_TASKS 100   //max size for RR arrays

struct node *head = NULL;
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
Task *taskList[MAX_TASKS];         
int total_waiting = 0;             
int total_turnaround = 0;          

void add(char *name, int arrivalTime, int burstTime) {
    Task *newTask = (Task *) malloc(sizeof(Task));

    newTask->name = name;
    newTask->tid = nextTid++;
    newTask->arrivalTime = arrivalTime;
    newTask->burst = burstTime;
    insert(&head, newTask);
}

void sortTasks(Task *tasks[], int count)   //puts tasks in correct arrival order
{
    int i;
    int j;
    Task *temp;

    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (tasks[j]->arrivalTime > tasks[j + 1]->arrivalTime) {
                temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
            else if (tasks[j]->arrivalTime == tasks[j + 1]->arrivalTime) {
                if (strcmp(tasks[j]->name, tasks[j + 1]->name) > 0) {   //breaks ties by name
                    temp = tasks[j];
                    tasks[j] = tasks[j + 1];
                    tasks[j + 1] = temp;
                }
            }
        }
    }
}

//RR scheduler
void schedule()
{
    Task *current;
    struct node *temp;
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

    printf("Enter quantum time: ");   // user entered RR time
    scanf("%d", &quantum);

    if (quantum <= 0) {   //stops incorrect input
        printf("Quantum time must be greater than 0.\n");
        return;
    }

    // sanity checker
    traverse(head);

    temp = head;
    while (temp != NULL) {
        taskList[count] = temp->task;   //moves linked-list tasks into array
        count++;
        temp = temp->next;
    }

    sortTasks(taskList, count);   //keeps RR starting order correct

    for (i = 0; i < count; i++) {
        burst[taskList[i]->tid] = taskList[i]->burst;          
        remaining[taskList[i]->tid] = taskList[i]->burst;       
        waiting[taskList[i]->tid] = 0;                          
        turnaround[taskList[i]->tid] = 0;                       
        response[taskList[i]->tid] = 0;                         
        completion[taskList[i]->tid] = 0;                       
        ratio[taskList[i]->tid] = 0.0;                         
        started[taskList[i]->tid] = 0;                          

    current_time = taskList[0]->arrivalTime;   //starts clock at arrival

    while (next_arrival < count && taskList[next_arrival]->arrivalTime <= current_time) {
        queue[rear] = taskList[next_arrival]->tid;   //load first ready tasks into queue
        rear++;
        next_arrival++;
    }

    while (finished < count) {   //keep cycling until all tasks are done
        if (front == rear) {
            current_time = taskList[next_arrival]->arrivalTime;   //jump forward if CPU is idle

            while (next_arrival < count && taskList[next_arrival]->arrivalTime <= current_time) {
                queue[rear] = taskList[next_arrival]->tid;   //add newly arrived tasks
                rear++;
                next_arrival++;
            }
        }

        current = taskList[queue[front]];   //take next task from RR queue
        front++;

        if (started[current->tid] == 0) {
            response[current->tid] = current_time - current->arrivalTime;   //first response only
            started[current->tid] = 1;
        }

        printf("Running %s at time %d\n", current->name, current_time);   //shows RR order

        if (remaining[current->tid] > quantum) {
            run(current, quantum);   //task only gets one quantum
            current_time += quantum;
            remaining[current->tid] = remaining[current->tid] - quantum;
        }
        else {
            run(current, remaining[current->tid]);   //finish task if it needs less than one quantum
            current_time += remaining[current->tid];
            remaining[current->tid] = 0;
        }

        while (next_arrival < count && taskList[next_arrival]->arrivalTime <= current_time) {
            queue[rear] = taskList[next_arrival]->tid;   //add tasks that arrived during the run
            rear++;
            next_arrival++;
        }

        if (remaining[current->tid] > 0) {
            queue[rear] = current->tid;   //unfinished task goes back in queue
            rear++;
        }
        else {
            completion[current->tid] = current_time;  
            turnaround[current->tid] = completion[current->tid] - current->arrivalTime;   //turnaround 
            waiting[current->tid] = turnaround[current->tid] - burst[current->tid];        //waiting 
            ratio[current->tid] = (float)(waiting[current->tid] + burst[current->tid]) / burst[current->tid];   //response ratio 

            total_waiting += waiting[current->tid];         //average waiting total
            total_turnaround += turnaround[current->tid];   //average turnaround total
            finished++;
        }
    }

    printf("\nTask\tArr\tBurst\tWait\tTurn\tResp\tRatio");   //process metrics table
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

    avg_waiting = (float) total_waiting / count;      //average waiting time
    avg_turnaround = (float) total_turnaround / count;   //average turnaround time
    throughput = (float) count / current_time;        //throughput formula

    printf("\n\nAverage Waiting Time: %.2f", avg_waiting);
    printf("\nAverage Turnaround Time: %.2f", avg_turnaround);
    printf("\nThroughput: %.6f tasks/unit time\n", throughput);
}
