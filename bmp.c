#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"

BYTE setrgba(bgra *pixel, BYTE r, BYTE g, BYTE b, BYTE a) {
	if (pixel == NULL) {
		return 0;
	}
	pixel->r = r;
	pixel->g = g;
	pixel->b = b;
	pixel->a = a;
	return 1;
}

BYTE explainError(BMPError err, BYTE silent) {
	if (err == OK) {
		if (!silent)
			fprintf(stderr, "Success\n");
		return 1;
	}
	if ((err & 0x100) != 0) {
		if ((err & 0xFF) == 0) {
			if (!silent)
				fprintf(stderr, "Input is probably not BMP file, but it was read as it would be bmp (results may be incorrect)\n");
			return 1;
		} else {
			fprintf(stderr, "Input file is not BMP, and there was error:\n");
			return 0;
		}
	}
	err &= 0xFF;
/*	if (silent)
		return 0;*/
	if (err == noInput) {
		fprintf(stderr, "Input file is missing.\n");
	} else if (err == headerReading)
		fprintf(stderr, "Error while reading BMP header part.\n");
	else if (err == infoReading)
		fprintf(stderr, "Error while reading BMP info part.\n");
	else if (err == noMemory)
		fprintf(stderr, "There was memory allocation problem somewhere.\n");
	else if (err == noSuchInputBitCount)
		fprintf(stderr, "This is neither 1/4/8/16/24/32-bit BMP\n");
	else if (err == hasCompression)
		fprintf(stderr, "This BMP is compressed. Where did you get it?... :o\n");
	else if (err == pixelReading)
		fprintf(stderr, "There was an error while reading pixel canvas. Probably, end of file was reached. Results are erased.\n");
	else if (err == noOutput)
		fprintf(stderr, "There is an error of opening output file.\n");
	else if (err == headerWriting)
		fprintf(stderr, "Error while writing BMP header part\n");
	else if (err == infoWriting)
		fprintf(stderr, "Error while writing BMP info part.\n");
	else if (err == noSuchOutputBitCount)
		fprintf(stderr, "Wrong writing parameter: biBitCount.\n");
	else if (err == pixelWriting)
		fprintf(stderr, "There was an error while writing pixel canvas.\n");
	else
		fprintf(stderr, "Unknown error: number %d.\n", err);
	return 0;
}

BMPError readBitMapFileHeader(FILE *fin, const bitMapFileHeader *bmF) {
	if (fin == NULL)
		return noInput;
	if (!(fread((void*) &bmF->bfType, 2, 1, fin) > 0 && fread((void*) &bmF->bfSize, 4, 1, fin) > 0 &&
	      fread((void*) &bmF->bfReserved1, 2, 1, fin) > 0 && fread((void*) &bmF->bfReserved2, 2, 1, fin) > 0 &&
	      fread((void*) &bmF->bfOffBits, 4, 1, fin) > 0)
	)
		return headerReading;
	if (*(char*)&(bmF->bfType) != 'B' || ((char*)(&(bmF->bfType)))[1] != 'M')
		return notBMP;
	return OK;
}

BMPError writeBitMapFileHeader(FILE *fout, const bitMapFileHeader *bmF) {
	if (fout == NULL)
		return noOutput;
	fwrite((void*) &bmF->bfType, 2, 1, fout);
	fwrite((void*) &bmF->bfSize, 4, 1, fout);
	fwrite((void*) &bmF->bfReserved1, 2, 1, fout);
	fwrite((void*) &bmF->bfReserved2, 2, 1, fout);
	fwrite((void*) &bmF->bfOffBits, 4, 1, fout);
	return OK;
}

