#pragma once

class Compressor2Module;

class VULabels : public widget::TransparentWidget {
public:
    // Compressor2Module* module;
    VULabels() = delete;
    VULabels(int* stereo, int* labelMode, int* channel) : isStereo_(stereo), labelMode_(labelMode), channel_(channel) {
        box.size = Vec(125, 10);
        labels.resize(16);
    }
    void draw(const DrawArgs& args) override;

private:
    int* const isStereo_;
    int* const labelMode_;
    int* const channel_;
    const NVGcolor textColor = nvgRGB(0x70, 0x70, 0x70);
    const NVGcolor textHighlighColor = nvgRGB(0xff, 0xff, 0xff);
    std::vector<std::string> labels;
    int lastStereo = -1;
    int lastLabelMode = -1;
    //int lastChannel = -1;

    void updateLabels();
};

inline void VULabels::updateLabels() {
    if ((*isStereo_ < 0) || (*labelMode_ < 0)) {
      //  INFO("short 1");
        return;
    }
    if ((*isStereo_ == lastStereo) && (lastLabelMode == *labelMode_)) {
      //   INFO("short 2 %d,%d  %d,%d", *isStereo_, lastStereo, lastLabelMode, *labelMode_);
        return;
    }

    for (int i = 0; i < 8; ++i) {
        SqStream sq;
        sq.add(i + 1);
        std::string s = sq.str();
        labels[i] = s;
        INFO("set all text");
    }

    lastStereo = *isStereo_;
    lastLabelMode = *labelMode_;
}

inline void VULabels::draw(const DrawArgs& args) {
    updateLabels();
    int f = ::rack::appGet()->window->uiFont->handle;
    NVGcontext* vg = args.vg;

    nvgFontFaceId(vg, f);
    nvgFontSize(vg, 14);
    float y = 5;

    if (lastStereo > 0) {
        const float dx = 15.5;  // 16 too much
        for (int i = 0; i < 8; ++i) {
            float x = 4 + i * dx;
            nvgFillColor(vg, (*channel_ == (i+1)) ? textHighlighColor : textColor);
            nvgText(vg, x, y, labels[i].c_str(), nullptr);
        }
    }
}

// this control adapted from Fundamental VCA 16 channel level meter
// widget::TransparentWidget
class MultiVUMeter : public widget::TransparentWidget {
private:
    int* const isStereo_;
    int* const labelMode_;

public:
    Compressor2Module* module;
    MultiVUMeter() = delete;
    MultiVUMeter(int* stereo, int* labelMode) : isStereo_(stereo), labelMode_(labelMode) {
        box.size = Vec(125, 75);
    }
    void draw(const DrawArgs& args) override;
};

inline void MultiVUMeter::draw(const DrawArgs& args) {
    const int numberOfSegments = 25;
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
    nvgFillColor(args.vg, nvgRGB(0, 0, 0));
    nvgFill(args.vg);

    const Vec margin = Vec(1, 1);
    const Rect r = box.zeroPos().grow(margin.neg());
    const int channels = module ? module->getNumVUChannels() : 1;

    // Segment value
    const float value = 1;
    nvgBeginPath(args.vg);
    nvgRect(args.vg,
            r.pos.x,
            r.pos.y + r.size.y * (1 - value),
            r.size.x,
            r.size.y * value);
    nvgFillColor(args.vg, color::mult(color::WHITE, 0.33));
    nvgFill(args.vg);

    // Segment gain
    nvgBeginPath(args.vg);
    for (int c = 0; c < channels; c++) {
        // gain == 1...0
        const float gain = module ? module->getChannelGain(c) : 1.f;
        // db = 0.... -infi

        // let's do 1 db per segment
        const double dbMaxReduction = -numberOfSegments;
        const double db = std::max(AudioMath::db(gain), dbMaxReduction);
        const double y0 = r.pos.y;
        const double h = db * r.size.y / dbMaxReduction;

        if (h >= 0.005f) {
            nvgRect(args.vg,
                    r.pos.x + r.size.x * c / channels,
                    y0,
                    r.size.x / channels,
                    h);
        }
    }
    const NVGcolor blue = nvgRGB(48, 125, 238);
    nvgFillColor(args.vg, blue);
    nvgFill(args.vg);

    // Invisible separators
    nvgBeginPath(args.vg);
    for (int i = 1; i <= numberOfSegments; i++) {
        nvgRect(args.vg,
                r.pos.x - 1.0,
                r.pos.y + r.size.y * i / float(numberOfSegments),
                r.size.x + 2.0,
                1.0);
    }
    nvgFillColor(args.vg, color::BLACK);
    nvgFill(args.vg);
}
