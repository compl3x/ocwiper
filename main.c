#include <stdio.h>
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
}

// Function for converting string to int.
}

/*
	Attempts to chmod-away RO attribute
	Returns: 1 if successful
*/
}

/*
    Rand range implementation slightly modified from: http://stackoverflow.com/a/2509699
*/
unsigned int

}

    }
}

/*
    Delete a single file
    @return Zero on successful wipe
*/
int deleteFile(char fileName[], int passes) {
    pcg32_random_t rng;



        // Move to end of file to get a correct result from ftell
        fseek(fp, 0, SEEK_END);
        int fileSize = ftell(fp);

        // Wipe file
        int wipeCounter;
        for (wipeCounter = 0; wipeCounter < passes; wipeCounter++) {
            int offsetCounter;
            // Reset position
            for (offsetCounter = 0; offsetCounter < fileSize; offsetCounter++) {
            }
        }
        fclose(fp);

        if (!keepFiles) {
        }
        return -1;
    }
    return 0;
}

void deleteFolder(char folder[], int passes, int recursive) {
    // Handle top folder
            // Delete only the top level

            char * fullPath = malloc(PATH_MAX);

            if (!recursive) {
                }
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

        // Get the filename as the last file
        struct stat sb;

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
        int c;
        while ((c = getopt(argc, argv, "kasrp:")) != -1) {
            case 'p':
                passes = toInt(optarg);
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
        }
