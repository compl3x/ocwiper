#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "pcg_basic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dirent.h"

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

/*
    Delete a single file
    @return Zero on successful wipe
*/
int deleteFile(char fileName[], int passes) {
    pcg32_random_t rng;
    printf("\nWiping file: %s",fileName);
    if (access(fileName,W_OK) == 0) {
        FILE *fp = fopen(fileName,"rb+");

        // Move to end of file to get a correct result from ftell
        fseek(fp, 0, SEEK_END);
        int fileSize = ftell(fp);

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
        return -1;
    }
    return 0;
}

int main(int argc,char *argv[]) {
	if (argc >= 1) {
        // Get the filename as the last file
        struct stat sb;
        printf("\nTarget: %s",argv[argc-1]);
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

        if (!isRecursive && !isFile) {
            // TODO: Move into its own function with recurse parameter
            // Handle top folder
            DIR* dir;
            struct dirent *ent;
            if ( (dir = opendir(wipeTarget)) != NULL) {
                while ( (ent = readdir(dir)) != NULL) {
                    if (ent->d_type != DT_DIR) {
                        char fullPath[strlen(wipeTarget) + ent->d_namlen + 2];
                        sprintf(fullPath,"%s/%s",wipeTarget,ent->d_name);
                        deleteFile(fullPath,passes);
                    }
                }
            }
        }
        else if (isRecursive && !isFile) {
            // Handle folder recursively
        }
        else {
            deleteFile(wipeTarget,passes);
        }

        //printf("\n passes = %d, isRecursive = %d, isFile = %d",passes,isRecursive,isFile);



	} else {
		printf("\nocwiper %s by Michael Cowell (2015)\n",VERSION);
		printf("\n\nUsage:\tocwiper [-p passes] [-k] [-q] [-a] [-r] target");
		printf("\n\n\t-p passes\tWipe with X passes. (default is 5)");
		printf("\n\t-k\t\tKeep File after shredding");
		printf("\n\t-a\t\tRemove read-only attribute.");
		printf("\n\t-q\t\tQuit (suppress all output and errors)");
		printf("\n\t-r\t\tRecurse subdirectories");
	}
	getchar();
	return 0;
}
