#include <stdio.h>
#include "tables.h"
#include <string.h>

#define MAX_SYMBOLS 100     /* Max number of symbols allowed in the symbol table*/
#define MAX_ENTRIES 100         /* Max number of .entry labels*/
#define MAX_LINES 500           /* Max number of LineInfo entries*/
#define MAX_EXT_USAGES 100      /* Max number of .extern symbol*/
#define MAX_DATA 1000         /* Max number of data entries (.data, .string, .mat)*/

/* --- Global static storage ---*/

/* Stores parsed instructions and label info */
static LineInfo line_infos[MAX_LINES];
static int line_info_count = 0;

/* Stores entry label names */
static char entry_labels[MAX_ENTRIES][31];
static int entry_count = 0;

/* Symbol table (for labels) */
static Symbol symbols[MAX_SYMBOLS];
static int symbol_count = 0;

/* Stores usages of external symbols for .ext file */
static ExtUsage ext_usages[MAX_EXT_USAGES];
static int ext_usage_count = 0;

/* Data section (for .data, .string, .mat) */
static int data[MAX_DATA];
static int data_count = 0;


/* --- Symbol Table Functions ---*/

/**
 * Adds a symbol to the symbol table
 * Returns 1 on error (duplicate or overflow), 0 on success
 */
int add_symbol(const char *name, int address, SymbolType type) {
int i;
	for (i = 0; i < symbol_count; i++) {
		if (strcmp(symbols[i].name, name) == 0) {
			printf("Error: duplicate symbol '%s'\n", name);
			return 1;
		}
	}

	if (symbol_count >= MAX_SYMBOLS) {
        	printf("Error: symbol table overflow\n");
        	return 1;
	}

	strncpy(symbols[symbol_count].name, name, 30);
	symbols[symbol_count].name[30] = '\0';
	symbols[symbol_count].address = address;
	symbols[symbol_count].type = type;
	symbol_count++;
	return 0;
}

/**
 * Finds a symbol in the symbol table by name
 * returns a pointer to the symbol or NULL if not found.
 */
const Symbol *find_symbol(const char *name) {
int i;
	for (i = 0; i < symbol_count; i++) {
        	if (strcmp(symbols[i].name, name) == 0) {
			return &symbols[i];
		}
	}
return NULL;
}

/* clears the symbol table.*/
void reset_symbol_table() {
	symbol_count = 0;
}

/* print current contents of the symbol table and all line_infos*/
void print_symbol_table() {
int i, j, k;

printf("Symbol Table:\n");
	for (i = 0; i < symbol_count; i++) {
        	printf("%s -> %d (%s)\n",
			symbols[i].name,
			symbols[i].address,
			symbols[i].type == SYMBOL_CODE ? "code" :
			symbols[i].type == SYMBOL_DATA ? "data" : "extern");
	}

printf("\n--- Stored line info ---\n");
	for (j = 0; j < get_line_info_count(); j++) {
		const LineInfo *li = get_line_info(j);
        	printf("%d: ", li->address);
        	if (li->label[0] != '\0') {
			printf("%s ", li->label);
		}
		printf("%s", li->command);
        	for (k = 0; k < li->operand_count; k++) {
			printf(" %s", li->operands[k]);
		}
		printf("\n");
	}
}


/* --- Entry Labels (.entry) ---*/

/* Adds a new .entry label if valid and not duplicate.*/
int add_entry_label(const char *name, int line_number) {
int i;
const Symbol *sym = find_symbol(name);
	if (sym && sym->type == SYMBOL_EXTERN) {
        printf("Error: label '%s' cannot be both .entry and .extern (line %d)\n",
		name, line_number);
		return 1;
	}

	if (entry_count >= MAX_ENTRIES) {
        	printf("Error: too many .entry labels\n");
        	return 1;
	}

	for (i = 0; i < entry_count; i++) {
        	if (strcmp(entry_labels[i], name) == 0) {
       printf("Warning: duplicate .entry for '%s' (line %d)\n", name, line_number);
            		return 1;
		}
	}

	strncpy(entry_labels[entry_count], name, 30);
	entry_labels[entry_count][30] = '\0';
	entry_count++;
return 0;
}

/**
 * Check if a given label is an entry label
 * return 1 if true 0 otherwise
 */
int is_entry_label(const char *name) {
int i;
	for (i = 0; i < entry_count; i++) {
		if (strcmp(entry_labels[i], name) == 0)
			return 1;
	}
	return 0;
}

/* Prints all collected .entry labels*/
void print_entry_labels() {
int i;
printf("Entry labels:\n");
	for (i = 0; i < entry_count; i++) {
		printf(" %s\n", entry_labels[i]);
	}
}

/* Return the number of .entry labels stored*/
int get_entry_count() {
	return entry_count;
}

/* Retrieve .entry label name by index*/
const char *get_entry_label(int index) {
	if (index >= 0 && index < entry_count) {
		return entry_labels[index];
	}
return NULL;
}


/* --- Line Info Storage ---*/

/* Stores information about a parsed code line (instruction/label)*/
void add_line_info(LineInfo info) {
	if (line_info_count < MAX_LINES) {
		line_infos[line_info_count++] = info;
	} else {
		printf("Error: too many lines for line info buffer\n");
	}
}

/* return the number of stored line info*/
int get_line_info_count() {
	return line_info_count;
}

/* Retrieve a pointer to a stored LineInfo at given index*/
const LineInfo *get_line_info(int index) {
	if (index >= 0 && index < line_info_count)
		return &line_infos[index];
return NULL;
}

/* Clear the line info buffer*/
void reset_line_info() {
	line_info_count = 0;
}


/* --- External Usage (.extern) ---*/

/* Add a usage of an external label for later .ext file*/
void add_ext(const char *name, int address) {
	if (ext_usage_count >= MAX_EXT_USAGES) {
        	printf("Error: too many external usages\n");
        	return;
	}

	strncpy(ext_usages[ext_usage_count].name, name, 30);
	ext_usages[ext_usage_count].name[30] = '\0';
	ext_usages[ext_usage_count].address = address;
	ext_usage_count++;
}

/* Return number of external usages recorded*/
int get_ext_count() {
	return ext_usage_count;
}

/* Retrieve an external usage record by index*/
const ExtUsage *get_ext(int index) {
	if (index >= 0 && index < ext_usage_count)
		return &ext_usages[index];
return NULL;
}


/* --- Data Section (.data / .string / .mat) ---*/

/* Add a word to the data section*/
void add_data_word(int value) {
	if (data_count >= MAX_DATA) {
		printf("Error: too much data\n");
        	return;
	}
data[data_count++] = value;
}

/* Return number of data words stored*/
int get_data_count() {
	return data_count;
}

/* Retrieve a data word by index*/
int get_data_word(int index) {
	if (index >= 0 && index < data_count)
        	return data[index];
return 0;
}




















