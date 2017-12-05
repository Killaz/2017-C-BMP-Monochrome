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

void strCat(char *s1, const char *s2) {
	if (s1 == NULL || s2 == NULL)
		return;
	while(*s1++ != 0);
	s1--;
	while((*s1++ = *s2++) != 0);
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
		} else if (!silent)
			fprintf(stderr, "Input file is not BMP, and there was error:\n");
	}
	err &= 0xFF;
	if (silent)
		return 0;
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
	if (fwrite((void*) &bmF->bfType, 2, 1, fout) > 0 && fwrite((void*) &bmF->bfSize, 4, 1, fout) > 0 &&
	    fwrite((void*) &bmF->bfReserved1, 2, 1, fout) > 0 && fwrite((void*) &bmF->bfReserved2, 2, 1, fout) > 0 &&
	    fwrite((void*) &bmF->bfOffBits, 4, 1, fout) > 0
	)
		return OK;
	return headerWriting;
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
	if (fwrite((void*) &bmI->biSize, 4, 1, fout) > 0 && fwrite((void*) &bmI->biWidth, 4, 1, fout) > 0 &&
	    fwrite((void*) &bmI->biHeight, 4, 1, fout) > 0 && fwrite((void*) &bmI->biPlanes, 2, 1, fout) > 0 &&
	    fwrite((void*) &bmI->biBitCount, 2, 1, fout) > 0 && fwrite((void*) &bmI->biCompression, 4, 1, fout) > 0 &&
	    fwrite((void*) &bmI->biSizeImage, 4, 1, fout) > 0 && fwrite((void*) &bmI->biXPelsPerMeter, 4, 1, fout) > 0 &&
	    fwrite((void*) &bmI->biYPelsPerMeter, 4, 1, fout) > 0 && fwrite((void*) &bmI->biClrUsed, 4, 1, fout) > 0 &&
	    fwrite((void*) &bmI->biClrImportant, 4, 1, fout) > 0
	)
		return OK;
	return infoWriting;
}

BMPError readBMP(const char *finName, bitMapFileHeader *bmF, bitMapInfoHeader *bmI, bgr ***bmD) {
	FILE *fin = fopen(finName, "rb");
	int i, j;
	BYTE number;
	BMPError err = OK;
	bgr palette[256] = {{0, 0, 0}};
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
			goto errorPoint;
		}
#ifndef ECHO_OFF
	fprintf(stderr, "Opened picture: width = %lu, height = %lu, bitsPerPixel = %d, compression = %lu, bfOffBits = %lu, size = %lu bytes, biClrUsed = %lu\n",
	                bmI->biWidth, bmI->biHeight, bmI->biBitCount, bmI->biCompression, bmF->bfOffBits, bmF->bfSize, bmI->biClrUsed);
