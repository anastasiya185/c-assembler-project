#include "preprocessor.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_MCROS 100        /*Maximum number of mcros allowed*/
#define MAX_LINE_LENGTH 256   /*Maximum length of a line*/

/*Struct a mcro with its name and body lines*/
typedef struct{
	char name[31];                          /*mcro name*/
	char body[100][MAX_LINE_LENGTH];        /*mcro body: array of lines*/
	int line_count;                         /*number of lines in the acro body*/
}Mcro;

/*Global static storage for defined mcros*/
static Mcro mcros[MAX_MCROS];               /*array of all defined mcros*/
static int mcro_count =0;                     /*number of currently defined mcros*/


/*Check whether given line is the start of a mcro definition*/
static int is_mcro_definition(const char *line){
	return strncmp(line, "mcro", 4) == 0 && isspace((unsigned char)line[4]);
}


/*Check whether a given line marks the end of mcro definition*/
static int is_mcro_end(const char *line){
	return strncmp(line, "mcroend", 7) == 0;
}


/*find a previously defined mcro by name. Return a pointer to the mcro if found*/
static Mcro *find_mcro(const char *name){
int i;
	for(i=0; i<mcro_count; i++){
		if(strcmp(mcros[i].name, name)==0){
			return &mcros[i];
		}
	}
return NULL;
}

/*Skip lines in the file until the next "mcroend" is found*/
static void skip_to_mcroend(FILE *fp){
char line[MAX_LINE_LENGTH];
	while(fgets(line, sizeof(line), fp)){
		if(is_mcro_end(line)) break;
	}
}

/**
*Save a mcro from the input file to the mcros table.
*Read lines until "mcroend" is reached.
*Return 1 on success, 0 on duplicate/overflow, -1 on missing end. 
**/
static int save_mcro(FILE *fp, const char *name, int line_number, char *last_line){
Mcro *m;
char line[MAX_LINE_LENGTH];

	if(find_mcro(name)){
		printf("Error: mcro '%s' already defined (line %d)\n", name, line_number);
		skip_to_mcroend(fp);
		return 0;
	}

	if(mcro_count >= MAX_MCROS){
		printf("Error: too many mcros (limit %d)\n", MAX_MCROS);
		skip_to_mcroend(fp);
		return 0; 
	}
	
	m = &mcros[mcro_count];
	strncpy(m->name, name, 30);
	m->line_count =0;
	
	while(fgets(line, sizeof(line), fp)){
		if(is_mcro_end(line)){
			 mcro_count++;
			 return 1;
		}
		if(m->line_count < 100){
			strcpy(m->body[m->line_count], line);
			m->line_count++;
		}else{
			printf("warning: mcro '%s' body too long (max 100 lines)\n", name);
			break;
		}
	}
printf("Error: missing 'mcroend' for mcro '%s'\n", name);
strcpy(last_line, line);
return -1; 
}


/**
*Main function for running the preprocessor. 
*Replace macro calls with their bodies and saves output to .am file. 
**/
void run_preprocessor(const char *filename){
char input_file[100], output_file[100];
FILE *fp_in, *fp_out;
char line[MAX_LINE_LENGTH];
char first_word[31];
Mcro *m;
int line_number =0; 
int result;

	sprintf(input_file, "%s.as", filename);
	sprintf(output_file, "%s.am", filename);

	fp_in = fopen(input_file, "r");
	if(!fp_in){
		printf("Cannot open %s for reading. \n", input_file);
		return;
	}

	fp_out = fopen(output_file, "w");
	if(!fp_out){
		printf("Cannot open %s for writing. \n", output_file);
		fclose(fp_in);
		return;
	}

	while(fgets(line, sizeof(line), fp_in)){
	char leftover_line[MAX_LINE_LENGTH];
	leftover_line[0] = '\0';
	line_number++;
		if(is_mcro_definition(line)){
			char mcro_name[31];
			if(sscanf(line +4, "%30s", mcro_name) != 1){
				printf("Error: mcro name missing (line %d)\n", line_number);
				skip_to_mcroend(fp_in);
				continue;
			}

			result = save_mcro(fp_in, mcro_name, line_number,  				leftover_line);

			if(result == -1 && leftover_line[0] != '\0'){
			strcpy(line, leftover_line);   /*resume processing leftover line*/
			}else{
				continue;  /*Skip rest of loop if mcro was defined*/
			}
		}
		
		/*Check if line starts with a mcro call*/
		if(sscanf(line, "%30s", first_word) == 1){	
			m = find_mcro(first_word);
			if(m){
				int i;
				for(i=0; i < m->line_count; i++){
					fputs(m->body[i], fp_out);
				}
			}else{
				fputs(line, fp_out); /*Reglar line*/
			}
		}else{
			fputs(line, fp_out);        /*Blank or comment only line*/
		}
	}

fclose(fp_in);	
fclose(fp_out);
printf("Preprocessor finished: %s\n", output_file);
}










