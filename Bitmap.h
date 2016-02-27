#ifndef PLOTTER_CDROM_BITMAP_H
#define PLOTTER_CDROM_BITMAP_H

#include "common.h"

struct Bitmap {
	unsigned char fileInfo[2];
	uint32_t fileSize;
	uint32_t longTemp;
	uint32_t dataOffset;
	uint32_t headerSize;
	int32_t  pictureWidth;
	int32_t  pictureHeight;
	uint16_t intTemp;
	uint16_t colorDepth;
	uint32_t compressionType;
	uint32_t pictureSize;
	uint32_t xPixelPerMeter;
	uint32_t yPixelPerMeter;
	uint32_t colorNumber;
	uint32_t colorUsed;
};

int Bitmap_readHeader(Bitmap* this, FILE* f, char* errBuf, size_t errBufSize);

int Bitmap_validateHeader(Bitmap* this, char* errBuf, size_t errBufSize);

#endif
