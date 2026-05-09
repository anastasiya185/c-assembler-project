#include "second_pass.h"
#include "tables.h"
#include "command_info.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* Global flag to track if an error during the second pass */
int error_in_pass2 = 0;

/* Counter for how many lines were written to the .ob file */
int total_lines_writen = 0;

/*
 * Returns the numeric register number from a string
 * If the string is not a valid register returns 0
 */
static int get_register_number(const char *reg) {
    	if (reg[0] == 'r' && isdigit(reg[1])) {
        	return reg[1] - '0';
    	}
return 0;
}

/*
 * Converts a signed 10bit word into a 5-character base-4 string using 'abcd' digits.
 */
const char *to_base4_word(int num, char *buf) {
const char *digits = "abcd";
int i;
unsigned int unum = (unsigned int)num;

    	for (i = 4; i >= 0; i--) {
        	buf[i] = digits[unum % 4];
        	unum /= 4;
    	}
buf[5] = '\0';
return buf;
}

/* Converts an address into a 4-character base-4 string using 'abcd'*/
const char *to_base4_address(int num, char *buf) {
const char *digits = "abcd";
int i;

	for (i = 3; i >= 0; i--) {
        	buf[i] = digits[num % 4];
        	num /= 4;
    	}
buf[4] = '\0';
return buf;
}

/* General utility to convert a number to a base-4 string of specified length*/
const char *to_base4(int num, char *buf, int digits) {
const char *digits_map = "abcd";
int i;

    	for (i = digits - 1; i >= 0; i--) {
        	buf[i] = digits_map[num % 4];
        	num /= 4;
    	}
buf[digits] = '\0';
return buf;
}

/* Writes a single line to the .ob file*/
static void write_binary(FILE *fp, int address, int word) {
char addr_buf[5], word_buf[6];
	to_base4_address(address, addr_buf);
    	to_base4_word(word, word_buf);
    	fprintf(fp, "%s %s\n", addr_buf, word_buf);
    	total_lines_writen++;
}

/* Encodes and writes the first instruction word */
static void write_first_word(const LineInfo *info, FILE *ob_file, int src_mode, int dst_mode, int opcode) {
int ARE = 0;

    	int first_word = (opcode << 6) | (src_mode << 4) | (dst_mode << 2) | ARE;
    	write_binary(ob_file, info->address, first_word);
}

/* Handles case where both operands are registers*/
static int handle_registers_pair(const LineInfo *info, FILE *ob_file) {

    	if (info->operand_count == 2 &&
        get_address_mode(info->operands[0]) == 3 &&
        get_address_mode(info->operands[1]) == 3) {

        	int src_reg = get_register_number(info->operands[0]);
        	int dst_reg = get_register_number(info->operands[1]);
        	int word = (src_reg << 6) | (dst_reg << 2);
       		write_binary(ob_file, info->address + 1, word);
        	return 1;
    	}
return 0;
}

/* Encodes matrix operand: writes label word + register pair word */
static int encode_matrix(const char *operand, int current_address, FILE *ob_file) {
char label[31], reg1[4], reg2[4];
const Symbol *sym;
int ARE = 0, reg_word = 0, r1, r2;

	if (sscanf(operand, "%[^[][%[^]]][%[^]]]", label, reg1, reg2) != 3) {
        	printf("Error: malformed matrix operand '%s'\n", operand);
        	error_in_pass2 = 1;
        	return 2;
    	}

    	sym = find_symbol(label);

    	if (sym) {
        	int val = sym->address;
        	ARE = (sym->type == SYMBOL_EXTERN) ? 1 : 2;
        	if (ARE == 1) add_ext(sym->name, current_address);
        	write_binary(ob_file, current_address++, (val << 2) | ARE);
    	} 
	if(!sym) {
        	printf("Error: unknown matrix label '%s'\n", label);
        	error_in_pass2 = 1;
        	return 2;
    	}

    	if (reg1[0] == 'r' && isdigit(reg1[1]) &&
        reg2[0] == 'r' && isdigit(reg2[1])) {
        	r1 = reg1[1] - '0';
        	r2 = reg2[1] - '0';
        	reg_word = (r1 << 6) | (r2 << 2);
       		write_binary(ob_file, current_address++, reg_word);
    	}else {
        	printf("Error: invalid registers in matrix operand\n");
        	error_in_pass2 = 1;
        	return 2;
    	}

return 2;
}

