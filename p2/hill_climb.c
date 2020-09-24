///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
//
///////////////////////////////////////////////////////////////////////////////
// Main File:        hill_climb.c
// This File:        hill_climb.c
// Other Files:      N/A
// Semester:         CS 354 Fall 2019
//
// Author:           Sage Livingstone
// Email:            slivingstone@wisc.edu
// CS Login:         sage
//
/////////////////////////// OTHER SOURCES OF HELP /////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>   
#include <stdlib.h>     
#include <string.h>  

/*
 * USE THIS:
 * Structure representing the Matrix
 * numRows: number of rows
 * numCols: number of columns
 * matrix: 2D heap allocated array of integers
 */
typedef struct {    
    int numRows;
    int numCols;
    int **matrix;
} Matrix;

/* Get the dimensions of the matrix that are on
 * the first line of the input file.
 * 
 * infp: file pointer for input file
 * nRows: pointer to number of rows in the matrix   
 * nCols: pointer to number of columns in the matrix
 */
void get_dimensions(FILE *infp, int *nRows, int *nCols) {         
    // Read the dimensions on the first line from input
    fscanf(infp, "%d, %d", nRows, nCols);
}            

/* Continually find and print the largest neighbor
 * that is also larger than the current position
 * until nothing larger is found.       
 * 
 * outfp: file pointer for output file  
 * mstruct: pointer to the Matrix structure
 */
void hill_climb(FILE *outfp, Matrix *mstruct) {  
    int** matrix = mstruct->matrix; // (ease of use pointer)
    int r = 0, c = 0, max = -1, dir_r, dir_c;
    // Write the intial starting number to outfp
    fprintf(outfp, "%d", **matrix);
    
    while(1)
    {
        // dir represent the intended direction of movement (if max found)
        dir_r = 0; dir_c = 0;
        max = *(*(matrix+r)+c);

        // Find the largest neighbor that is also larger than the current position
        // Check left side
        if (c != 0) 
        {
            if (max < *(*(matrix+r)+c-1))
            {
                max = *(*(matrix+r)+c-1);
                dir_r = 0; dir_c = -1;
            }

            if (r != 0)
            {
                if (max < *(*(matrix+r-1)+c-1))
                {
                    max = *(*(matrix+r-1)+c-1);
                    dir_r = -1; dir_c = -1;
                }
            }
            if (r != mstruct->numRows - 1)
            {
                if (max < *(*(matrix+r+1)+c-1))
                {
                    max = *(*(matrix+r+1)+c-1);
                    dir_r = 1; dir_c = -1;
                }
            }
        }
        // Check right side
        if (c != mstruct->numCols - 1)
        {
            if (max < *(*(matrix+r)+c+1))
            {
                max = *(*(matrix+r)+c+1);
                dir_r = 0; dir_c = 1;
            }

            if (r != 0)
            {
                if (max < *(*(matrix+r-1)+c+1))
                {
                    max = *(*(matrix+r-1)+c+1);
                    dir_r = -1; dir_c = 1;
                }
            }
            if (r != mstruct->numRows - 1)
            {
                if (max < *(*(matrix+r+1)+c+1))
                {
                    max = *(*(matrix+r+1)+c+1);
                    dir_r = 1; dir_c = 1;
                }
            }
        }
        //Check top
        if (r != 0)
        {
            if (max < *(*(matrix+r-1)+c))
            {
                max = *(*(matrix+r-1)+c);
                dir_r = -1; dir_c = 0;
            }
        }
        // Check bottom
        if (r != mstruct->numRows - 1)
        {
            if (max < *(*(matrix+r+1)+c))
            {
                max = *(*(matrix+r+1)+c);
                dir_r = 1; dir_c = 0;
            }
        }
        
        // If the max didn't change, we're done
        if (max == *(*(matrix+r)+c))
        {
            fprintf(outfp, "\n");
            break;
        }
        // Move to that position
        r += dir_r;
        c += dir_c; 
        // Write that number to outfp
        fprintf(outfp, " %d", max);
    }
}    

/* This program reads an m by n matrix from the input file
 * and outputs to a file the path determined from the
 * hill climb algorithm.    
 *
 * argc: CLA count
 * argv: CLA values
 */
int main(int argc, char *argv[]) {
    // Check if number of command-line arguments is correct
    if (argc != 3)
    {
        printf("Usage: ./hill_climb <input_filename> <output_filename>\n");
        exit(1);
    }

    // Open the input file and check if it opened successfully
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        printf("Cannot open file for reading\n");
        exit(1);
    }

    // Declare local variables including the Matrix structure
    Matrix hill;

    // Call the function get_dimensions
    get_dimensions(fp, &(hill.numRows), &(hill.numCols));
   
    // Dynamically allocate a 2D array in the Matrix structure
    hill.matrix = malloc(sizeof(int) * hill.numRows);
    if (hill.matrix == NULL)
    {
        printf("Error: failed to allocate memory for hill array\n");
        exit(1);
    }
    for(int i = 0; i < hill.numRows; i++)
    {
        *(hill.matrix + i) = malloc(sizeof(int) * hill.numCols);
        if (*(hill.matrix + i) == NULL)
        {
            printf("Error: failed to allocate memory while creating hill array\n");
            exit(1);
        }
    }
    
    // Read the file line by line base on the number of rows
    for (int i = 0; i < (hill.numRows); i++)
    {
        for (int j = 0; j < hill.numCols; j++)
        {
            fscanf(fp, "%d,", (*(hill.matrix + i) + j));
        }
    }
    
    // Open the output file  
    FILE* ofp = fopen(argv[2], "w");
    if (ofp == NULL)
    {
        printf("Could not open output file.\n");
        exit(1);
    }

    // Call the function hill_climb
    hill_climb(ofp, &hill);
    
    // Free the dynamically allocated memory
    for(int i = 0; i < (hill.numRows); i++)
    {
        free(*(hill.matrix + i));
    }
    free(hill.matrix);

    // Close the input file  
    // Close the output file
    if(fclose(fp))
    {
        printf("There was an error when closing the input file");
        exit(1);
    }
    if(fclose(ofp))
    {
        printf("There was an error when closing the output file");
        exit(1);
    }

    return 0; 
}

