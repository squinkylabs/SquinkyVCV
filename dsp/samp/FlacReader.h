#pragma once

#include "stream_decoder.h"
#include <stdio.h>


class FlacReader {
public:
    void read(const char* filePath);
private:

	FLAC__StreamDecoder *decoder = nullptr;
//	FLAC__StreamDecoderInitStatus init_status;
	FILE *fout = nullptr;

};
