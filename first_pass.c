#include "first_pass.h"
#include "command_info.h"
#include "tables.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

int IC = 100;  /* Instruction Counter (starts at address 100)*/
int DC = 0;    /* Data Counter*/

/* Global flag to track if an errorduring the first pass */
int error_in_pass1 = 0;

/* Counts the number of operands in a comma separated list*/
static int count_operands(const char *operands_part) {
int count = 0;
const char *p = operands_part;
	while (*p) {
        	if (*p == ',') count++;
        		p++;
    	}
return *operands_part ? count + 1 : 0;
}

/* Checks whether a word is a directive (starts with a .)*/
static int is_directive(const char *word) {
	return word[0] == '.';
}

/*
 * Handles the '.data' directive
 * Parses a list of comma separated integers and adds them to the data section
 */
static void handle_data(const char *params, int line_number, int *DC) {
const char *p = params;
char *endptr;
int number_flag = 1;
int value;

	while (*p) {
        	while (isspace((unsigned char)*p)) p++;

        	if (*p == ',') {
			if (number_flag) {
                printf("Error: invalid number in .data (line %d)\n", line_number);
		error_in_pass1=1;
                	return;
            		}

            		number_flag = 1;
            		p++;
            		continue;
        	}

		if (*p == '\0') break;

		value = strtol(p, &endptr, 10);

		if (endptr == p) {
            printf("Error: invalid number in .data (line %d)\n", line_number);
			error_in_pass1=1;
            		return;
        	}

        	add_data_word(value);
        	(*DC)++;
        	p = endptr;
        	number_flag = 0;
	}

	if (number_flag && p != params) {
        printf("Error: trailing comma or missing number in .data (line %d)\n", line_number);
	error_in_pass1=1;
	}
}

/*
 * Handles the '.string' directive.
 * Each character of the string is stored as a separate word in the data section
 */
static void handle_string(const char *params, int line_number, int *DC) {
const char *end;


	while (isspace((unsigned char)*params)) params++;

	if (*params != '"') {
        printf("Error: .string must start with '\"' (line %d)\n", line_number);
	error_in_pass1=1;
        return;
	}
	
	end = strrchr(params, '"');

	if (!end || end == params) {
        printf("Error: .string missing closing '\"' (line %d)\n", line_number);
	error_in_pass1=1;
        return;
	}

	params++;  /* Skip opening quote*/

	while (params < end) {
		add_data_word((int)*params);
        	(*DC)++;
        	params++;
	}

add_data_word(0);  /* Null terminator*/
(*DC)++;
}

/*
 * Handles the '.mat' directive
 * Parses matrix size and values: .mat [rows][cols], values
 * Fills matrix row-by-row, pads with zeros if there are no value 
 */
static void handle_mat(const char *params, int line_number, int *DC) {
int rows = 0, cols = 0;
const char *p = params;
char *matrix_data;
int init_values = 0;
char *token;
char buffer[MAX_LINE_LENGTH];
int value;

	while (*p && *p != '[') p++;

	if (sscanf(p, "[%d][%d]", &rows, &cols) != 2 || rows <= 0 || cols <= 0) {
        printf("Error: invalid matrix dimensions in .mat (line %d)\n", line_number);
	error_in_pass1=1;
        return;
	}

	/* Find start of values list*/
	matrix_data = strchr(p, ']');
	if (matrix_data) matrix_data = strchr(matrix_data + 1, ']');
	if (!matrix_data) {
        	printf("Error: second ']' not found in \"%s\" (line %d)\n", p, line_number);
		error_in_pass1=1;
        	return;
	}

	matrix_data++;

	/* Skip leading whitespace*/
	while (*matrix_data && isspace((unsigned char)*matrix_data)) matrix_data++;

	/* Copy values into buffer*/
	strncpy(buffer, matrix_data, MAX_LINE_LENGTH - 1);
	buffer[MAX_LINE_LENGTH - 1] = '\0';

	token = strtok(buffer, ",");
	while (token && init_values < rows * cols) {
        	value = atoi(token);
        	add_data_word(value);
        	(*DC)++;
        	init_values++;
        	token = strtok(NULL, ",");
	}

	/* Pad with zeros if not enough values*/
	while (init_values < rows * cols) {
        	add_data_word(0);
        	(*DC)++;
        	init_values++;
	}
}

/*
 * Handles the '.extern' directive
 * Adds a label to the symbol table with type SYMBOL_EXTERN
 */
static void handle_extern(const char *params, int line_number) {
char label[31];

	/* Skip whitespace*/
	while (isspace((unsigned char)*params)) params++;

	if (!isalpha((unsigned char)*params)) {
        	printf("Error: missing label name in .extern (line %d)\n", line_number);
		error_in_pass1=1;
        	return;
	}

	if (sscanf(params, "%30s", label) == 1) {
       		add_symbol(label, 0, SYMBOL_EXTERN);
	}
}

/* Calculates how many words a given instruction will require in memory.*/
int count_instruction_words(const LineInfo *info) {
int words = 1;  
int extra = 0;
int mode1, mode2, mode;

	if (info->operand_count == 2) {
        	mode1 = get_address_mode(info->operands[0]);
        	mode2 = get_address_mode(info->operands[1]);

        	if (mode1 == 3 && mode2 == 3) {
			extra += 1;  
		}else {
            		extra += (mode1 == 2) ? 2 : 1;
            		extra += (mode2 == 2) ? 2 : 1;
       		}

	} else if (info->operand_count == 1) {
        	mode = get_address_mode(info->operands[0]);
        	extra += (mode == 2) ? 2 : 1;
	}

return words + extra;
}


/*
 * Attempts to extract a label and command name from a line of assembly
 * Updates the label and command_name buffers
 */
