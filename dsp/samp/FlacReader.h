#pragma once

#include "stream_decoder.h"
#include <stdio.h>


class FlacReader {
public:
	~FlacReader();
    void read(const char* filePath);
	bool ok() const { return isOk; }
private:

	FLAC__StreamDecoder *decoder = nullptr;
//	FLAC__StreamDecoderInitStatus init_status;
//	FILE *fout = nullptr;
	bool isOk = false;

	static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data);
	static void metadata_callback(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data);
	static void error_callback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data);

};
