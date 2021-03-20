
#include "FlacReader.h"
#include "SqLog.h"

#include <assert.h>
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


static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;

FlacReader::~FlacReader() {
	delete decoder;
}

/*
	FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE,
< The write was OK and decoding can continue. 

	FLAC__STREAM_DECODER_WRITE_STATUS_ABORT
	*/
FLAC__StreamDecoderWriteStatus FlacReader::write_callback(const FLAC__StreamDecoder* decoder, const FLAC__Frame* frame, const FLAC__int32* const buffer[], void* client_data) {
//	assert(false);
//	return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	SQINFO("write_callback");
	SQINFO("sample number = %lld", frame->header.number.sample_number);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}
void FlacReader::metadata_callback(const FLAC__StreamDecoder* decoder, const FLAC__StreamMetadata* metadata, void* client_data) {
	
	SQINFO("metadata callback");

	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		/* save for later */
		total_samples = metadata->data.stream_info.total_samples;
		sample_rate = metadata->data.stream_info.sample_rate;
		channels = metadata->data.stream_info.channels;
		bps = metadata->data.stream_info.bits_per_sample;

		SQINFO("sample rate    : %u Hz\n", sample_rate);
		SQINFO("channels       : %u\n", channels);
		SQINFO("bits per sample: %u\n", bps);
		SQINFO("total samples  : %lld", total_samples);
	}

}
void FlacReader::error_callback(const FLAC__StreamDecoder* decoder, FLAC__StreamDecoderErrorStatus status, void* client_data) {
	assert(false);
}
