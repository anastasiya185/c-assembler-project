#ifndef COMMAND_INFO_H
#define COMMAND_INFO_H

/*
 * Represents the metadata for a single assembly command.
 */
typedef struct {
	char *name;               /* Command name*/
	int opcode;               /* Opcode value*/
	int operands;             /* Number of expected operands (0, 1 or 2)*/
	int allowed_src[4];       /* Allowed source addressing modes*/
	int allowed_dst[4];       /* Allowed destination addressing modes*/
} CommandInfo;

/*
 * Finds the CommandInfo for a given command name.
 * Return:
 *   Pointer to the CommandInfo struct NULL if not found.
 */
const CommandInfo *get_command_info(const char *name);

#endif  
