#pragma once

/**
 * When a patch is asked to "play", it serves up one of these.
 */
class VoicePlayInfo {
public:
    bool valid = false;
    int sampleIndex = 0;
    bool needsTranspose = false;
    float transposeAmt = 1;

    bool canPlay() const {
        return valid && (sampleIndex > 0);
    }
};
using VoicePlayInfoPtr = std::shared_ptr<VoicePlayInfo>;


class ISamplerPlayback {
public:
    virtual ~ISamplerPlayback() = default;
    virtual void play(VoicePlayInfo&, int midiPitch, int midiVelocity) = 0;
};