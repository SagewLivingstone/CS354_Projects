////////////////////////////////////////////////////////////////////////////////
// Main File:        division.c
// This File:        division.c
// Other Files:      intdate.c, sendsig.c
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
#include <string.h>
#include <stdlib.h>
#include <signal.h>

/* Signal Handlers */
void handler_SIGFPE();
void handler_SIGINT();

static const int BUF_SIZE = 100;
static int NUM_OPERATIONS = 0;


int main()
{
    // Set up handler for SIGFPE
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler_SIGFPE;
    if(sigaction(SIGFPE, &sa, NULL)) {
        printf("division.c: Error binding SIGFPE handler\n");
        exit(1);
    }

    // Set up handler for SIGINT
    struct sigaction isa;
    memset(&sa, 0, sizeof(isa));
    isa.sa_handler = &handler_SIGINT;
    if(sigaction(SIGINT, &isa, NULL)) {
        printf("division.c: Error binding SIGINT handler\n");
        exit(1);
    }

    // Create buffer for input
    char buf[BUF_SIZE];
    int a, b, r, q; // Input values
    while(1)
    {
        // Prompt for user input and convert
        printf("Enter first integer: ");
        fgets(buf, BUF_SIZE, stdin);
        a = atoi(buf);
        printf("Enter second integer: ");
        fgets(buf, BUF_SIZE, stdin);
        b = atoi(buf);
        // Calculate division params
        r = a % b;
        q = (a - r) / b;
        printf("%d / %d is %d with a remainder of %d\n", a, b, q, r);

        NUM_OPERATIONS++;
    }
}

/* Handler for SIGFPE */
void handler_SIGFPE()
{
    printf("Error: a division by 0 operation was attempted.\n");
    printf("Total number of operations completed successfully: %d\n", NUM_OPERATIONS);
    printf("The program will be terminated.\n");
    exit(0);
}

/* Handler for SIGINT */
void handler_SIGINT()
{
    printf("\nTotal number of operations completed successfully: %d\n", NUM_OPERATIONS);
    printf("The program will be terminated.\n");
    exit(0);
}