static int handle_label_command(char *line, char *label, char *command_name, char **command_ptr, int line_number) {
char directive_name[31];
char full_directive[31];
char *colon;
char *ptr = line;

	label[0] = '\0';
	command_name[0] = '\0';

	while (isspace((unsigned char)*ptr)) ptr++;
	if (*ptr == '\0' || *ptr == ';' || *ptr == '\n') return 0;

	colon = strchr(ptr, ':');

	if (colon) {
        	sscanf(ptr, "%30[^:]", label);
        	ptr = colon + 1;

        	while (isspace((unsigned char)*ptr)) ptr++;

        	if (label[0] == '\0') {
            		printf("Error: empty label (line %d)\n", line_number);
			error_in_pass1=1;
            		return 0;
        	}

        	if (!isalpha((unsigned char)label[0])) {
            		printf("Error: invalid label name '%s' (line %d)\n", label, 				line_number);
			error_in_pass1=1;
            		return 0;
		}
	}

	*command_ptr = ptr;

	if (sscanf(ptr, "%30[^ \t\n]", full_directive) != 1) return 0;
	sscanf(full_directive, "%30[.a-zA-Z]", directive_name);
	strncpy(command_name, full_directive, 30);
	command_name[30] = '\0';

return 1;
}

/*
 * Handles .data, .string, .mat, .entry, and .extern directives
 * Returns 1 if line was a directive 0 otherwise
 */
static int handle_directive(const char *command_name, const char *directive_name, char *label, char *params, int line_number) {

if (strcmp(directive_name, ".data") == 0) {
	if (label[0]) {
            	if (add_symbol(label, IC + DC, SYMBOL_DATA)) return 1;
        }
        handle_data(params, line_number, &DC);
        return 1;

} else if (strcmp(directive_name, ".string") == 0) {
        if (label[0]) {
            	if (add_symbol(label, IC + DC, SYMBOL_DATA)) return 1;
        }
        handle_string(params, line_number, &DC);
        return 1;

} else if (strcmp(directive_name, ".mat") == 0 || strncmp(command_name, ".mat", 4) == 0) {
	if (label[0]) {
            	if (add_symbol(label, IC + DC, SYMBOL_DATA)) return 1;
        }
        handle_mat(params - strlen(command_name), line_number, &DC);
        return 1;
	
} else if (strcmp(directive_name, ".extern") == 0) {
        handle_extern(params, line_number);
        return 1;

} else if (strcmp(directive_name, ".entry") == 0) {
        char entry_label[31];
        if (sscanf(params, "%30s", entry_label) == 1) {
        	if(add_entry_label(entry_label, line_number)){
			error_in_pass1=1;
		}
        } else {
            	printf("Error: missing label name in .entry (line %d)\n", line_number);
		error_in_pass1=1;
        }
        return 1;

} else {
        printf("Error: unknown directive '%s' (line %d)\n", command_name, line_number);
	error_in_pass1=1;
        return 1;
}

return 0;
}

/* Handles regular instruction line: mov, add, etc.*/
static void handle_instruction(const CommandInfo *cmd, const char *command_name, char *label, char *ptr, int line_number) {
LineInfo info;
char safe_operands[2][31];
int i;

    	info.address = IC;

    	if (label[0]) {
        	if (add_symbol(label, IC, SYMBOL_CODE)) return;
        	strncpy(info.label, label, 30);
        	info.label[30] = '\0';
    	} else {
        	info.label[0] = '\0';
    	}

   	strncpy(info.command, command_name, 9);
    	info.command[9] = '\0';

    	ptr = strstr(ptr, command_name) + strlen(command_name);
    	while (isspace((unsigned char)*ptr)) ptr++;

    	info.operand_count = count_operands(ptr);

    	if (info.operand_count != cmd->operands) {
	printf("Error: wrong number of operands for command '%s' (line %d)\n", command_name, line_number);
	error_in_pass1=1;
    }

    	split_operands(ptr, safe_operands);
    	for (i = 0; i < info.operand_count; i++) {
        	strncpy(info.operands[i], safe_operands[i], 30);
        	info.operands[i][30] = '\0';
        	trim_space(info.operands[i]);
    	}

add_line_info(info);
IC += count_instruction_words(&info);
}



/*
 * Executes the first pass on a given file
 * - Handles symbol definitions
 * - Processes .data/.string/.mat/.entry/.extern directives
 * - Stores instruction in memory for the second pass
 */
void run_first_pass(const char *filename) {
char input_file[100];
FILE *fp;
char *ptr;
char line[MAX_LINE_LENGTH];
int line_number = 0;
char label[31], command_name[31];
const CommandInfo *cmd;

    	printf("Running first pass on %s\n", filename);

   	sprintf(input_file, "%s.am", filename);
    	fp = fopen(input_file, "r");

    	if (!fp) {
        	printf("Cannot open %s for reading.\n", input_file);
		error_in_pass1=1;
        	return;
    	}

    	while (fgets(line, sizeof(line), fp)) {
        	line_number++;
if (!handle_label_command(line, label, command_name, &ptr, line_number)) continue;

        	/* Try directive*/
        	if (is_directive(command_name)) {
            		ptr = ptr + strlen(command_name);
            		while (isspace((unsigned char)*ptr)) ptr++;
if (handle_directive(command_name, command_name, label, ptr, line_number)) continue;
        	}

        	/* Otherwise try command*/
        	cmd = get_command_info(command_name);
        	if (cmd) {
            		handle_instruction(cmd, command_name, label, ptr, line_number);
        	} else {
            	printf("Error: unknown command '%s' (line %d)\n", command_name, line_number);
		error_in_pass1=1;
        	}
    	}

fclose(fp);
print_symbol_table();
print_entry_labels();
}



















