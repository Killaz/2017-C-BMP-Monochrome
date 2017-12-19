#ifndef BMP_H

#define BMP_H
#include <stdio.h>

#define upInt(a, b) ((a) + ((b) - (a) % (b)) % (b))

typedef unsigned long  int DWORD;
typedef          long  int LONG;
typedef unsigned short int WORD;
typedef unsigned char      BYTE;

#pragma pack(1)

typedef struct tagBitMapFileHeader {
	DWORD   bfSize, bfOffBits;
	WORD    bfType, bfReserved1, bfReserved2;
} __attribute__((packed)) bitMapFileHeader;

typedef struct tagBitMapInfoHeader {
	DWORD  biSize, biCompression, biSizeImage, biClrUsed, biClrImportant;
	LONG   biWidth, biHeight, biXPelsPerMeter, biYPelsPerMeter;
	WORD   biPlanes, biBitCount;
} __attribute__((packed)) bitMapInfoHeader;

#pragma pack()

typedef struct tagBgr {
	BYTE b, g, r;
} bgr;

typedef struct tagBgra {
	BYTE b, g, r, a;
} bgra;

typedef enum tagBMPError {OK, noInput, headerReading, infoReading, noMemory, noSuchInputBitCount, hasCompression, pixelReading,
                          noOutput, headerWriting, infoWriting, noSuchOutputBitCount, pixelWriting, notBMP = 0x100}
        BMPError;

void strCat(char *, const char *);
BYTE setrgba(bgra *, BYTE, BYTE, BYTE, BYTE);

BYTE explainError(BMPError, BYTE);

BMPError readBitMapFileHeader(FILE *, const bitMapFileHeader *);
BMPError writeBitMapFileHeader(FILE *, const bitMapFileHeader *);
BMPError readBitMapInfoHeader(FILE *, bitMapInfoHeader *);
BMPError writeBitMapInfoHeader(FILE *, const bitMapInfoHeader *);
BMPError readBMP(const char *, bitMapFileHeader *, bitMapInfoHeader *, bgr ***);
BMPError writeBMP(const char *, const bitMapFileHeader *, const bitMapInfoHeader *, bgr **, BYTE);
void freePixels(bgr ***, const bitMapInfoHeader *);

#endif