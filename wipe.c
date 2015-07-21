#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mtwist.h"
#include <unistd.h>
#include <sys/stat.h>

#ifndef VERSION
#define VERSION "1.04"
#endif

int isEqual(char first[], char second[]) {
	return (strcmp(first,second) == 0);
}

// Function for converting string to int.
long int toInt(char *input) {
	char *ptr;
	return strtol(input,&ptr,10);
}

// Main wiping function - 
void wipeFile(char file[], int passes, int keepFile)
{
		// Set up file pointer/generator
		int wipeCount;
		if (access(file,W_OK) == 0) {
			FILE *fp = fopen(file,"rb+");
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			mt_seed();
			// Some handy information
			printf("\n\tUsing %i passes",passes);
			printf("\n\tIn total, we will write %.2f megabytes.\n",((double)size)*passes/1024/1024 );
			
			//
			for (wipeCount = 1; wipeCount <= passes; wipeCount++) {
				printf("\tPass %i...",wipeCount);
				fseek(fp, 0, SEEK_SET);
				int offset;
				char rand[1];
				for (offset = 0; offset < size; offset++)
				{
					rand[0] = (100* mt_ldrand());
					fwrite(rand,1,1,fp);
				}
				if (wipeCount != passes) {
					printf("\n\033[F\033[J");
				}
			}
			fclose(fp);
			if (!keepFile) {
				unlink(file);
			}
		}
		else {
			printf("\nERROR: Could not access file (re-run with -a flag)");
		}
}

void main(int argc,char *argv[]) {
	if (argc >= 2) {
		// Get the filename
		char *fileName = argv[argc-1];
		// How many times the file will be overwritten
		int passes = -1;
		int count;
		int keepFile = 0;
		
		// Loop through arguments and process
		for (count = 0; count < argc; count++) {
			
			// Keep file with -k
			if (isEqual(argv[count],"-k")) {
				keepFile = 1;
			}
			else if (isEqual(argv[count],"-q")) {
				fclose(stdout);
				fclose(stderr);
			}
			// If -p(asses) argument is passed AND it isn't the last argument AND the next argument isn't the filename
			else if (isEqual(argv[count],"-p")) {
				passes = toInt(argv[count+1]);
			}
			else if (isEqual(argv[count],"-a")) {
				int ret = chmod(fileName,strtol("0777",0,8));
			}
		}
		
		printf("\ncwipe %s by Michael Cowell (2015)",VERSION);
		if (passes == -1) {
			printf("\nWARNING: You didn't specify a pass count - Set to 5.");
			passes = 5;
		}
		printf("\nWipe will perform %d passes.",passes);
		
		wipeFile(fileName,passes,keepFile);
	} else {
		printf("\n\nUsage:\twipe [-p passes] [-k] [-q] [-a] fileName");
		printf("\n\n\t-p passes\tWipe with X passes. (default is 5)");
		printf("\n\t-k\t\tKeep File after shredding");
		printf("\n\t-a\t\tRemove read-only attribute.");
		printf("\n\t-q\t\tQuit (suppress all output and errors)");
	}

}