BMPError readBitMapInfoHeader(FILE *fin, bitMapInfoHeader *bmI) {
	if (fread((void*) &bmI->biSize, 4, 1, fin) > 0 && fread((void*) &bmI->biWidth, 4, 1, fin) > 0 &&
	    fread((void*) &bmI->biHeight, 4, 1, fin) > 0 && fread((void*) &bmI->biPlanes, 2, 1, fin) > 0 &&
	    fread((void*) &bmI->biBitCount, 2, 1, fin) > 0 && fread((void*) &bmI->biCompression, 4, 1, fin) > 0 &&
	    fread((void*) &bmI->biSizeImage, 4, 1, fin) > 0 && fread((void*) &bmI->biXPelsPerMeter, 4, 1, fin) > 0 &&
	    fread((void*) &bmI->biYPelsPerMeter, 4, 1, fin) > 0 && fread((void*) &bmI->biClrUsed, 4, 1, fin) > 0 &&
	    fread((void*) &bmI->biClrImportant, 4, 1, fin) > 0
	)
		return OK;
	return infoReading;
}

BMPError writeBitMapInfoHeader(FILE *fout, const bitMapInfoHeader *bmI) {
	fwrite((void*) &bmI->biSize, 4, 1, fout);
	fwrite((void*) &bmI->biWidth, 4, 1, fout);
	fwrite((void*) &bmI->biHeight, 4, 1, fout);
	fwrite((void*) &bmI->biPlanes, 2, 1, fout);
	fwrite((void*) &bmI->biBitCount, 2, 1, fout);
	fwrite((void*) &bmI->biCompression, 4, 1, fout);
	fwrite((void*) &bmI->biSizeImage, 4, 1, fout);
	fwrite((void*) &bmI->biXPelsPerMeter, 4, 1, fout);
	fwrite((void*) &bmI->biYPelsPerMeter, 4, 1, fout);
	fwrite((void*) &bmI->biClrUsed, 4, 1, fout);
	fwrite((void*) &bmI->biClrImportant, 4, 1, fout);
	return OK;
}

void readingError(FILE **f, bgr ***bmD, bitMapInfoHeader *bmI) {
	int i;
	fclose(*f);
	*f = NULL;
	for (i = 0; i < bmI->biHeight; i++) {
		free((*bmD)[i]);
		(*bmD)[i] = NULL;
	}
	free(*bmD);
	*bmD = NULL;
}

void readCanvasWithPalette(FILE **fin, bgr ***bmD, bitMapInfoHeader *bmI, bgr *palette, BMPError *err) {
	BYTE number;
	int i, j, k;
	int skip = (3 * bmI->biWidth) % 4;
	if (bmI->biBitCount == 4)
		skip = (3 * ((bmI->biWidth & 1) == 0 ? bmI->biWidth : bmI->biWidth + 1) / 2) % 4;
	else if (bmI->biBitCount == 2)
		skip = (3 * upInt(bmI->biWidth, 8) / 8) % 4;
	for (i = bmI->biHeight - 1; i >= 0; i--) {
		for (j = 0; j < bmI->biWidth; j++) {
			if (fread((void *) &number, 1, 1, *fin) != 1) {
				*err |= pixelReading;
				readingError(fin, bmD, bmI);
			}
			if (bmI->biBitCount == 8)
				(*bmD)[i][j] = palette[number];
			else if (bmI->biBitCount == 4) {
				(*bmD)[i][j] = palette[number >> 4];
				if (j + 1 < bmI->biWidth)
					(*bmD)[i][j + 1] = palette[number & 0xF];
			} else
				for (k = 0; k < 8 && j + k < bmI->biWidth; k++) {
					(*bmD)[i][j + k] = palette[(number >> (7 - k)) & 1];
				}
		}
		fseek(*fin, skip, SEEK_CUR);
	}
}

void readCanvasWithoutPalette(FILE **fin, bgr ***bmD, bitMapInfoHeader *bmI, BMPError *err) {
	int i, j;
	for (i = bmI->biHeight - 1; i >= 0; i--) {
		for (j = 0; j < bmI->biWidth; j++) {
			if (bmI->biBitCount > 16) {
				if (bmI->biBitCount == 32)
					fseek(*fin, 1, SEEK_CUR);
				if (fread((void *) &(*bmD)[i][j], 1, 3, *fin) != 3) {
					*err |= pixelReading;
					readingError(fin, bmD, bmI);
				}
			} else {
				WORD pixel;
				if (fread((void *) &pixel, 1, 2, *fin) != 2) {
					*err |= pixelReading;
					readingError(fin, bmD, bmI);
				}
				(*bmD)[i][j].b = (pixel & 0x1F) * 255 / 31.0;
				(*bmD)[i][j].g = ((pixel >> 5) & 0x1F) * 255 / 31.0;
				(*bmD)[i][j].r = ((pixel >> 10) & 0x1F) * 255 / 31.0;
			}
		}
		if (bmI->biBitCount < 32)
			fseek(*fin, (1 + ((bmI->biBitCount - 16) > 0)) * bmI->biWidth % 4, SEEK_CUR);
	}
}

