#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mtwist.h"
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#ifndef VERSION
#define VERSION "1.09" 
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
			printf("\nERROR: Could not access file %s (re-run with -a flag)",file);
		}
}

void handleFolder(char* path, int recurse, int passes, int maintain,int removeRO){
	DIR* dir;
	struct dirent *ent;
	if((dir=opendir(path)) != NULL){
	while (( ent = readdir(dir)) != NULL){
	  if(ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0  && strcmp(ent->d_name, "..") != 0){
		char fullPath[5] = "";
		strcat(fullPath,path);
		strcat(fullPath,"/");
		strcat(fullPath,ent->d_name);
		if (recurse) {
			handleFolder(fullPath,recurse,passes,maintain,removeRO);
		}
	  }
	  else if (ent->d_type == DT_REG) {
		  char fullPath[5] = "";
		  strcat(fullPath,path);
		  strcat(fullPath,"/");
		  strcat(fullPath,ent->d_name);
		  printf("\nFile: %s",fullPath);
		  if (removeRO) {
			removeAttribute(fullPath);
		  }
		  wipeFile(fullPath,passes,maintain);
	  }
	}
	closedir(dir);
	}
}

/*
	Attempts to chmod-away RO attribute
	Returns: 1 if successful
*/
int removeAttribute(char* fileName) {
	return (chmod(fileName,strtol("0777",0,8)) == 0);
}

void main(int argc,char *argv[]) {
	if (argc >= 2) {
		// Get the filename
		char *fileName = argv[argc-1];
		
		int keepFile, recurse, passes,count,removeRO;
		keepFile = recurse = removeRO = 0;
		passes = -1;
		
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
				removeRO = 1;
			}
			else if (isEqual(argv[count],"-r")) {
				recurse = 1;
			}
		}
		
		printf("\nocwiper %s by Michael Cowell (2015)",VERSION);
		if (passes == -1) {
			printf("\nWARNING: You didn't specify a pass count - Set to 5.");
			passes = 5;
		}
		printf("\nWipe will perform %d passes.",passes);
		
		handleFolder(fileName,recurse,passes,keepFile,removeRO);
	} else {
		printf("\nocwiper %s by Michael Cowell (2015)\n",VERSION);
		printf("\n\nUsage:\tocwiper [-p passes] [-k] [-q] [-a] [-r] target");
		printf("\n\n\t-p passes\tWipe with X passes. (default is 5)");
		printf("\n\t-k\t\tKeep File after shredding");
		printf("\n\t-a\t\tRemove read-only attribute.");
		printf("\n\t-q\t\tQuit (suppress all output and errors)");
		printf("\n\t-r\t\tRecurse subdirectories");
	}

}