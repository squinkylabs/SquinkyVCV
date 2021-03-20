#pragma once

#include "stream_decoder.h"
#include <stdio.h>


class FlacReader {
public:
	~FlacReader();
    void read(const char* filePath);
	bool ok() const { return isOk; }

	const float* getSamples() const { return monoData; }
	uint64_t getNumSamples() const { return framesRead; }
private:
	static float read16Bit(const void *);
	static float read24Bit(const void *);

	FLAC__StreamDecoder *decoder = nullptr;
	bool isOk = false;

	float* monoData = nullptr;
	float* writePtr = nullptr;

	uint64_t framesExpected = 0;
	uint64_t framesRead = 0;
	unsigned bitsPerSample = 0;
	unsigned channels_ = 0;
	unsigned bitsPerSample_ = 0;
	// bps = metadata->data.stream_info.bits_per_sample;
	void onFormat(uint64_t totalSamples, unsigned sampleRate, unsigned channels, unsigned bitsPerSampl );

	/**
	 * returns true if ok
	 */
	bool onData(const void* data, unsigned samples);

	static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data);
	static void metadata_callback(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data);
	static void error_callback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data);
};


