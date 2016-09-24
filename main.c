#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "pcg_basic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dirent.h"
#include <limits.h>
#include <errno.h>
#include <libgen.h>

#ifndef VERSION
    #define VERSION "2.06"
#endif

// This is set to 1 when -k is supplied - controls whether or not files are kept after deletion
int keepFiles = 0;

// This is set to 1 when -a is supplied - controls whether we chmod files to attempt to gain write access
int forceFiles = 0;

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
    Rand range implementation slightly modified from: http://stackoverflow.com/a/2509699
*/
unsigned int
randr(unsigned int min, unsigned int max)
{
       double scaled = (double)rand()/RAND_MAX;

       return (max - min +1)*scaled + min;
}

void scrambleName(char filename[],int scrambleCount) {
	// The length of the scrambled file names
	// Change this if you want, but DO NOT in some way (strlen etc) connect it
	// to the original filename.
	const int SCRAMBLED_FILE_NAME = 5;

	if (scrambleCount > 0) {
		char *result = strstr(filename,basename(filename));
		int position = result - filename;
		// Where to store the filename
		char path[PATH_MAX - strlen(result)];
		strncpy(path,filename,position);

		// Make up new name
		char newFileName[SCRAMBLED_FILE_NAME];
		strncpy(newFileName,basename(filename),SCRAMBLED_FILE_NAME);

		int i;
		for (i = 0; i < strlen(newFileName); i++) {
			newFileName[i] = randr(97,122);
		}

		// The new name is the same length as the original, so strlen(filename) works fine
		char fullNewName[strlen(filename)];
		sprintf(fullNewName,"%s%s",path,newFileName);
		rename(filename,fullNewName);
		scrambleName(fullNewName,--scrambleCount);
	}
	if (!keepFiles) {
            unlink(filename);
    }
}

/*
    Delete a single file
    @return Zero on successful wipe
*/
int deleteFile(char fileName[], int passes) {
    pcg32_random_t rng;

    if (access(fileName,W_OK) == 0) {
        printf("\nWiping %s...",fileName);

        // chmod if requested (-a)
        if (forceFiles) {
            removeAttribute(fileName);
        }

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

        if (!keepFiles) {
            printf("\nScrambling and deleting %s...",fileName);
            scrambleName(fileName,25);
        }
    }
    else {
        printf("\nERROR: Cannot get write access to %s (try running with -a arg)",fileName);
        return -1;
    }
    return 0;
}

void deleteFolder(char folder[], int passes, int recursive) {
    // Handle top folder
    DIR* dir;
    struct dirent *ent;
    if ( (dir = opendir(folder)) != NULL) {
        while ( (ent = readdir(dir)) != NULL) {
            // Delete only the top level

            char * fullPath = malloc(PATH_MAX);
            sprintf(fullPath,"%s/%s",folder,ent->d_name);

            if (!recursive) {
                if (ent->d_type != DT_DIR) {
                    deleteFile(fullPath,passes);
                }
            }
            else {
                if (!isEqual(ent->d_name,".") && !isEqual(ent->d_name,"..")) {
                    if (ent->d_type == DT_DIR) {
                        deleteFolder(fullPath,passes,recursive);
                    }
                    else {
                        deleteFile(fullPath,passes);
                    }
                }
            }
            free(fullPath);
        }
        closedir(dir);

        if (!keepFiles) {
            rmdir(folder);
            if (errno == ENOTEMPTY) {
                printf("\nWARNING: UNABLE TO DELETE FOLDER AS FILE(S) MIGHT STILL EXIST.");
            }
        }
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
        int c;
        while ((c = getopt(argc, argv, "kasrp:")) != -1) {
            switch(c) {
            case 'p':
                passes = toInt(optarg);
                printf("\nocwiper is set for %d passes.",passes);
                break;
            case 'r':
            case 's':
                isRecursive = 1;
            case 'a':
                forceFiles = 1;
            case 'k':
                keepFiles = 1;
            }
        }

        if (!isFile) {
            deleteFolder(wipeTarget,passes,isRecursive);
        }
        else {
            deleteFile(wipeTarget,passes);
        }
	} else {
		printf("\nocwiper %s by Michael Cowell (2015)\n",VERSION);
		printf("\n\nUsage:\tocwiper [-p passes] [-k] [-q] [-a] [-r] target");
		printf("\n\n\t-p passes\tWipe with X passes. (default is 5)");
		printf("\n\t-k\t\tKeep File after shredding");
		printf("\n\t-a\t\tRemove read-only attribute. (EXPERIMENTAL)");
		printf("\n\t-r\t\tRecurse subdirectories");
	}
	return 0;
}
