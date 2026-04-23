/**
 * Implementation of various scheduling algorithms.
 *
 * FCFS scheduling updated to use
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#define MAX_TASKS 100

// reference to the head of the list
struct node *head = NULL;

// sequence counter of next available thread identifier
int nextTid = 0;
int current_time = 0;
int waiting[MAX_TASKS];
int burst[MAX_TASKS];
int turnaround[MAX_TASKS];
int response[MAX_TASKS];
int completion[MAX_TASKS];
float ratio[MAX_TASKS];
int arrival[MAX_TASKS];
int total_waiting = 0;
int total_turnaround = 0;
int start[MAX_TASKS];

Task *selectNextTask();

// add a new task to the list of tasks
void add(char *name, int arrivalTime, int burst) {
    // first create the new task
    Task *newTask = (Task *) malloc(sizeof(Task));

    newTask->name = name;
    newTask->tid = nextTid++;
    newTask->arrivalTime = arrivalTime;
    newTask->burst = burst;

    // insert the new task into the list of tasks
    insert(&head, newTask);
}

/**
 * Run the FCFS scheduler
 */
void schedule()
{

    Task *current;

    // sanity checker
    traverse(head);


    while (head != NULL) {

	
        current = selectNextTask(current_time);
        arrival[current->tid] = current->arrivalTime;
        start[current->tid] = current_time;
        if (arrival[current->tid] > current_time)
        {
        	current_time = arrival[current->tid];
        	start[current->tid] = arrival[current->tid];
        }
        
        burst[current->tid] = current->burst;
        response[current->tid]=current_time - current->arrivalTime;
        if (response[current->tid] < 0)
        {
        	response[current->tid] = current->arrivalTime;
        }
        	
        printf("Running %s at time %d\n", current->name, current_time);
        run(current, current->burst);
        current_time +=current->burst;
        completion[current->tid] = current_time;

        	
        turnaround[current->tid] = completion[current->tid] - current->arrivalTime;
        waiting[current->tid] = turnaround[current->tid] - current->burst;
        ratio[current->tid] = (float)(waiting[current->tid] + burst[current->tid])/burst[current->tid];
        
        
        total_waiting += waiting[current->tid];
        total_turnaround += turnaround[current->tid];
        
        delete(&head, current);
        
    }
    
//calculating average tt
    float avgTurnaround = (float)total_turnaround / nextTid;
//avarage wt
    float avgWait = (float)total_waiting / nextTid;
//avg throughput
    float avgThroughput = (float)nextTid / current_time;
    
    printf("\nTask\tArr\tBurst\tWait\tTurn\tResp\tRatio");
    
    for(int i=0;i<nextTid;i++)
    {
    printf("\nT%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%d\t%d", i+1,arrival[i],burst[i], waiting[i], turnaround[i], response[i],ratio[i], completion[i], start[i]); 

    }
    printf("\nAverage Turnaround Time: %f\nAverage Wait Time: %f\nAverage Throughput: %f\n", avgTurnaround, avgWait, avgThroughput);       
}

/**
 * Returns the next task selected to run.
 */
Task *selectNextTask(int currentTime)
{
    struct node *temp = head;
    struct node *selected = head;
 
    while(temp != NULL){
        if(temp->task->arrivalTime < selected->task->arrivalTime){
            selected = temp;
        }
        
        else if(temp->task->arrivalTime == selected->task->arrivalTime)
        {
            if(strcmp(temp->task->name, selected->task->name)<0){
                selected = temp;
            }
        }
        temp = temp->next;
    }
    return selected->task;
}