/* Encodes immediate operand*/
static void encode_immediate(const char *operand, int *cur_address, FILE *ob_file) {

int value = strtol(operand + 1, NULL, 10);
int word = ((value & 0x3FFF) << 2);  /* ARE = 0 */
write_binary(ob_file, (*cur_address)++, word);

}

/* Encodes regster operand */
static void encode_register(const char *operand, int *current_address, FILE *ob_file, int operand_index, int operand_count) {
int reg_num = get_register_number(operand);
int word;

    	if (operand_count == 1 || operand_index == 1) {
        	word = (reg_num << 2);
    	}else if (operand_index == 0) {
        	word = (reg_num << 6);
    	}else{
        	word = (reg_num << 2);
    	}

write_binary(ob_file, (*current_address)++, word);
}

/* Encodes direct label operand */
static void encode_label(const char *operand, int *cur_address, FILE *ob_file) {
const Symbol *sym = find_symbol(operand);
int value = 0, ARE = 0;
int word;

    	if (sym) {
        	value = sym->address;
        	ARE = (sym->type == SYMBOL_EXTERN) ? 1 : 2;
        	if (ARE == 1) add_ext(sym->name, *cur_address);
    	}else{
        	printf("Error: unknown symbol '%s'\n", operand);
		error_in_pass2 = 1;
    	}

word = (value << 2) | ARE;
write_binary(ob_file, (*cur_address)++, word);
}

/* Main encoder function for instructions */
static void encode_command(const LineInfo *info, FILE *ob_file) {
int i;
int src_mode = 0, dst_mode = 0;
int cur_address;
char clean_operand[31];
const CommandInfo *cmd = get_command_info(info->command);
    	if (!cmd) return;

    	/* Validate operand count */
    	if (info->operand_count != cmd->operands) {
        	printf("Error at address %d: command '%s' expects %d operand(s), got %d\n",
               info->address, info->command, cmd->operands, info->operand_count);
        	error_in_pass2 = 1;
        	return;
    	}

    	/* Get addressing modes */
src_mode = info->operand_count >= 2 ? get_address_mode(info->operands[0]) : 0;
dst_mode = info->operand_count >= 1 ?
get_address_mode(info->operands[info->operand_count - 1]) : 0;

    	/* Check addressing mode validity */
    	if (cmd->operands == 2 && cmd->allowed_src[src_mode] == 0) {
printf("Error at address %d: invalid source address mode (%d) for command '%s'\n",
                info->address, src_mode, info->command);
        	error_in_pass2 = 1;
        	return;
    	}

    	if (cmd->operands >= 1 && cmd->allowed_dst[dst_mode] == 0) {
printf("Error at address %d: invalid destination address mode (%d) for command '%s'\n",
                info->address, dst_mode, info->command);
        	error_in_pass2 = 1;
        	return;
    	}

    	write_first_word(info, ob_file, src_mode, dst_mode, cmd->opcode);

    	/*both operands are registers */
    	if (handle_registers_pair(info, ob_file)) return;

    	cur_address = info->address + 1;

   	 /* Encode each operand */
    	for (i = 0; i < info->operand_count; i++) {
        	strncpy(clean_operand, info->operands[i], 30);
        	clean_operand[30] = '\0';
        	trim_space(clean_operand);

        	if (get_address_mode(clean_operand) == 2) {
cur_address += encode_matrix(clean_operand, cur_address, ob_file);
        	}else if (clean_operand[0] == '#') {
encode_immediate(clean_operand, &cur_address, ob_file);
        	}else if (get_address_mode(clean_operand) == 3) {
encode_register(clean_operand, &cur_address, ob_file, i, info->operand_count);
        	}else {
encode_label(clean_operand, &cur_address, ob_file);
        	}
   	 }
}


