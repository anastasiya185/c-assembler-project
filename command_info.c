#include <string.h>
#include "command_info.h"

/*
 * Static array containing all supported assembly commands.
 * Each command specifies:
 * - name
 * - opcode 
 * - number of operands (0, 1 or 2)
 * - allowed source addressing modes (immediate, direct, matrix, register)
 * - allowed destination addressing modes (immediate, direct, matrix, register)
 *
 * For addressing modes index meanings:
 *   0 = immediate(#number)
 *   1 = direct(LABEL)
 *   2 = matrix
 *   3 = register(r0–r7)
 */
static const CommandInfo commands[] = {
	{"mov",  0, 2, {1,1,1,1}, {0,1,0,1}}, 
	{"cmp",  1, 2, {1,1,0,1}, {0,1,0,1}}, 
	{"add",  2, 2, {1,1,0,1}, {0,1,0,1}},
	{"sub",  3, 2, {1,1,0,1}, {0,1,0,1}},
	{"not",  4, 1, {0,0,0,0}, {0,1,0,1}}, 
	{"clr",  5, 1, {0,0,0,0}, {0,1,0,1}}, 
	{"lea",  6, 2, {0,1,0,0}, {0,1,0,1}}, 
	{"inc",  7, 1, {0,0,0,0}, {0,1,0,1}},
	{"dec",  8, 1, {0,0,0,0}, {0,1,0,1}},
	{"jmp",  9, 1, {0,0,0,0}, {0,1,0,0}}, 
	{"bne", 10, 1, {0,0,0,0}, {0,1,0,0}}, 
	{"red", 11, 1, {0,0,0,0}, {0,1,0,1}}, 
	{"prn", 12, 1, {0,0,0,0}, {1,1,0,1}},
	{"jsr", 13, 1, {0,0,0,0}, {0,1,0,0}}, 
	{"rts", 14, 0, {0,0,0,0}, {0,0,0,0}}, 
	{"stop",15, 0, {0,0,0,0}, {0,0,0,0}}, 
	{NULL, -1, -1, {0,0,0,0}, {0,0,0,0}}  
};

/*
 * Finds the CommandInfo for a given command name
 * Return:
 *   Pointer to the CommandInfo struct, NULL if not found
 */
const CommandInfo *get_command_info(const char *name) {
int i;
	for (i = 0; commands[i].name != NULL; i++) {
        	if (strcmp(name, commands[i].name) == 0) {
			return &commands[i];
		}
	}
return NULL;
}

















