
#include "FlacReader.h"
#include "SqLog.h"
//#include "stream_decoder.h"

void FlacReader::read(const char* filePath) {
	if ((decoder = FLAC__stream_decoder_new()) == NULL) {
		SQWARN("ERROR: allocating flac decoder");
		fclose(fout);
		return;
	}
}