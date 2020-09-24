////////////////////////////////////////////////////////////////////////////////
// Main File:        intdate.c 
// This File:        intdate.c
// Other Files:      sendsig.c, division.c
// Semester:         CS 354 Fall 2019
//
// Author:           Sage Livingstone
// Email:            slivingstone@wisc.edu
// CS Login:         sage
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
//////////////////////////// 80 columns wide ///////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>

/* Signal Handlers */
void handler_SIGALRM();
void handler_SIGUSR1();
void handler_SIGINT();

static const int ALARM_ARM_TIME = 3;
static int SIGUSR1_COUNT = 0;


int main()
{
    // Set alarm for 3 seconds
    alarm(ALARM_ARM_TIME);

    // Set up signal handler for alarm
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler_SIGALRM;
    if(sigaction(SIGALRM, &sa, NULL))
    {
        printf("intdate.c: Error binding SIGALRM handler\n");
        exit(1);
    }

    // Set up signal handler for user sig
    struct sigaction usa;
    memset(&usa, 0, sizeof(usa));
    usa.sa_handler = &handler_SIGUSR1;
    if(sigaction(SIGUSR1, &usa, NULL))
    {
        printf("intdate.c: Error binding SIGUSR1 handler\n");
        exit(1);
    }

    // Set up handler for interrupt
    struct sigaction isa;
    memset(&isa, 0, sizeof(isa));
    isa.sa_handler = &handler_SIGINT;
    if(sigaction(SIGINT, &isa, NULL))
    {
        printf("intdate.c: Error binding SIGINT handler\n");
        exit(1);
    }

    // Infinite loop while waiting for signals
    while(1)
    {
    }
}


/* Handler for alarm signal */
void handler_SIGALRM()
{
    // Get proc pid
    pid_t pid;
    pid = getpid();
    
    // Get curren time str
    time_t now;
    time(&now);
    if(now == (time_t)(-1)){
        printf("Error getting current time!\n");
        return;
    }

    printf("PID: %d | Current Time: %s", pid, ctime(&now));

    //Re-arm the alarm signal
    alarm(ALARM_ARM_TIME);
    return;
}


/* Handler for user signal */
void handler_SIGUSR1()
{
    SIGUSR1_COUNT++;

    printf("SIGUSR1 Caught!\n");
}


/* Handler for interrupt signal */
void handler_SIGINT()
{
    printf("\nSIGINT received.\n");
    printf("SIGUSR1 was received %d times. Exiting now.\n", SIGUSR1_COUNT);
    exit(0);
}