#endif
	fseek(fin, 14 + bmI->biSize, SEEK_SET);
	if (bmI->biBitCount < 16)
		for (i = 0; i < (1 << bmI->biBitCount); i++) {
			if (fread(palette + i, 1, 3, fin) != 3) {
				break;
			}
			fseek(fin, 1, SEEK_CUR);
		}
	fseek(fin, bmF->bfOffBits, SEEK_SET);
	if (bmI->biBitCount == 24)
		for (i = bmI->biHeight - 1; i >= 0; i--) {
			for (j = 0; j < bmI->biWidth; j++)
				if (fread((void *) ((*bmD)[i] + j), 1, 3, fin) != 3) {
					err |= pixelReading;
					goto errorPoint;
				}
			fseek(fin, bmI->biWidth % 4, SEEK_CUR);
		}
	else if (bmI->biBitCount == 8)
		for (i = bmI->biHeight - 1; i >= 0; i--) {
			for (j = 0; j < bmI->biWidth; j++) {
				if (fread((void *) &number, 1, 1, fin) != 1) {
					err |= pixelReading;
					goto errorPoint;
				}
				(*bmD)[i][j] = palette[number];
			}
			fseek(fin, (3 * bmI->biWidth) % 4, SEEK_CUR);
		}
	else if (bmI->biBitCount == 4)
		for (i = bmI->biHeight - 1; i >= 0; i--) {
			for (j = 0; j < bmI->biWidth; j += 2) {
				if (fread((void *) &number, 1, 1, fin) != 1) {
					err |= pixelReading;
					goto errorPoint;
				}
				(*bmD)[i][j] = palette[number >> 4];
				if (j + 1 < bmI->biWidth)
					(*bmD)[i][j + 1] = palette[number & 0xF];
			}
			fseek(fin, (3 * ((bmI->biWidth & 1) == 0 ? bmI->biWidth : bmI->biWidth + 1) / 2) % 4, SEEK_CUR);
		}
	else if (bmI->biBitCount == 1)
		for (i = bmI->biHeight - 1; i >= 0; i--) {
			for (j = 0; j < bmI->biWidth; j += 8) {
				int k;
				if (fread((void *) &number, 1, 1, fin) != 1) {
					err |= pixelReading;
					goto errorPoint;
				}
				for (k = 0; k < 8 && j + k < bmI->biWidth; k++) {
					(*bmD)[i][j + k] = palette[(number >> (7 - k)) & 1];
				}
			}
			fseek(fin, (3 * upInt(bmI->biWidth, 8) / 8) % 4, SEEK_CUR);
		}
	else if (bmI->biBitCount == 32)
		for (i = bmI->biHeight - 1; i >= 0; i--) {
			for (j = 0; j < bmI->biWidth; j++) {
				fseek(fin, 1, SEEK_CUR);
				if (fread((void *) &(*bmD)[i][j], 1, 3, fin) != 3) {
					err |= pixelReading;
					goto errorPoint;
				}
			}
		}
	else if (bmI->biBitCount == 16) {
		WORD pixel;
		for (i = bmI->biHeight - 1; i >= 0; i--) {
			for (j = 0; j < bmI->biWidth; j++) {
				if (fread((void *) &pixel, 1, 2, fin) != 2) {
					err |= pixelReading;
					goto errorPoint;
				}
				(*bmD)[i][j].b = (pixel & 0x1F) * 255 / 31.0;
				(*bmD)[i][j].g = ((pixel >> 5) & 0x1F) * 255 / 31.0;
				(*bmD)[i][j].r = ((pixel >> 10) & 0x1F) * 255 / 31.0;
			}
			fseek(fin, (2 * bmI->biWidth) % 4, SEEK_CUR);
		}
	} else {
		err |= noSuchInputBitCount;
		goto errorPoint;
	}
	fclose(fin);
	if (0) {
		errorPoint:
		fclose(fin);
		for (i = 0; i < bmI->biHeight; i++) {
			free((*bmD)[i]);
			(*bmD)[i] = NULL;
		}
		free(*bmD);
		*bmD = NULL;
	}
	return err;
}

