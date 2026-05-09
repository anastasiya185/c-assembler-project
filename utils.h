#ifndef UTILS_H
#define UTILS_H

/*
 * Removes leading and trailing whitespace from the given string.
 * Modifies the string in-place.
 */
void trim_space(char *str);

/*
 * Checks whether the given line is empty or a comment (starts with ';').
 * Returns 1 if true, 0 otherwise.
 */
int is_empty_or_comment(const char *line);

/*
 * Determines the addressing mode of an operand string.
 * Returns:
 *   0 - immediate addressing (e.g. "#5")
 *   1 - direct addressing (e.g. "LABEL")
 *   2 - matrix addressing (e.g. "M1[r2][r3]")
 *   3 - register addressing (e.g. "r5")
 *  -1 - invalid operand
 */
int get_address_mode(const char *operand);

/*
 * Splits a comma-separated operand string into up to 2 operands,
 * preserving matrix notation.
 * Parameters:
 *   input    - operand string
 *   operands - output array of 2 strings, each max 30 chars
 * Returns the number of operands parsed (1 or 2).
 */
int split_operands(const char *input, char operands[2][31]);
	
#endif
