#include "headers.h"

int main()
{
    enum queueInsertionKey insertionFactor = 1;
    // 1 2 50 2 
    // 2 5 25 1
    // 3 6 30 0
    // 4 7 30 3
    // 5 8 30 4
    // -----------
    // 1 2 50 1 
    // 2 2 50 3
    // 3 2 70 1
    // 4 5 100 7
    // 5 7 60 8
    struct process* p1;
    p1=(struct process*)malloc(sizeof(struct process));
    strcpy(p1->state,"running");
    p1->id = 1;
    p1->arrivalTime = 2;
    p1->runTime = 50;
    p1->priority = 2;
    p1->remainingTime = p1->runTime;

    struct process* p2;
    p2=(struct process*)malloc(sizeof(struct process));
    strcpy(p2->state,"ready");
    p2->id = 2;
    p2->arrivalTime = 5;
    p2->runTime = 25;
    p2->priority = 1;
    p2->remainingTime = p2->runTime;

    struct process* p3;
    p3=(struct process*)malloc(sizeof(struct process));
    strcpy(p3->state,"ready");
    p3->id = 3;
    p3->arrivalTime = 6;
    p3->runTime = 30;
    p3->priority = 0;
    p3->remainingTime = p3->runTime;

    struct process* p4;
    p4=(struct process*)malloc(sizeof(struct process));
    strcpy(p4->state,"ready");
    p4->id = 4;
    p4->arrivalTime = 7;
    p4->runTime = 30;
    p4->priority = 3;
    p4->remainingTime = p4->runTime;

    struct process* p5;
    p5=(struct process*)malloc(sizeof(struct process));
    strcpy(p5->state,"ready");
    p5->id = 5;
    p5->arrivalTime = 8;
    p5->runTime = 30;
    p5->priority = 4;
    p5->remainingTime = p5->runTime;

    struct customPriorityQueue* CPQptr = newCustomPriorityQueue();
    enqueue(&CPQptr, p1, insertionFactor);
    enqueue(&CPQptr, p2, insertionFactor);
    enqueue(&CPQptr, p3, insertionFactor);
    // enqueue(&CPQptr, p4, insertionFactor);
    // enqueue(&CPQptr, p5, insertionFactor);
    printQueue(&CPQptr);
    printProcess(front(&CPQptr));
    return 0;
}