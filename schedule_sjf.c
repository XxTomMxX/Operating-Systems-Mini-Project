#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int current_time = 0;

// Picks the next available task with the smallest burst
// Nick ... sjf.c file
struct task* pickNextTask() 
{
    struct node* temp = head;
    struct task* best = NULL;

    while (temp != NULL) {
        if (temp->task->arrival_time <= current_time) 
        {
            if (best == NULL ||
                temp->task->burst < best->burst ||
                (temp->task->burst == best->burst &&
                 strcmp(temp->task->name, best->name) < 0)) 
                 {
                best = temp->task;
            }
        }
        temp = temp->next;
    }

    return best;
}

void schedule() 
{
    struct node* temp = head;

    int n = 0;
    while (temp != NULL) {
        n++;
        temp = temp->next;
    }

    char* names[n];
    int arrival[n];
    int burst[n];
    int start[n];
    int completion[n];
    int turnaround[n];
    int waiting[n];
    int response[n];
    double response_ratio[n];
    temp = head;
    int idx = 0;
    while (temp != NULL) {
        names[idx] = temp->task->name;
        arrival[idx] = temp->task->arrival_time;
        burst[idx] = temp->task->burst;
        start[idx] = -1;
        idx++;
        temp = temp->next;
    }

    current_time = 0;

    while (head != NULL) {
        struct task* t = pickNextTask();

        if (t == NULL) {
            current_time++;
            continue;
        }

        int i = -1;
        for (int k = 0; k < n; k++) {
            if (strcmp(names[k], t->name) == 0) {
                i = k;
                break;
            }
        }

        if (start[i] == -1)
            start[i] = current_time;

        run(t, t->burst);
        current_time += t->burst;

        completion[i] = current_time;

        delete(&head, t);
    }

    for (int i = 0; i < n; i++) 
    {
        turnaround[i] = completion[i] - arrival[i];
        waiting[i] = turnaround[i] - burst[i];
        response[i] = start[i] - arrival[i];
        response_ratio[i] = (double)(waiting[i] + burst[i]) / burst[i];
    }

    printf("\n================ SJF Scheduling Results ================\n");
    printf("--------------------------------------------------------\n");
    printf("| Task | Arr | Burst | Start | Comp | Turn | Wait | Resp | RR   |\n");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        printf("| %-4s | %-3d | %-5d | %-5d | %-4d | %-4d | %-4d | %-4d | %.2f |\n",
               names[i], arrival[i], burst[i], start[i], completion[i],
               turnaround[i], waiting[i], response[i], response_ratio[i]);
    }

    printf("--------------------------------------------------------\n");

    // averages
    double avg_turn = 0, avg_wait = 0;
    for (int i = 0; i < n; i++) 
    {
        avg_turn += turnaround[i];
        avg_wait += waiting[i];
    }
    avg_turn /= n;
    avg_wait /= n;

    double throughput = (double)n / current_time;

    printf("Average Turnaround Time: %.4f\n", avg_turn);
    printf("Average Waiting Time: %.4f\n", avg_wait);
    printf("Throughput: %.4f processes/unit time\n", throughput);
    printf("========================================================\n\n");
}
