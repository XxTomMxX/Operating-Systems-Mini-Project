#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "cpu.h"

#define MAX_TASKS 100

// Shared globals u
struct node *head = NULL;
int nextTid = 0;
int current_time = 0;

int waiting[MAX_TASKS];
int burst[MAX_TASKS];
int turnaround[MAX_TASKS];
int response[MAX_TASKS];
int completion[MAX_TASKS];
float ratio[MAX_TASKS];
int arrival[MAX_TASKS];
int start[MAX_TASKS];

char *names[MAX_TASKS];

Task *pickNextTask();

void add(char *name, int arrivalTime, int burstTime) 
{
    Task *newTask = malloc(sizeof(Task));
    newTask->name = name;
    newTask->tid = nextTid++;
    newTask->arrivalTime = arrivalTime;
    newTask->burst = burstTime;
    insert(&head, newTask);
}

// Selects the shortest job that has arr 0 
Task *pickNextTask() 
{
    struct node *temp = head;
    Task *best = NULL;

    while (temp != NULL) {
        Task *t = temp->task;

        if (t->arrivalTime <= current_time) {
            if (best == NULL ||
                t->burst < best->burst ||
                (t->burst == best->burst &&
                 strcmp(t->name, best->name) < 0)) {
                best = t;
            }
        }
        temp = temp->next;
    }

    return best;
}

// SJF scheduler hander .1
void schedule() 
{
    traverse(head);

    while (head != NULL) {
        Task *t = pickNextTask();

        if (t == NULL) {
            current_time++;
            continue;
        }

        int id = t->tid;
        arrival[id] = t->arrivalTime;
        burst[id] = t->burst;
        names[id] = t->name;


        start[id] = current_time;
        if (start[id] < arrival[id]) {
            start[id] = arrival[id];
            current_time = arrival[id];
        }

        response[id] = start[id] - arrival[id];

        printf("Running %s at time %d\n", t->name, current_time);

        run(t, t->burst);
        current_time += t->burst;

        completion[id] = current_time;
        turnaround[id] = completion[id] - arrival[id];
        waiting[id] = turnaround[id] - burst[id];
        ratio[id] = (float)(waiting[id] + burst[id]) / burst[id];

        delete(&head, t);
    }

    printf("\nTask\tArr\tBurst\tWait\tTurn\tResp\tRatio");
    for (int i = 0; i < nextTid; i++) 
    {
        printf("\n%s\t%d\t%d\t%d\t%d\t%d\t%.2f",
               names[i],
               //i + 1,
               //taskList[i]->name,
               arrival[i],
               burst[i],
               waiting[i],
               turnaround[i],
               response[i],
               ratio[i]);
    }

// Averages
    float total_wait = 0, total_turn = 0;
    for (int i = 0; i < nextTid; i++) 
    {
        total_wait += waiting[i];
        total_turn += turnaround[i];
    }

    float avg_wait = total_wait / nextTid;
    float avg_turn = total_turn / nextTid;
    float throughput = (float)nextTid / current_time;

    printf("\n\nAverage Waiting Time: %.2f", avg_wait);
    printf("\nAverage Turnaround Time: %.2f", avg_turn);
    printf("\nThroughput: %.6f tasks/unit time\n", throughput);
}
