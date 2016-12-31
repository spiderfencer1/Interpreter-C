#include <ctype.h>
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	if (argc < 2) { fprintf(stderr, "Error: need a filename."); return 1; }
	FILE* fp = fopen(argv[1],"r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* buffer = malloc(length + 1);
		fread(buffer, 1, length, fp);
		buffer[length] = 0;
		fclose(fp);

		p_init();
		parse(buffer);
		p_close();

		free(buffer);
	} else { fprintf(stderr, "Error: cannot open file: %s.\n", argv[1]); return 1; }
	return 0;
}
