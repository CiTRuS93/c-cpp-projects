#include <stdio.h>
#include <string.h>
int main(int argc, char **argv)
{
	int debug = 0;
	int enc = 0;
	FILE *input = stdin;
	FILE *output = stdout;
	char *encrypt;
	int encryptSize = 0;
	char a;
	int counter = 0;
	//configuration
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-D") == 0)
		{
			debug = 1;
			for (int j = 1; j < argc; j++)
				fprintf(stderr, "%s\n", argv[j]);
		}
		if (strncmp(argv[i], "-e", 2) == 0)
		{
			encrypt = &argv[i][2];
			for (int j = 0; encrypt[j] != '\0'; j++)
			{
				encryptSize = j;
			}
			enc = -1;
		}
		if (strncmp(argv[i], "+e", 2) == 0)
		{
			encrypt = &argv[i][2];
			for (int j = 0; encrypt[j] != '\0'; j++)
			{
				encryptSize = j;
			}
			enc = 1;
		}
		if (strncmp(argv[i], "-i", 2) == 0)
		{
			input = fopen(&argv[i][2],"r");
		}
		if (strncmp(argv[i], "-o", 2) == 0)
		{
			output = fopen(&argv[i][2],"w");
		}
	}
	// end of configuration
	while ((a = fgetc(input)) != EOF)
	{
		int endOfLine = 0;
		if (debug == 1)
			//taken from stackoverflow
			fprintf(stderr, "%#02x\t", a);
		if (a == '\n')
			endOfLine = 1;
		//change section
		if (enc == 0)
		{
			if (a >= 'A' && a <= 'Z')
				a += 'a'-'A';
		}
		if (enc == 1)
		{
			a += encrypt[counter % (encryptSize + 1)];
			counter++;
		}
		if (enc == -1)
		{
			a -= encrypt[counter % (encryptSize + 1)];
			counter++;
		}
		if (debug == 1)
			fprintf(stderr, "%#02x\n", a);
		//print after change
		fprintf(output, "%c", a);
		if (endOfLine == 1 && enc!=0)
		{
			fprintf(output, "\n");
			counter = 0;
		}
	}
	if(input!=stdin){
		fclose(input);
	}
	if(output!=stdout){
		fclose(output);
	}
	return 0;
}