void readPixelCanvas(FILE **fin, bgr ***bmD, bitMapFileHeader *bmF, bitMapInfoHeader *bmI, BMPError *err) {
	int i;
	bgr palette[256] = {{0, 0, 0}};
	fseek(*fin, 14 + bmI->biSize, SEEK_SET);
	if (bmI->biBitCount < 16)
		for (i = 0; i < (1 << bmI->biBitCount); i++) {
			if (fread(palette + i, 1, 3, *fin) != 3) {
				break;
			}
			fseek(*fin, 1, SEEK_CUR);
		}
	fseek(*fin, bmF->bfOffBits, SEEK_SET);
	if (bmI->biBitCount == 8 || bmI->biBitCount == 4 || bmI->biBitCount == 1)
		readCanvasWithPalette(fin, bmD, bmI, palette, err);
	else if (bmI->biBitCount == 32 || bmI->biBitCount == 24 || bmI->biBitCount == 16)
		readCanvasWithoutPalette(fin, bmD, bmI, err);
	else {
		*err |= noSuchInputBitCount;
		readingError(fin, bmD, bmI);
	}
}

BMPError readBMP(const char *finName, bitMapFileHeader *bmF, bitMapInfoHeader *bmI, bgr ***bmD) {
	FILE *fin = fopen(finName, "rb");
	int i;
	BMPError err = OK;
	if (fin == NULL)
		return noInput;
	if ((err = readBitMapFileHeader(fin, bmF)) != OK)
		if (err != notBMP)
			return err;
	if (((err |= readBitMapInfoHeader(fin, bmI)) & 0xFF) != OK)
		return err;
	if ((*bmD = (bgr **) malloc(bmI->biHeight * sizeof(bgr *))) == NULL)
		return err | noMemory;
	for (i = 0; i < bmI->biHeight; i++)
		if (((*bmD)[i] = malloc(bmI->biWidth * sizeof(bgr))) == NULL) {
			err |= noMemory;
			readingError(&fin, bmD, bmI);
		}
	readPixelCanvas(&fin, bmD, bmF, bmI, &err);
	if (fin != NULL)
		fclose(fin);
	return err;
}

BMPError writeHeaders(FILE **fout, const char *foutName, bitMapFileHeader bmF, bitMapInfoHeader bmI, int newBitCount) {
	BMPError err;
	if (newBitCount != 1 && newBitCount != 4 && newBitCount != 8)
		return noSuchOutputBitCount;
	if ((*fout = fopen(foutName, "wb")) == NULL)
		return noOutput;
	bmI.biSize = sizeof(bitMapInfoHeader);
	bmF.bfOffBits = sizeof(bitMapFileHeader) + bmI.biSize + (1 << newBitCount) * sizeof(bgra);
	bmI.biCompression = 0;
	if (newBitCount == 1)
		bmF.bfSize = bmF.bfOffBits + upInt(bmI.biWidth, 8) / 8 * bmI.biHeight * sizeof(BYTE) +
		              bmI.biHeight * ((3 * upInt(bmI.biWidth, 8) / 8) % 4) * sizeof(BYTE);
	else if (newBitCount == 4)
		bmF.bfSize = bmF.bfOffBits + upInt(bmI.biWidth, 8) / 4 * bmI.biHeight * sizeof(WORD) +
	                 bmI.biHeight * (3 * ((bmI.biWidth & 1) == 0 ? bmI.biWidth : bmI.biWidth + 1) / 2) % 4 * sizeof(BYTE);
	else
		bmF.bfSize = bmF.bfOffBits + upInt(bmI.biWidth, 8) * bmI.biHeight * sizeof(BYTE) +
	                 bmI.biHeight * (3 * bmI.biWidth) % 4 * sizeof(BYTE);
	bmI.biBitCount = newBitCount;
	if ((err = writeBitMapFileHeader(*fout, &bmF) | writeBitMapInfoHeader(*fout, &bmI)) != OK) {
		fclose(*fout);
		*fout = NULL;
		return err;
	}
	return OK;
}

