/* Chapter 12. commands.c									*/
/*															*/
/* "In Process Server" commands to use with serverSK, etc.	*/
/*															*/
/* There are several commands implemented as DLLs			*/
/* Each command function must be a thread-safe function 	*/
/* and take two parameters. The first is a string:			*/
/* command arg1 arg2 ... argn (i.e.; a normal command line)	*/
/* and the second is the file name for the output			*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void extract_token (int, char *, char *);

_declspec (dllexport)
int wcip (char * command, char * output_file)
/* word count; in process					*/
/* Count the number of characters, works, and lines in		*/
/* the file specified as the second token in "command"		*/
/* NOTE: Simple version; results may differ from wc utility	*/
{
	FILE * fin, *fout;
	int ch, c, nl, nw, nc;
	char input_file[256];
	
	extract_token (1, command, input_file);
			
	fin = fopen (input_file, "r");
	if (fin == NULL) return 1;
	
	ch = nw = nc = nl = 0;
	while ((c = fgetc (fin)) != EOF) {
		if (c == '\0') break;
		if (isspace(c) && isalpha(ch))
			nw++;
		ch = c;
		nc++;
		if (c == '\n')
			nl++;
	}
	fclose (fin);

	/* Write the results */
	fout = fopen (output_file, "w");
	if (fout == NULL) return 2;
	fprintf (fout, " %9d %9d %9d %s\n", nl, nw, nc, input_file);	
	fclose (fout);
	return 0;
}

_declspec (dllexport)
int toupperip (char * command, char * output_file)
/* convert input to upper case; in process			*/
/* Input file is the second token ("toupperip" is the first)	*/
{
	FILE * fin, *fout;
	int c;
	char input_file[256];
	
	extract_token (1, command, input_file);
			
	fin = fopen (input_file, "r");
	if (fin == NULL) return 1;
	fout = fopen (output_file, "w");
	if (fout == NULL) return 2;
	
	while ((c = fgetc (fin)) != EOF) {
		if (c == '\0') break;
		if (isalpha(c)) c = toupper(c);
		fputc  (c, fout);	
	}
	fclose (fin);
	fclose (fout);	
	return 0;
}

static void extract_token (int it, char * command, char * token)
{
	/* Extract token number "it" (first token is number 0)	*/
	/* from "command". Result goes in "token"		*/
	/* tokens are white space delimited			*/

	int i;
	size_t tlen;
	char *pc, *pe, *ws = " \0\t\n"; /* white space */
	
	/* Skip first "it" tokens	*/
	
	pc = command;
	tlen = strlen(command);
	pe = pc + tlen;
	for (i = 0; i < it && pc < pe; i++) {
		pc += strcspn (pc, ws); /* Add length of next token */
			/* pc points to start of white space field */
		pc += strspn (pc, ws); /* Add length of white space field */
	}
	/* pc now points to the start of the token */
	tlen = strcspn (pc, ws);
	memcpy (token, pc, (int)tlen);
	token[tlen] = '\0';

	return;
}