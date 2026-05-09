#ifndef TABLES_H
#define TABLES_H

/* Represents the type of a symbol (label).*/
typedef enum {
    SYMBOL_CODE,     /* Label defined in the code section (instruction)*/
    SYMBOL_DATA,     /* Label defined in the data section (.data, .string, .mat)*/
    SYMBOL_EXTERN    /* Label declared as .extern*/
} SymbolType;

/* Represents a symbol (label) in the symbol table.*/
typedef struct {
    char name[31];       /* Symbol name*/
    int address;         /* Address in memory*/
    SymbolType type;     /* Type of symbol (code, data, extern)*/
} Symbol;

/* Represents a line of assembly code after first pass parsing.*/
typedef struct {
    int address;             /* Address of the command*/
    char label[31];          /* Optional label (may be empty)*/
    char command[10];        /* Command name*/
    char operands[2][31];    /* Up to two operands*/
    int operand_count;       /* Number of operands (0, 1 or 2)*/
} LineInfo;

/* Represents a single usage of an external label, for .ext file generation.*/
typedef struct {
    char name[31];       /* External symbol name*/
    int address;         /* Address where it is used*/
} ExtUsage;



/* ----- Symbol Table Functions -----*/

/** Adds a symbol to the symbol table.*/
int add_symbol(const char *name, int address, SymbolType type);

/* Finds a symbol in the symbol table by name.*/
const Symbol *find_symbol(const char *name);

/* Clears the symbol table.*/
void reset_symbol_table();

/* Prints the current symbol table and line info.*/
void print_symbol_table();




/* ----- Entry Labels (.entry) -----*/

/* Adds a new .entry label.*/
int add_entry_label(const char *name, int line_number);

/* Checks whether the given label is marked as .entry.*/
int is_entry_label(const char *name);

/* Prints all collected .entry labels.*/
void print_entry_labels();

/* Returns the number of entry labels collected.*/
int get_entry_count();

/* Retrieves a .entry label name by index.*/
const char* get_entry_label(int index);




/* ----- Line Info Storage -----*/

/* Stores information about a parsed code line (instruction/label).*/
void add_line_info(LineInfo info);

/** Returns the number of stored line infos.*/
int get_line_info_count();

/* Retrieves a pointer to a stored LineInfo at given index.*/
const LineInfo *get_line_info(int index);

/* Clears the line info buffer.*/
void reset_line_info();




/* ----- External Symbol Usage (.extern) -----*/

/* Adds a usage of an external label for later .ext file generation.*/
void add_ext(const char *name, int address);

/* Returns number of external usages recorded.*/
int get_ext_count();

/* Retrieves an external usage record by index.*/
const ExtUsage *get_ext(int index);




/* ----- Data Section (.data, .string, .mat) -----*/

/* Adds a word to the data section.*/
void add_data_word(int value);

/* Returns number of data words stored.*/
int get_data_count(void);

/* Retrieves a data word by index.*/
int get_data_word(int index);

#endif  

