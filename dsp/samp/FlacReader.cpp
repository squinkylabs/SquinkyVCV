
#include "FlacReader.h"

#include <assert.h>
#include <stdlib.h>

#include <limits>

#include "FilePath.h"
#include "SqLog.h"
#include "share/windows_unicode_filenames.h"


const float* FlacReader::getSamples() const {
    return monoData;
}

float* FlacReader::takeSampleBuffer() {
    // transfer ownership to caller.
    float* ret = monoData;
    monoData = nullptr;
    return ret;
}

uint64_t FlacReader::getNumSamples() const {
    return framesRead;
}

unsigned int FlacReader::getSampleRate() {
    return sampleRate_;
}

uint64_t FlacReader::getTotalFrameCount() {
    return framesRead;
}

void FlacReader::read(const FilePath& filePath) {
    if (filePath.empty()) {
        SQWARN("bogus path");
        return;
    }
    if ((decoder = FLAC__stream_decoder_new()) == NULL) {
        SQWARN("ERROR: allocating flac decoder");
        return;
    }

    FLAC__stream_decoder_set_md5_checking(decoder, false);
    flac_set_utf8_filenames(true);
    auto init_status = FLAC__stream_decoder_init_file(decoder, filePath.toString().c_str(), write_callback, metadata_callback, error_callback, /*client_data=*/this);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        SQWARN("ERROR: initializing decoder: %s", FLAC__StreamDecoderInitStatusString[init_status]);
        SQWARN("file path: >%s<", filePath.toString().c_str());
        isOk = false;
        return;
    }

    FLAC__bool ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
    FLAC__stream_decoder_finish(decoder);

    isOk = (ok != false);
}

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
    sampleRate_ = sampleRate;
}

float FlacReader::read16Bit(const int32_t* data) {
    //const int16_t* data16 = reinterpret_cast<const int16_t *>(data);
    int32_t input = *data;
    float x = float(input) * (1.f / std::numeric_limits<int16_t>::max());
    assert(x >= -1 && x <= 1);
    return x;
}

float FlacReader::read24Bit(const int32_t* data) {
    //  const uint8_t* lsb = reinterpret_cast<const uint8_t*>(data);
    //  const int16_t* data16 = reinterpret_cast<const int16_t*>(lsb + 1);
    //  assert(false);
    const float one_over_max24 = 1.f / float(std::numeric_limits<int16_t>::max() * 256);

    int32_t input = *data;
    float x = float(input) * one_over_max24;
    assert(x >= -1.01 && x <= 1.01);
    return x;
}

bool FlacReader::onData(unsigned samples, const int32_t* leftData, const int32_t* rightData) {
    const unsigned framesInBlock = samples;

    if (framesExpected == 0) {
        SQWARN("empty flac");
        return false;
    }
    if (channels_ != 1 && channels_ != 2) {
        SQWARN("can only decode stereo and mono flac");
        return false;
    }

    if (bitsPerSample_ != 16 && bitsPerSample_ != 24) {
        SQWARN("can only accept 16 and 24 bit flac\n");
        return false;
    }

#if 0
	if (framesRead == 0) {
		SQINFO("first frame L");
		const uint8_t* p = reinterpret_cast<const uint8_t*>(leftData); 
		for (int i=0; i<8; ++i) {
			SQINFO("lbyte[%d]=%x", i, p[i]);
		}
	}
	if (framesRead == 0 && rightData) {
		SQINFO("first frameR");
		const uint8_t* p = reinterpret_cast<const uint8_t*>(rightData); 
		for (int i=0; i<8; ++i) {
			SQINFO("lbyte[%d]=%x", i, p[i]);
		}
	}
#endif

    // copy data
    //const int8_t* cdata = reinterpret_cast<const int8_t*>(leftData);
    if (channels_ == 1 && bitsPerSample_ == 16) {
        while (samples) {
            float x = read16Bit(leftData++);
            //  ++leftData;
            *writePtr++ = x;
            samples -= 1;
        }

    } else if (channels_ == 2 && bitsPerSample_ == 16) {
        while (samples) {
            float x = read16Bit(leftData++);
            x += read16Bit(rightData++);
            *writePtr++ = x / 2;
            samples -= 1;
        }

    } else if (channels_ == 1 && bitsPerSample_ == 24) {
        while (samples) {
            float x = read24Bit(leftData++);
            *writePtr++ = x;
            samples -= 1;
        }
    } else if (channels_ == 2 && bitsPerSample_ == 24) {
        while (samples) {
            float x = read24Bit(leftData++);
            x += read24Bit(rightData++);
            *writePtr++ = x / 2;
            samples -= 1;
        }
    } else
        assert(false);

    framesRead += framesInBlock;
    if (framesRead >= framesExpected) {
        isOk = true;
    }
    // if (isOk) SQINFO("leaving block with %d framesRaad %lld expected", framesRead, framesExpected);
    return true;
}

FLAC__StreamDecoderWriteStatus FlacReader::write_callback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data) {
    FlacReader* client = reinterpret_cast<FlacReader*>(client_data);

#if 0  // from example
    for (int i = 0; i < 5; ++i) {
        auto x = buffer[0];
        auto y = buffer[0][i];

        auto z = (FLAC__int16)buffer[0][i]; /* left channel */
        int xx = 7;
    }
#endif

    const int32_t* leftData = nullptr;
    const int32_t* rightData = nullptr;

    leftData = buffer[0];
    if (client->channels_ > 1) {
        rightData = buffer[1];
    }
    const bool ok = client->onData(frame->header.blocksize, leftData, rightData);

    return ok ? FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE : FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
}

void FlacReader::metadata_callback(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data) {
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        // auto pp = metadata->data.stream_info;
        FlacReader* client = reinterpret_cast<FlacReader*>(client_data);
        client->onFormat(metadata->data.stream_info.total_samples,
                         metadata->data.stream_info.sample_rate,
                         metadata->data.stream_info.channels,
                         metadata->data.stream_info.bits_per_sample);
    }
}
void FlacReader::error_callback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data) {
    // TODO
    assert(false);
}
