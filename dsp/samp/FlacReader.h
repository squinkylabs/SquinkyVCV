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
	bool isOk = false;

	float* monoData = nullptr;

	/*
	static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;
*/
	uint64_t samplesExpected = 0;
	void onFormat(uint64_t totalSamples, unsigned sampleRate, unsigned channels);
	void onData(const void* data, unsigned samples);

	static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data);
	static void metadata_callback(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data);
	static void error_callback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data);

};
