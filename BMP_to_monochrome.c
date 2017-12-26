#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

#define INPUT_FILE_MISSING_ERROR_MEANS_HELP_COMMAND 1
#define MAX_PATH_LEN 150

enum commands_names {input_command, output_command, help_command};
const char *commands[] = {"--input=", "--output=", "--help", };

BYTE isEchoOn() {
#ifdef ECHO_ON
	return 1;
#else
	return 0;
#endif
}

int main(int argc, char *argv[]) {
	int i;
	char finName[MAX_PATH_LEN] = "test_in.bmp", foutName[MAX_PATH_LEN] = "test_out.bmp", foutBasicName[MAX_PATH_LEN] = "test_out";
	BMPError err = 0;
	bgr **bmD = NULL;
	bitMapFileHeader bmF;
	bitMapInfoHeader bmI;
	for (i = 1; i < argc; i++) {
		if (strstr(argv[i], commands[input_command]) != NULL) {
			finName[0] = '\0';
			strcat(finName, argv[i] + strlen(commands[input_command]));
		} else if (strstr(argv[i], commands[output_command]) != NULL) {
			foutBasicName[0] = '\0';
			strcat(foutBasicName, argv[i] + strlen(commands[input_command]));
		} else if (strstr(argv[i], commands[help_command]) != NULL) {
			fprintf(stderr,
		        "\tPrinting help:\nThis program recodes 1/4/8/16(5-5-5)/24/32-bit BMP file to 1(,4,8)-bit BMP file\n\n"
		        "Without arguments it will read from \"test_in.bmp\", "
#ifdef DEBUG
		        "write 1-bit output to \"test_out_1bit.bmp\", "
		        "4-bit output to \"test_out_4bit.bmp\" and 8-bit output to \"test_out_8bit.bmp\".\n"
#else
			    "and write 1-bit result bmp to test_out.bmp\n"
#endif
		        "Parameters can be:\n"
		        "\t\"%s\" - print this help\n"
		        "\t\"%s<path.format>\" - input file path\n"
#ifdef DEBUG
		        "\t\"%s<path_part>\" - output file path prefix: it will be concatinated with "
		        "\"_1bit.bmp\", \"_4bit.bmp\" and \"_8bit.bmp\"\n"
#else
			    "\t\"%s<path.format>\" - output file path\n"
#endif
		        , commands[help_command], commands[input_command], commands[output_command]
		       );
	       system("pause");
	       return 0;
		} else
			printf("Can't recognize \"%s\" (%d) input option\n", argv[i], i);
	}
	if (isEchoOn())
		fprintf(stderr, "Opening \"%s\" input:\n", finName);
	if (!(err = explainError(readBMP(finName, &bmF, &bmI, &bmD), !isEchoOn())))
	{
		freePixels(&bmD, &bmI);
		return 1;
	}
	if (isEchoOn())
		fprintf(stderr, "Opened picture: width = %lu, height = %lu, bitsPerPixel = %d, compression = %lu, bfOffBits = %lu, size = %lu bytes, biClrUsed = %lu\n",
		                 bmI.biWidth, bmI.biHeight, bmI.biBitCount, bmI.biCompression, bmF.bfOffBits, bmF.bfSize, bmI.biClrUsed);
// debug - output 1,4,8-bit output
#ifdef DEBUG
	strcpy(foutName, foutBasicName);
	strcat(foutName, "_1bit.bmp");
	if (isEchoOn()) {
		fprintf(stderr, "Wtiring 1-bit bmp output to \"%s\"...\n", foutName);
		if (explainError(writeBMP(foutName, &bmF, &bmI, bmD, 1), 0))
			fprintf(stderr, "\tOK\n");
		else
			fprintf(stderr, "\tError!\n");
	} else
		writeBMP(foutName, &bmF, &bmI, bmD, 1);

	strcpy(foutName, foutBasicName);
	strcat(foutName, "_4bit.bmp");
	if (isEchoOn()) {
		fprintf(stderr, "Writing 4-bit bmp output to \"%s\"...\n", foutName);
		if (explainError(writeBMP(foutName, &bmF, &bmI, bmD, 4), 0))
			fprintf(stderr, "\tOK\n");
		else
			fprintf(stderr, "\tError!\n");
	} else
		writeBMP(foutName, &bmF, &bmI, bmD, 4);

	strcpy(foutName, foutBasicName);
	strcat(foutName, "_8bit.bmp");
	if (isEchoOn()) {
		fprintf(stderr, "Writing 8-bit bmp output to \"%s\"...\n", foutName);
		if (explainError(writeBMP(foutName, &bmF, &bmI, bmD, 8), 0))
			fprintf(stderr, "\tOK\n");
		else
			fprintf(stderr, "\tError!\n");
	} else
		writeBMP(foutName, &bmF, &bmI, bmD, 8);

#else
// no debug mode - output only 1-bit bmp
	if (!explainError(writeBMP(foutName, &bmF, &bmI, bmD, 1), 1))
		fprintf(stderr, "\tError writing output file: \"%s\"!\n", foutName);
#endif
	freePixels(&bmD, &bmI);
	return 0;
}
