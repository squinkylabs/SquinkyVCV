#pragma once

class Compressor2Module;

class VULabels : public widget::TransparentWidget {
public:
    // Compressor2Module* module;
    VULabels() = delete;
    VULabels(int* stereo, int* labelMode, int* channel, bool isFake) : isStereo_(stereo), labelMode_(labelMode), channel_(channel), isFake_(isFake) {
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
    const bool isFake_;

    void updateLabels();
};

inline void VULabels::updateLabels() {
    if (isFake_) {
         for (int i = 0; i < 8; ++i) {
            SqStream sq;
            sq.add(i+1);
            std::string s = sq.str();
            labels[i] = s;
        }
    }
    if ((*isStereo_ < 0) || (*labelMode_ < 0)) {
        INFO("short 1");
        return;
    }
    if ((*isStereo_ == lastStereo) && (lastLabelMode == *labelMode_)) {
        return;
    }

    // INFO("came thru %d, %d, %d", *isStereo_, *labelMode_, *channel_);

    if (*isStereo_ > 0) {
        for (int i = 0; i < 8; ++i) {
           // SqStream sq;
            std::string s = Comp2TextUtil::channelLabel(*labelMode_, i + 1);
            labels[i] = s;
        }
    } else {
        for (int i = 0; i < 16; ++i) {
            SqStream sq;
            sq.add(i + 1);
            std::string s = sq.str();
            labels[i] = s;
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

    if ((lastStereo > 0) || isFake_) {
        // ---------- Stereo -------------
        float y = 5;
        float fontSize = (lastLabelMode == 2) ? 11 : 12;
        nvgFontSize(vg, fontSize);
        const float dx = 15.5;  // 15.6 slightly too much
        for (int i = 0; i < 8; ++i) {
            //FO("draw lab stereo %d", i);
            bool twoDigits = ((lastLabelMode == 1) && (i > 0));
            float x = 5 + i * dx;
            if (twoDigits) {
                // INFO("two digits");
                x -= 3;  // 6  to0 much
            }
            if (lastLabelMode == 2) {  // for funny label mode
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
        const float dx = 7.65;  // 9 way too big 7.5 slightly low
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
    const NVGcolor lineColor = nvgRGB(0x40, 0x40, 0x40);

public:
    Compressor2Module* module;
    MultiVUMeter() = delete;
    MultiVUMeter(int* stereo, int* labelMode, int* channel) : isStereo_(stereo), labelMode_(labelMode), channel_(channel) {
        box.size = Vec(125, 75);
    }
    void draw(const DrawArgs& args) override;
    float getFakeGain(int channel);
    int getNumFakeChannels() {
        return 8;
    }
};

inline float MultiVUMeter::getFakeGain(int channel) {
    float ret = 1;
    switch(channel) {
        case 1:
            ret = .25f;
            break;
        case 2:
            ret = .6f;
            break;
        case 5:
            ret = .157f;
            break;
    }
    return ret;
}

inline void MultiVUMeter::draw(const DrawArgs& args) {
    nvgBeginPath(args.vg);
    nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(args.vg, nvgRGB(0, 0, 0));
    nvgFill(args.vg);

    const Vec margin = Vec(1, 1);
    const Rect r = box.zeroPos().grow(margin.neg());
    const int channels = module ? module->getNumVUChannels() : getNumFakeChannels();
    const double dbMaxReduction = -18;
    const double y0 = r.pos.y;
    const float barMargin = .5;
    const float barWidth = (r.size.x / channels) - 2 * barMargin;

    // draw the scale lines ---------------------------------------------------------
    {
        const int f = ::rack::appGet()->window->uiFont->handle;
        nvgFontFaceId(args.vg, f);
        nvgFontSize(args.vg, 11);
        nvgBeginPath(args.vg);
        nvgFillColor(args.vg, this->lineColor);

        const float y6 = y0 + r.size.y / 3;
        const float y12 = y0 + 2 * r.size.y / 3;
        const float x0 = r.pos.x;
        const float w = r.size.x;
       // const float xLabel = x0 + (r.size.x / 2) - 10;

        nvgRect(args.vg, x0, y6, w, 1);
        //  nvgText(args.vg, xLabel, y6,"-6 dB.", nullptr);

        nvgRect(args.vg, x0, y12, w, 1);

        nvgFill(args.vg);
    }

    //  draw the bars themselves
    nvgBeginPath(args.vg);
    for (int c = 0; c < channels; c++) {
        // gain == 1...0
        const float gain = module ? module->getChannelGain(c) : getFakeGain(c);
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

    const NVGcolor blue = nvgRGB(48, 125, 238);
    nvgFillColor(args.vg, blue);
    nvgFill(args.vg);

    // Re-draw the active channel in a different color so we see it. ----------
    {
        nvgBeginPath(args.vg);
        const NVGcolor ltBlue = nvgRGB(48 + 50, 125 + 50, 255);
        int c = *channel_ - 1;
        if (!module) {
            c = 1;
        }
        if (c >= 0) {
            const float gain = module ? module->getChannelGain(c) : getFakeGain(c);
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
