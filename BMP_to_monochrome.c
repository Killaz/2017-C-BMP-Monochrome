#include <stdlib.h>
#include <string.h>
#include "bmp.h"

#define INPUT_FILE_MISSING_ERROR_MEANS_HELP_COMMAND 1

int main(int argc, char *argv[]) {
	char finName[90] = "", foutName[90] = "", foutBasicName[90] = "";
	BMPError err = 0;
	bgr **bmD = NULL;
	bitMapFileHeader bmF;
	bitMapInfoHeader bmI;
	if (argc > 1)
		strCat(finName, argv[1]);
	else
		strCat(finName, "test_in.bmp");
	if (argc > 2)
		strCat(foutBasicName, argv[2]);
	else
		strCat(foutBasicName, "test_out");
#ifndef ECHO_OFF
	fprintf(stderr, "Opening \"%s\" input:\n", finName);
	if (!(err = explainError(readBMP(finName, &bmF, &bmI, &bmD), 0)))
#else
	if (!(err = explainError(readBMP(finName, &bmF, &bmI, &bmD), 1)))
#endif
	{
		if (INPUT_FILE_MISSING_ERROR_MEANS_HELP_COMMAND) {
			fprintf(stderr,
		        "\tPrinting help:\nThis program recodes 1/4/8/16(5-5-5)/24/32-bit BMP file to 1,4,8-bit BMP file\n\n"
		        "Without arguments it will read from \"test_in.bmp\", write 1-bit output to \"test_out_1bit.bmp\", "
		        "4-bit output to \"test_out_4bit.bmp\" and 8-bit output to \"test_out_8bit.bmp\".\n"
		        "If one parameter is given, it will be interpretaded as output file name: it will be concated with "
		        "\"_1bit.bmp\", \"_4bit.bmp\" and \"_8bit.bmp\".\n"
		       );
		       system("pause");
		}
		goto exitPoint;
	}


	foutName[0] = '\0';
	strCat(foutName, foutBasicName);
	strCat(foutName, "_1bit.bmp");
#ifndef ECHO_OFF
	fprintf(stderr, "Wtiring 1-bit bmp output to \"%s\"...\n", foutName);
	if (explainError(writeBMP(foutName, &bmF, &bmI, bmD, 1), 0))
		fprintf(stderr, "\tOK\n");
	else
		fprintf(stderr, "\tError!\n");
#else
	writeBMP(foutName, &bmF, &bmI, bmD, 1);
#endif

	foutName[0] = '\0';
	strCat(foutName, foutBasicName);
	strCat(foutName, "_4bit.bmp");
#ifndef ECHO_OFF
	fprintf(stderr, "Writing 4-bit bmp output to \"%s\"...\n", foutName);
	if (explainError(writeBMP(foutName, &bmF, &bmI, bmD, 4), 0))
		fprintf(stderr, "\tOK\n");
	else
		fprintf(stderr, "\tError!\n");
#else
	writeBMP(foutName, &bmF, &bmI, bmD, 4);
#endif

	foutName[0] = '\0';
	strCat(foutName, foutBasicName);
	strCat(foutName, "_8bit.bmp");
#ifndef ECHO_OFF
	fprintf(stderr, "Writing 8-bit bmp output to \"%s\"...\n", foutName);
	if (explainError(writeBMP(foutName, &bmF, &bmI, bmD, 8), 0))
		fprintf(stderr, "\tOK\n");
	else
		fprintf(stderr, "\tError!\n");
#else
	writeBMP(foutName, &bmF, &bmI, bmD, 8);
#endif
	exitPoint:
	freePixels(&bmD, &bmI);
	return 0;
}