/* Writes the .ent file with all entry labels and ther addresses.*/
void write_ent_file(const char *filename) {
char ent_filename[100];
char addr_buf[5];
FILE *ent_file;
int i;

    	if (get_entry_count() == 0)
       		return;

    	sprintf(ent_filename, "%s.ent", filename);
    	ent_file = fopen(ent_filename, "w");

    	if (!ent_file) {
        	printf("Error: cannot create .ent file %s\n", ent_filename);
        	return;
    	}

    	for (i = 0; i < get_entry_count(); i++) {
        	const char *label = get_entry_label(i);
        	const Symbol *sym = find_symbol(label);

        	if (sym && sym->type != SYMBOL_EXTERN) {
			to_base4_address(sym->address, addr_buf);
            		fprintf(ent_file, "%s %s\n", label, addr_buf);
        	}
    	}

fclose(ent_file);
}

/*Writes the .ext file with all external symbol usages*/
void write_ext_file(const char *filename) {
char ext_filename[100];
char addr_buf[5];
FILE *ext_file;
int i;

	if(get_ext_count() ==0)
		return;	

    	/* Build filename: original + ".ext" */
    	sprintf(ext_filename, "%s.ext", filename);
    	ext_file = fopen(ext_filename, "w");

    	if (!ext_file) {
        	printf("Error: cannot create .ext file %s\n", ext_filename);
        	return;
    	}

    	/* Write each external usage: label name + address in base-4 */
    	for (i = 0; i < get_ext_count(); i++) {
        	const ExtUsage *usage = get_ext(i);
        	to_base4_address(usage->address, addr_buf);
        	fprintf(ext_file, "%s %s\n", usage->name, addr_buf);
    	}

fclose(ext_file);
}

/*
 * Performs the second pass of the assembler on a given file
 * Generates .ob, .ent and .ext output files.
 */
void run_second_pass(const char *filename) {
char ob_filename[100];
FILE *ob_file;
int i;
extern int IC, DC;
extern int error_in_pass2;
char ic_buf[6];
char dc_buf[6];
int data_start;

    	error_in_pass2 = 0;

    	printf("Running second pass on %s\n", filename);

    	/* Create .ob filename and open for writing */
    	sprintf(ob_filename, "%s.ob", filename);
    	ob_file = fopen(ob_filename, "w");

    	if (!ob_file) {
        	printf("Error: cannot create output file %s\n", ob_filename);
        	return;
    	}

    	/* Write IC and DC header to .ob file, both encoded in base-4 */
    	to_base4(IC - 100, ic_buf, 3);  
    	to_base4(DC, dc_buf, 2);        
    	fprintf(ob_file, "%s %s\n", ic_buf, dc_buf);

    	/* Write instruction words */
    	for (i = 0; i < get_line_info_count(); i++) {
        	const LineInfo *info = get_line_info(i);
        	encode_command(info, ob_file);
		
		if(error_in_pass2){
			break;
		}
    	}

    	/* Append data section at end of .ob file */
    	data_start = IC;

    	for (i = 0; i < get_data_count(); i++) {
        	write_binary(ob_file, data_start+ i, get_data_word(i));
    	}

    	/* If errors occurred during second pass, abort and delete .ob */
    	if (error_in_pass2) {
        	printf("Second pass aborted due to errors.\n");
        	fclose(ob_file);
        	remove(ob_filename);
        	return;
    	}

printf(".ob written, total instructions: %d, data: %d\n", IC - 100, get_data_count());
fclose(ob_file);

/* Write additional output files */
write_ent_file(filename);
write_ext_file(filename);
}















