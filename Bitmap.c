#include "Bitmap.h"

static int Bitmap_readHeader2(Bitmap* this, FILE* f) {
	if (fread(&this->fileInfo, sizeof(this->fileInfo), 1, f) != 1) return -1;
	if (fread(&this->fileSize, sizeof(this->fileSize), 1, f) != 1) return -1;
	if (fread(&this->longTemp, sizeof(this->longTemp), 1, f) != 1) return -1;
	if (fread(&this->dataOffset, sizeof(this->dataOffset), 1, f) != 1) return -1;
	if (fread(&this->headerSize, sizeof(this->headerSize), 1, f) != 1) return -1;
	if (fread(&this->pictureWidth, sizeof(this->pictureWidth), 1, f) != 1) return -1;
	if (fread(&this->pictureHeight, sizeof(this->pictureHeight), 1, f) != 1) return -1;
	if (fread(&this->intTemp, sizeof(this->intTemp), 1, f) != 1) return -1;
	if (fread(&this->colorDepth, sizeof(this->colorDepth), 1, f) != 1) return -1;
	if (fread(&this->compressionType, sizeof(this->compressionType), 1, f) != 1) return -1;
	if (fread(&this->pictureSize, sizeof(this->pictureSize), 1, f) != 1) return -1;
	if (fread(&this->xPixelPerMeter, sizeof(this->xPixelPerMeter), 1, f) != 1) return -1;
	if (fread(&this->yPixelPerMeter, sizeof(this->yPixelPerMeter), 1, f) != 1) return -1;
	if (fread(&this->colorNumber, sizeof(this->colorNumber), 1, f) != 1) return -1;
	if (fread(&this->colorUsed, sizeof(this->colorUsed), 1, f) != 1) return -1;
	return 0;
}

int Bitmap_readHeader(Bitmap* this, FILE* f, char* errBuf, size_t errBufSize) {
	if (Bitmap_readHeader2(this, f) != 0) {
		snprintf(errBuf, errBufSize, "Error reading bitmap header [%s]", geterr());
		return -1;
	}
	return 0;
}

int Bitmap_validateHeader(Bitmap* this, char* errBuf, size_t errBufSize) {
	if (this->fileInfo[0] != 'B' || this->fileInfo[1] != 'M') {
		snprintf(errBuf, errBufSize, "Wrong file info: '%c%c' (expected: 'BM')!", this->fileInfo[0], this->fileInfo[1]);
		return -1;
	}
	if (this->pictureWidth != BITMAP_MAX_WIDTH || this->pictureHeight != BITMAP_MAX_HEIGHT) {
		snprintf(errBuf, errBufSize, "Wrong picture size: %dx%d (expected %dx%d)!",
			this->pictureWidth, this->pictureHeight, BITMAP_MAX_WIDTH, BITMAP_MAX_HEIGHT);
		return -1;
	}
	if (this->colorDepth != 24) {
		snprintf(errBuf, errBufSize, "Wrong color depth: %hu (expected be 24)!", this->colorDepth);
		return -1;
	}
	if (this->compressionType != 0) {
		snprintf(errBuf, errBufSize, "Wrong compression type: %u (expected 0)!", this->compressionType);
		return -1;
	}
	return 0;
}
