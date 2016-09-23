#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "pcg_basic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef VERSION
#define VERSION "2.01"
#endif

#define max(x,y) ((x) > (y) ? (x) : (y))

// Returns 1 if first[] and second[] are identical
int isEqual(char first[], char second[]) {
	return (strcmp(first,second) == 0);
}

// Function for converting string to int.
long int toInt(char *input) {
	char *ptr;
	return strtol(input,&ptr,10);
}

/*
	Attempts to chmod-away RO attribute
	Returns: 1 if successful
*/
int removeAttribute(char* fileName) {
	return (chmod(fileName,strtol("0777",0,8)) == 0);
}

/* Delete a single file
*/
void deleteFile(char fileName[], int passes) {
    pcg32_random_t rng;

    if (access(fileName,W_OK) == 0) {
        FILE *fp = fopen(fileName,"rb+");
        fseek(fp, 0, SEEK_END);

        int fileSize = ftell(fp);
        printf("\nFilesize: %d",fileSize);

        // Wipe file
        int wipeCounter;
        for (wipeCounter = 0; wipeCounter < passes; wipeCounter++) {
            int offsetCounter;
            // Reset position
            fseek(fp,0,SEEK_SET);
            for (offsetCounter = 0; offsetCounter < fileSize; offsetCounter++) {
                int rand = pcg32_boundedrand_r(&rng,256);
                fprintf(fp,"%c",rand);
            }
        }
        fclose(fp);
    }
    else {
        printf("\nERROR: Cannot get write access to file (try running with -a arg)");
    }

}

int main(int argc,char *argv[]) {
	if (argc >= 2) {
        // Get the filename as the last file
        struct stat sb;
        int ret = stat(argv[argc-1],&sb);
        // If target doesn't exist
        if (ret < 0) {
            printf("\nERROR: The specified target does not seem to exist. Exiting...");
            return -1;
        }

        // Process arguments
        int passes, isFile, isRecursive;
        passes = 3;
        isFile = S_ISDIR(sb.st_mode) ? 0 : 1;
        isRecursive = 0;

        // Store target
        char * wipeTarget = argv[argc-1];


        int counter;
        for (counter = 0; counter < argc; counter++) {
            if (isEqual(argv[counter],"-p")) {
                // Set next arg as the pass count - minimum of 3
                passes = max(3,toInt(argv[counter+1]));
                printf("\nWiping file a total of %d times.",passes);
            }
            else if (isEqual(argv[counter],"-r")) {
                // Recursively delete
                if (isFile) {
                    printf("\nERROR: Expecting folder, given file. Exiting...");
                    return -2;
                }
                isRecursive = 1;
            }
        }

        if (isRecursive) {
            // Do folder handling
            printf("\nFolder");
        }
        else {
            // Do file handling with wipeTarget
            deleteFile(wipeTarget,passes);
        }

        printf("\nIs file: %d",isFile);
        printf("\nPasses: %d",passes);



	} else {
		printf("\nocwiper %s by Michael Cowell (2015)\n",VERSION);
		printf("\n\nUsage:\tocwiper [-p passes] [-k] [-q] [-a] [-r] target");
		printf("\n\n\t-p passes\tWipe with X passes. (default is 5)");
		printf("\n\t-k\t\tKeep File after shredding");
		printf("\n\t-a\t\tRemove read-only attribute.");
		printf("\n\t-q\t\tQuit (suppress all output and errors)");
		printf("\n\t-r\t\tRecurse subdirectories");
	}
	return 0;
}
