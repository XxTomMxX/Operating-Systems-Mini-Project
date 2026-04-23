/**

adjsutments to Makefile:

fcfsdat: driver.o list.o CPU.o scheduler_fcfs_diff_arrival.o
	$(CC) $(CFLAGS) -o fcfsdat driver.o scheduler_fcfs_diff_arrival.o list.o CPU.o

this is the second iteration. it uses Maddox's updated code that accounts for task name, cpu util, 
and other additional calculations

input file intended: schedule_arrive_diff.txt

* Implementation of various scheduling algorithms.
* FCFS scheduling
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
int waiting [MAX_TASKS];
int burst [MAX_TASKS];
int turnaround [MAX_TASKS];
int response [MAX_TASKS];
int completion[MAX_TASKS];
int arrival[MAX_TASKS];
float ratio [MAX_TASKS];
int total_waiting = 0;
int total_turnaround = 0;
int total_burst = 0;
char *names [MAX_TASKS];
int idle_time = 0;

Task *selectNextTask();

// add a new task to the list of tasks
void add(char *name, int arrivalTime, int burst) {
  // first create the new task
  Task *newTask = (Task *) malloc(sizeof(Task));//Allocates memory for task byte size

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
	traverse (head);

	while (head != NULL) {

		current = selectNextTask();

		burst[current->tid] = current->burst;//collect burst times
		arrival[current->tid] = current->arrivalTime;//collect arrival times
		names [current->tid] = strdup(current->name);//collect names
		
		//check if the task arrives after the CPU is ready abnd updates idle time
		if (arrival[current->tid]>current_time){
			idle_time += (arrival[current->tid] - current_time);
			current_time = arrival[current->tid];
		}

		response [current->tid] = current_time - current->arrivalTime;//response=current-arrival

		printf("Running %s at time %d\n", current->name, current_time);

		run(current, current->burst) ;
		current_time += current->burst;//time at present
		completion [current->tid] = current_time;
		turnaround [current->tid] = completion[current->tid] - current->arrivalTime;
		//turnaround=completion-arrival

		waiting[current->tid] = turnaround[current->tid] - current->burst;
		//waiting=turnaround-burst
		ratio[current->tid] = (float) (waiting[current->tid] + burst[current->tid]) / burst[current->tid];
		//ratio=(waiting+burst)/burst
		total_waiting += waiting[current->tid];
		total_turnaround += turnaround[current->tid];
		total_burst += burst[current->tid];
		
		delete(&head, current);

		
	}
	

		printf("\nTask\tArr\tBurst\tWait\tTurn\tResp\tRatio");

		for (int i=0; i<nextTid; i++)

		printf("\n%s\t%d\t%d\t%d\t%d\t%d\t%.2f",
		names[i], arrival[i], burst[i], waiting[i], turnaround[i], response[i],
		ratio[i]);

		float avg_waiting = (float)total_waiting / nextTid;
		float avg_turnaround = (float) total_turnaround / nextTid;
		//int idle_time = current_time - total_burst;//Idle time will now change as processes arrive some time after the CPU is ready
		float cpu_utilization = (float) total_burst / current_time * 100;
		//CPU util will change now that idle time can now change
		float active_throughput = (float) nextTid / total_burst;//With idle time
		float passive_throughput = (float) nextTid / current_time;//Without idle time

		printf("\n\nIdle Time: %d", idle_time);
		printf("\nCPU Utilization: %.2f", cpu_utilization);
		printf("\nActive Throughput: %.2f task/unit time", active_throughput);
		printf("\nPassive Throughput: %.2f task/unit time", passive_throughput);
		printf("\n\nAverage Waiting Time: %.3f", avg_waiting);
		printf("\nAverage Turnaround Time: %.3f", avg_turnaround);
		printf("\nCPU Utilization: %.3f", cpu_utilization);
		printf("\ncompletiontime: %d", current_time);
}
/**
* Returns the next task selected to run.
*/
Task *selectNextTask()
{
	struct node *temp = head;//Holds current process
	struct node *selected = head;//Holds best selected process

	while (temp != NULL) {//Keep looping until end of list
		if (temp->task->arrivalTime <= selected->task->arrivalTime) {
		selected = temp;//Compares arrival times, sets selected to temp
		}
		else if (temp->task->arrivalTime == selected->task->arrivalTime) {//if arrival time equal
			if (strcmp(temp->task->name, selected->task->name) < 0) {
			selected = temp;//Compares alphabetically
			}
		}
		temp = temp->next;//Next node
	}
	return selected->task;//Best node returned

}
