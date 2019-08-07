#pragma once

#include <string>
#include <utility>

class PitchUtils
{
public:
    static constexpr float semitone = 1.f / 12.f;    // one semitone is a 1/12 volt
    static constexpr float octave = 1.f;
    static std::pair<int, int> cvToPitch(float cv);
    static int cvToSemitone(float cv);
    static int deltaCVToSemitone(float cv);
    static float pitchToCV(int octave, int semi);
    static bool isAccidental(float cv);
    static bool isC(float cv);
    static std::string pitch2str(float cv);
    static const char* semi2name(int);
};

inline const char* PitchUtils::semi2name(int semi)
{
    const char* ret = "-";
    switch (semi) {
        case 0:
            ret = "C";
            break;
        case 1:
            ret = "C#";
            break;
        case 2:
            ret = "D";
            break;
        case 3:
            ret = "D#";
            break;
        case 4:
            ret = "E";
            break;
        case 5:
            ret = "F";
            break;
        case 6:
            ret = "F#";
            break;
        case 7:
            ret = "G";
            break;
        case 8:
            ret = "G#";
            break;
        case 9:
            ret = "A";
            break;
        case 10:
            ret = "A#";
            break;
        case 11:
            ret = "B";
            break;
        default:
            assert(false);
    }
    return ret;
}

inline std::string PitchUtils::pitch2str(float cv)
{
    auto p = cvToPitch(cv);
    char buffer[256];
    const char* pitchName = semi2name(p.second);
    snprintf(buffer, 256, "%s:%d", pitchName, p.first);
    return buffer;
}

inline std::pair<int, int> PitchUtils::cvToPitch(float cv)
{
     // VCV 0 is C4
    int octave = int(std::floor(cv));
    float remainder = cv - octave;
    octave += 4;
    float s = remainder * 12;
    int semi = int(std::round(s));
    if (semi >= 12) {
        semi -= 12;
        octave++;
    }
    return std::pair<int, int>(octave, semi);
}

inline  int PitchUtils::cvToSemitone(float cv)
{
    auto p = cvToPitch(cv);
    return p.first * 12 + p.second;
}

inline  int PitchUtils::deltaCVToSemitone(float cv)
{
    auto p = cvToPitch(cv);
    return (p.first-4) * 12 + p.second;
}

inline float PitchUtils::pitchToCV(int octave, int semi)
{
    return float(octave - 4) + semi * semitone;
}

inline bool PitchUtils::isAccidental(float cv)
{
    int semi = cvToPitch(cv).second;
    bool ret = false;
    switch (semi) {
        case 1:
        case 3:
        case 6:
        case 8:
        case 10:
            ret = true;
            break;
    }
    return ret;
}


inline bool PitchUtils::isC(float cv)
{
    int semi = cvToPitch(cv).second;
    return semi == 0;
}
