
#include "SqWaveFile.h"

//#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <assert.h>

const char* wave_path = "C:\\Users\\bruce\\Documents\\VCV\\samples\\multi-note.wav";

SqWaveFile::SqWaveFile()
{

}

bool SqWaveFile::load(const std::string& path)
{
    drwav wav;
    if (!drwav_init_file(&wav, wave_path, NULL)) {
        printf("can't init dr_wav\n");
        return false;;
    }

    bool retValue = true;
    if (wav.channels != 1) {
        printf("can only open mono files\n");
        retValue = false;
    }

    if (retValue) {
        data.resize(wav.totalPCMFrameCount);
        assert(false);  // fix this
        //drwav_read_pcm_frames_f32__pcm(&wav, wav.totalPCMFrameCount, data.data());
    }

    drwav_uninit(&wav);
    return retValue;
}

 bool SqWaveFile::loadTest(TestFiles testFile)
 {
     std::string name;
     switch(testFile) {
         case TestFiles::MultiNote:
            name = "multi-note.wav";
            break;
        case TestFiles::SingleShortNote:
            name = "single-short-note.wav";
            break;
        default:
            assert(false);
     }
     return load(name);
 }