BMPError writeBMP(const char *foutName, const bitMapFileHeader *bmFIn, const bitMapInfoHeader *bmIIn, bgr **bmD, BYTE newBitCount) {
	FILE *fout;
	LONG i, j;
	bgra tmp;
	BMPError err = OK;
	bitMapFileHeader *bmF;
	bitMapInfoHeader *bmI;
	if (newBitCount != 1 && newBitCount != 4 && newBitCount != 8)
		return noSuchOutputBitCount;
	bmF = (bitMapFileHeader *) malloc(sizeof(bitMapFileHeader));
	bmI = (bitMapInfoHeader *) malloc(sizeof(bitMapInfoHeader));
	if (bmF == NULL || bmI == NULL)
		return noMemory;
	if ((fout = fopen(foutName, "wb")) == NULL)
		return noOutput;
	memcpy(bmF, bmFIn, sizeof(bitMapFileHeader));
	memcpy(bmI, bmIIn, sizeof(bitMapInfoHeader));
	bmI->biSize = sizeof(bitMapInfoHeader);
	bmF->bfOffBits = sizeof(bitMapFileHeader) + bmI->biSize + (1 << newBitCount) * sizeof(bgra);
	bmI->biCompression = 0;
	if (newBitCount == 1)
		bmF->bfSize = bmF->bfOffBits + upInt(bmI->biWidth, 8) / 8 * bmI->biHeight * sizeof(BYTE) +
		              bmI->biHeight * ((3 * upInt(bmI->biWidth, 8) / 8) % 4) * sizeof(BYTE);
	else if (newBitCount == 4)
		bmF->bfSize = bmF->bfOffBits + upInt(bmI->biWidth, 8) / 4 * bmI->biHeight * sizeof(WORD) +
	                  bmI->biHeight * (3 * ((bmIIn->biWidth & 1) == 0 ? bmIIn->biWidth : bmIIn->biWidth + 1) / 2) % 4 * sizeof(BYTE);
	else
		bmF->bfSize = bmF->bfOffBits + upInt(bmI->biWidth, 8) * bmI->biHeight * sizeof(BYTE) +
	                  bmI->biHeight * (3 * bmIIn->biWidth) % 4 * sizeof(BYTE);
	bmI->biBitCount = newBitCount;
	err = writeBitMapFileHeader(fout, bmF) | writeBitMapInfoHeader(fout, bmI);
	free(bmI);
	free(bmF);
	if (err != OK || err != OK) {
		fclose(fout);
		return err;
	}
	bmI = NULL, bmF = NULL;
	if (newBitCount == 1) {
		unsigned long long median = 0;
		setrgba(&tmp, 0, 0, 0, 0);
		if (fwrite(&tmp, 1, 4, fout) != 4) {
			fclose(fout);
			return pixelWriting;
		}
		setrgba(&tmp, 255, 255, 255, 0);
		if (fwrite(&tmp, 1, 4, fout) != 4) {
			fclose(fout);
			return pixelWriting;
		}
		for (i = 0; i < bmIIn->biHeight; i++)
			for (j = 0; j < bmIIn->biWidth; j++)
				median += (bmD[i][j].r + bmD[i][j].g + bmD[i][j].b);
		median /= (bmIIn->biHeight * bmIIn->biWidth);
		for (i = bmIIn->biHeight - 1; i >= 0; i--) {
			BYTE number = 0;
			for (j = 0; j < bmIIn->biWidth; j += 8) {
				int k;
				number = 0;
				for (k = 0; k < 8 && j + k < bmIIn->biWidth; k++)
					if (bmD[i][j + k].r + bmD[i][j + k].g + bmD[i][j + k].b > (int) median)
						number |= (1 << (7 - k));
				if (fwrite(&number, 1, 1, fout) != 1) {
					fclose(fout);
					return pixelWriting;
				}
			}
			number = 0;
			for (j = 0; j < (3 * upInt(bmIIn->biWidth, 8) / 8) % 4; j++)
				if (fwrite(&number, 1, 1, fout) != 1) {
					fclose(fout);
					return pixelWriting;
				}
		}
	} else if (newBitCount == 4) {
		for (i = 0; i < 256; i += 17) {
			setrgba(&tmp, i, i, i, 0);
			if (fwrite(&tmp, 1, 4, fout) != 4) {
				fclose(fout);
				return pixelWriting;
			}
		}
		for (i = bmIIn->biHeight - 1; i >= 0; i--) {
			BYTE number = 0;
			for (j = 0; j < bmIIn->biWidth; j += 2) {
				number = 0;
				number |= ((BYTE)((bmD[i][j].r + bmD[i][j].g + bmD[i][j].b) / 51.2 + 0.5) << 4);
				if (j + 1 < bmIIn->biWidth)
					number |= (BYTE)((bmD[i][j + 1].r + bmD[i][j + 1].g + bmD[i][j + 1].b) / 51.2 + 0.5);
				if ((fwrite(&number, 1, 1, fout) != 1)) {
					fclose(fout);
					return pixelWriting;
				}
			}
			number = 0;
			for (j = 0; j < (3 * ((bmIIn->biWidth & 1) == 0 ? bmIIn->biWidth : bmIIn->biWidth + 1) / 2) % 4; j++)
				if (fwrite(&number, 1, 1, fout) != 1) {
					fclose(fout);
					return pixelWriting;
				}
		}
	} else {
		for (i = 0; i < 256; i += 1) {
			setrgba(&tmp, i, i, i, 0);
			if (fwrite(&tmp, 1, 4, fout) != 4) {
				fclose(fout);
				return pixelWriting;
			}
		}
		for (i = bmIIn->biHeight - 1; i >= 0; i--) {
			BYTE number = 0;
			for (j = 0; j < bmIIn->biWidth; j++) {
				number = (BYTE) ((bmD[i][j].r + bmD[i][j].g + bmD[i][j].b) / 3.0 + 0.5);
				if (fwrite((void *) &number, 1, 1, fout) != 1) {
					fclose(fout);
					return pixelWriting;
				}
			}
			number = 0;
			for (j = 0; j < (3 * bmIIn->biWidth) % 4; j++)
				if (fwrite(&number, 1, 1, fout) != 1) {
					fclose(fout);
					return pixelWriting;
				}
		}
	}
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