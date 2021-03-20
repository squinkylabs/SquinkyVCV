
#include "FlacReader.h"
#include "SqLog.h"

#include <assert.h>
#include <limits>
#include <stdlib.h>
//#include "stream_decoder.h"

void FlacReader::read(const char* filePath) {
	if (!filePath) {
		SQWARN("bogus path");
		return;
	}
	if ((decoder = FLAC__stream_decoder_new()) == NULL) {
		SQWARN("ERROR: allocating flac decoder");
		return;
	}

	FLAC__stream_decoder_set_md5_checking(decoder, false);

	auto init_status = FLAC__stream_decoder_init_file(decoder, filePath, write_callback, metadata_callback, error_callback, /*client_data=*/this);
	if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		SQWARN("ERROR: initializing decoder: %s", FLAC__StreamDecoderInitStatusString[init_status]);
		isOk = false;
		return;
	}


	FLAC__bool ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
	
	isOk = (ok != false);


}

/*
static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;
*/

FlacReader::~FlacReader() {
	delete decoder;
	free(monoData);
}

void FlacReader::onFormat(uint64_t totalSamples, unsigned sampleRate, unsigned channels, unsigned bitspersample) {
	// flac calls something a "sample" even if it's a single multi-channel frame.
	framesExpected = totalSamples;
	framesRead = 0;
	assert(!monoData);

	// since we convert to mono on the fly, frames on input -> samples on output.
	void* p = malloc(framesExpected * sizeof(float));
	monoData = reinterpret_cast<float*>(p);
	writePtr = monoData;
	channels_ = channels;
	bitsPerSample_ = bitspersample;
}


float FlacReader::read16Bit(const void* data) {
	const int16_t* data16 = reinterpret_cast<const int16_t *>(data);
	float x = float(*data16) * (1.f / std::numeric_limits<int16_t>::max());
	return x;
}
float FlacReader::read24Bit(const void* data) {
	const uint8_t* lsb = reinterpret_cast<const uint8_t*>(data);
	const int16_t* data16 = reinterpret_cast<const int16_t*>(lsb+1);

	const float max24 = float(std::numeric_limits<int16_t>::max() * 256);
	int i = (*data16 << 8) | *lsb;
	float x = i * max24;
	return x;
}

bool FlacReader::onData(const void* data, unsigned samples) {

	const unsigned framesInBlock = samples;

	if(framesExpected == 0) {
		SQWARN("empty flac");
		return false;
	}
	if (channels_ != 1 && channels_ != 2) {
		SQWARN("can only decode stereo and mono flac");
		return false;
	}

	if(bitsPerSample_ != 16 && bitsPerSample_ != 24) {
		SQWARN("can only accept 16 and 24 bit flac\n");
		return false;
	}

	if (framesRead == 0) {
		SQINFO("first frame");
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data); 
		for (int i=0; i<16; ++i) {
			SQINFO("byte[%d]=%x", i, p[i]);
		}
	}

	// copy data
	const int8_t* cdata = reinterpret_cast<const int8_t*>(data);
	if (channels_==1 && bitsPerSample_==16) {
		while (samples) {
			float x = read16Bit(cdata);
			cdata += 2;
			*writePtr++ = x;
			samples -= 1;
		}
	} else if (channels_==2 && bitsPerSample_==16) {
		while (samples) {
			float x = read16Bit(cdata);
			cdata += 2;
			x += read16Bit(data);
			cdata += 2;
			*writePtr++ = x / 2;
			samples -= 2;
		}
	} else if (channels_==1 && bitsPerSample_==24) {
		while (samples) {
			float x = read24Bit(cdata);
			cdata += 3;
			*writePtr++ = x;
			samples -= 1;
		}
	} else if (channels_==2 && bitsPerSample_==24) {
		while(samples) {
			float x = read24Bit(cdata);
			cdata += 3;
			x += read24Bit(data);
			cdata += 3;
			*writePtr++ = x / 2;
			samples -= 2;
		}
	} else assert(false);

	framesRead += framesInBlock;
	if (framesRead >= framesExpected) {
		isOk = true;
	}
	if (isOk) SQINFO("leaving block with %d framesRaad %lld expected", framesRead, framesExpected);
	return true;
}

FLAC__StreamDecoderWriteStatus FlacReader::write_callback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data) {

	FlacReader* client = reinterpret_cast<FlacReader *>(client_data);
	const bool ok = client->onData(buffer, frame->header.blocksize);
	

	return ok ? FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE : FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
}
void FlacReader::metadata_callback(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data) {
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		FlacReader* client = reinterpret_cast<FlacReader*>(client_data);
		client->onFormat(metadata->data.stream_info.total_samples,
			metadata->data.stream_info.sample_rate,
			metadata->data.stream_info.channels,
			metadata->data.stream_info.bits_per_sample
			);
	}
}
void FlacReader::error_callback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data) {
	// TODO
	assert(false);
}
