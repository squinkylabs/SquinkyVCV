#pragma once

class Compressor2Module;

class VULabels : public widget::TransparentWidget {

public:
    void draw(const DrawArgs& args) override {
    }
   // Compressor2Module* module;
    VULabels() = delete;
    VULabels(int* stereo, int* labelMode) : isStereo_(stereo), labelMode_(labelMode) {
        box.size = Vec(125, 10);
    }

private:
    int* const isStereo_;
    int* const labelMode_;

};
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

    void draw(const DrawArgs& args) override {
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
};