void writePixelCanvas1bit(FILE *fout, const bitMapInfoHeader *bmI, bgr **bmD) {
	unsigned long long median = 0;
	int i, j, k;
	int skip = (3 * upInt(bmI->biWidth, 8) / 8) % 4;
	for (i = 0; i < bmI->biHeight; i++)
		for (j = 0; j < bmI->biWidth; j++)
			median += (bmD[i][j].r + bmD[i][j].g + bmD[i][j].b);
	median /= (bmI->biHeight * bmI->biWidth);
	for (i = bmI->biHeight - 1; i >= 0; i--) {
		BYTE number = 0;
		for (j = 0; j < bmI->biWidth; j += 8) {
			number = 0;
			for (k = 0; k < 8 && j + k < bmI->biWidth; k++)
				if (bmD[i][j + k].r + bmD[i][j + k].g + bmD[i][j + k].b > (BYTE) median)
					number |= (1 << (7 - k));
			fwrite(&number, 1, 1, fout);
		}
		number = 0;
		for (j = 0; j < skip; j++)
			fwrite(&number, 1, 1, fout);
 	}
}

void writePixelCanvas4_8bit(FILE *fout, const bitMapInfoHeader *bmI, bgr **bmD, int newBitCount) {
	int i, j, k;
	int skip = (3 * ((bmI->biWidth & 1) == 0 ? bmI->biWidth : bmI->biWidth + 1) / 2) % 4;
	if (newBitCount == 8)
		skip = (3 * bmI->biWidth) % 4;
	for (i = bmI->biHeight - 1; i >= 0; i--) {
		BYTE number = 0;
		for (j = 0; j < bmI->biWidth; j += 8 / newBitCount) {
			number = 0;
			for (k = 0; k < 8 / newBitCount && j + k < bmI->biWidth; k++)
				number |= ((BYTE)((bmD[i][j + k].r + bmD[i][j + k].g + bmD[i][j + k].b) / (51.0 / (newBitCount / 8 * 16 + 1)) + 0.5)
				                 << (8 - newBitCount) * (1 - k));
			fwrite(&number, 1, 1, fout);
		}
		number = 0;
		for (j = 0; j < skip; j++)
			fwrite(&number, 1, 1, fout);
 	}
}

BMPError writeBMP(const char *foutName, const bitMapFileHeader *bmF, const bitMapInfoHeader *bmI, bgr **bmD, BYTE newBitCount) {
	FILE *fout = NULL;
	LONG i, steps = 255 / ((1 << newBitCount) - 1);;
	bgra tmp;
	BMPError err = OK;
	if ((err = writeHeaders(&fout, foutName, *bmF, *bmI, newBitCount)) != OK)
		return err;
	for (i = 0; i < 256; i += steps) {
		setrgba(&tmp, i, i, i, 0);
		fwrite(&tmp, 1, 4, fout);
 	}
 	if (newBitCount == 1)
		writePixelCanvas1bit(fout, bmI, bmD);
	else
		writePixelCanvas4_8bit(fout, bmI, bmD, newBitCount);
	fclose(fout);
	return OK;
}

void freePixels(bgr ***bmD, const bitMapInfoHeader *bmI) {
	int i;
	if (bmD != NULL && *bmD != NULL) {
		for (i = 0; i < bmI->biHeight; i++) {
			free((*bmD)[i]);
			(*bmD)[i] = NULL;
		}
		free(*bmD);
	}
	*bmD = NULL;
}