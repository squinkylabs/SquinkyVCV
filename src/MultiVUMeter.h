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
        INFO("short 1");
        return;
    }
    if ((*isStereo_ == lastStereo) && (lastLabelMode == *labelMode_)) {
        //  INFO("short 2 %d,%d  %d,%d", *isStereo_, lastStereo, lastLabelMode, *labelMode_);
        return;
    }

    // INFO("came thru %d, %d, %d", *isStereo_, *labelMode_, *channel_);

    if (*isStereo_ > 0) {
        for (int i = 0; i < 8; ++i) {
            SqStream sq;
            std::string s = Comp2TextUtil::channelLabel(*labelMode_, i + 1);
            // INFO("set label[%d] to %s", i, s.c_str());
            labels[i] = s;
            //  INFO("set all text");
        }
    } else {
        for (int i = 0; i < 16; ++i) {
            SqStream sq;
            sq.add(i + 1);
            std::string s = sq.str();
            labels[i] = s;
            //INFO("set mlabel[%d] to %s", i, s.c_str());
        }
    }

    lastStereo = *isStereo_;
    lastLabelMode = *labelMode_;
}

inline void VULabels::draw(const DrawArgs& args) {
    updateLabels();
    int f = ::rack::appGet()->window->uiFont->handle;
    NVGcontext* vg = args.vg;

    nvgFontFaceId(vg, f);

    if (lastStereo > 0) {
        // ---------- Stereo -------------
        float y = 5;
        float fontSize = (lastLabelMode == 2) ? 11 : 12;
        nvgFontSize(vg, fontSize);
        const float dx = 15.5;  // 15.6 slightly too much
        for (int i = 0; i < 8; ++i) {
            bool twoDigits = ((lastLabelMode == 1) && (i > 0));
            float x = 5 + i * dx;
            if (twoDigits) {
               // INFO("two digits");
                x -= 3;    // 6  to0 much
            }
            if (lastLabelMode == 2) { // for funny label mode
               // INFO("funny label");
                x -= 3;
            }
            nvgFillColor(vg, (*channel_ == (i + 1)) ? textHighlighColor : textColor);
            nvgText(vg, x, y, labels[i].c_str(), nullptr);
        }
    } else {
        // ------------ Mono -----------
        float y = 2.5;
        nvgFontSize(vg, 11);
        const float dx = 7.65;         // 9 way too big 7.5 slightly low
        for (int i = 0; i < 16; ++i) {
            switch (i) {
                case 0:
                case 4:
                case 8:
                case 12: {
                    float x = 2 + i * dx;
                    const bool twoDigits = (i > 8);
                    if (twoDigits) {
                        x -= 2;  // move two digits to center
                    }
                    nvgFillColor(vg, (*channel_ == (i + 1)) ? textHighlighColor : textColor);
                    nvgText(vg, x, y, labels[i].c_str(), nullptr);
                } break;
            }
        }
    }
}

//*******************************************************************************************************
// this control adapted from Fundamental VCA 16 channel level meter
// widget::TransparentWidget
class MultiVUMeter : public widget::TransparentWidget {
private:
    int* const isStereo_;
    int* const labelMode_;
    int* const channel_;

public:
    Compressor2Module* module;
    MultiVUMeter() = delete;
    MultiVUMeter(int* stereo, int* labelMode, int* channel) : isStereo_(stereo), labelMode_(labelMode), channel_(channel) {
        box.size = Vec(125, 75);
    }
    void draw(const DrawArgs& args) override;
};

inline void MultiVUMeter::draw(const DrawArgs& args) {
  //  const int numberOfSegments = 25;
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.0);
    nvgFillColor(args.vg, nvgRGB(0, 0, 0));
    nvgFill(args.vg);

    const Vec margin = Vec(1, 1);
    const Rect r = box.zeroPos().grow(margin.neg());
    const int channels = module ? module->getNumVUChannels() : 1;

    // Segment gain
   // const double dbMaxReduction = -numberOfSegments;
    const double dbMaxReduction = -18;
    const double y0 = r.pos.y;

    const float barMargin = .5;

    const float barWidth = (r.size.x / channels) - 2 * barMargin;
    //  INFO("channels = %d", channels);
    nvgBeginPath(args.vg);
    for (int c = 0; c < channels; c++) {
        // gain == 1...0
        const float gain = module ? module->getChannelGain(c) : 1.f;
        // db = 0.... -infi

        // let's do 1 db per segment
        const double db = std::max(AudioMath::db(gain), dbMaxReduction);
        const double h = db * r.size.y / dbMaxReduction;

        if (h >= 0.005f) {
            //INFO("got level at c=%d", c);
            float x = r.pos.x + r.size.x * c / channels;
            x += barMargin;
            nvgRect(args.vg,
                    x,
                    y0,
                    barWidth,
                    h);
        }
    }

    //    const auto color = (atten >= attenThisSegment) ? UIPrefs::VU_ACTIVE_COLOR : UIPrefs::VU_INACTIVE_COLOR;
    const NVGcolor blue = nvgRGB(48, 125, 238);
    nvgFillColor(args.vg, blue);
    nvgFill(args.vg);

    // now the active channel
    {
        nvgBeginPath(args.vg);
        const NVGcolor ltBlue = nvgRGB(48 + 50, 125 + 50, 255);
        const int c = *channel_ - 1;
        if (c >= 0) {
            const float gain = module ? module->getChannelGain(c) : 1.f;
            const double db = std::max(AudioMath::db(gain), dbMaxReduction);

            const double h = db * r.size.y / dbMaxReduction;

            float x = r.pos.x + r.size.x * c / channels;
            x += barMargin;

            if (h >= 0.005f) {
                nvgRect(args.vg,
                        x,
                        y0,
                        barWidth,
                        h);
            }
        }
        nvgFillColor(args.vg, ltBlue);
        nvgFill(args.vg);
    }
}
