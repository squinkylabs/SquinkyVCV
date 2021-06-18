
#pragma once

#include "Compressor.h"
#include "Compressor2.h"
#include "SqStream.h"
#include "WidgetComposite.h"

class AttackQuantity2 : public rack::engine::ParamQuantity {
public:

#ifdef _CMP_SCHEMA2
   AttackQuantity2() : func(Compressor2<WidgetComposite>::getSlowAttackFunction_2()),
                        antiFunc(Compressor2<WidgetComposite>::getSlowAntiAttackFunction_2()) {
    }
#else
    AttackQuantity2() : func(Compressor2<WidgetComposite>::getSlowAttackFunction_1()),
                        antiFunc(Compressor2<WidgetComposite>::getSlowAntiAttackFunction_1()) {
    }
#endif

    std::string getDisplayValueString() override {
        auto value = getValue();
        auto mappedValue = func(value);
        if (mappedValue < .1) {
            mappedValue = 0;
        }
        SqStream str;
        str.precision(2);
        str.add(mappedValue);

        str.add(" mS");
        return str.str();
    }
    void setDisplayValueString(std::string s) override {

        float val = ::atof(s.c_str());
        if (val >= 0 && val <= 200) {
            val = antiFunc(val);
            ParamQuantity::setValue(val);
        }
    }

private:
    std::function<double(double)> const func;
    std::function<double(double)> const antiFunc;
};

class ReleaseQuantity2 : public rack::engine::ParamQuantity {
public:
#ifdef _CMP_SCHEMA2
 ReleaseQuantity2() : func(Compressor2<WidgetComposite>::getSlowReleaseFunction_2()),
                         antiFunc(Compressor2<WidgetComposite>::getSlowAntiReleaseFunction_2()) {
    }
#else
    ReleaseQuantity2() : func(Compressor2<WidgetComposite>::getSlowReleaseFunction_1()),
                         antiFunc(Compressor2<WidgetComposite>::getSlowAntiReleaseFunction_1()) {
    }
#endif

    std::string getDisplayValueString() override {
        auto value = getValue();
        auto mappedValue = func(value);
        SqStream str;
        str.precision(2);
        str.add(mappedValue);

        str.add(" mS");
        return str.str();
    }
    void setDisplayValueString(std::string s) override {
        float val = ::atof(s.c_str());
        if (val >= 0 && val <= 2000) {
            auto mappedValue = antiFunc(val);
            ParamQuantity::setValue(mappedValue);
        }
    }

private:
    std::function<double(double)> const func;
    std::function<double(double)> const antiFunc;
};

class ThresholdQuantity2 : public rack::engine::ParamQuantity {
public:
    ThresholdQuantity2() : func(Compressor2<WidgetComposite>::getSlowThresholdFunction()),
                           antiFunc(Compressor2<WidgetComposite>::getSlowAntiThresholdFunction()) {
    }

    std::string getDisplayValueString() override {
        const auto value = getValue();
        const auto mappedValue = func(value);
        // -20 because VCV 0 dB is 10V, but for AudioMath it's 1V
        const auto db = -20 + AudioMath::db(mappedValue);
        SqStream str;
        str.precision(2);
        str.add(db);
        str.add(" dB");
        return str.str();
    }
    void setDisplayValueString(std::string s) override {
        float val = ::atof(s.c_str());
        if (val <= 0 && val > -200) {
            val += 20;
            val = AudioMath::gainFromDb(val);
            val = antiFunc(val);
            ParamQuantity::setValue(val);
        }
    }

private:
    std::function<double(double)> const func;
    std::function<double(double)> const antiFunc;
};

class MakeupGainQuantity2 : public rack::engine::ParamQuantity {
public:
    std::string getDisplayValueString() override {
        auto mappedValue = getValue();
        SqStream str;
        str.precision(2);
        str.add(mappedValue);
        str.add(" dB");
        return str.str();
    }
};

class WetdryQuantity2 : public rack::engine::ParamQuantity {
public:
    std::string getDisplayValueString() override {
        auto value = getValue();
        auto mappedValue = (value + 1) * 50;
        SqStream str;
        str.precision(2);
        str.add(mappedValue);
        str.add(" % wet");
        return str.str();
    }

    void setDisplayValueString(std::string s) override {
        float val = ::atof(s.c_str());
        if (val >= 0 && val <= 100) {
            val = val / 50.f;  //0..2
            val -= 1;
            ParamQuantity::setValue(val);
        }
    }
};

class RatiosQuantity2 : public rack::engine::ParamQuantity {
public:
    std::string getDisplayValueString() override {
        auto value = getValue();

        int index = int(std::round(value));
        std::string ratio = Compressor<WidgetComposite>::ratiosLong()[index];
        return ratio;
    }
};

class BypassQuantity2 : public rack::engine::ParamQuantity {
public:
    std::string getDisplayValueString() override {
        auto value = getValue();
        return value < .5 ? "Bypassed" : "Normal";
    }
};

class BypassQuantityComp2 : public rack::engine::ParamQuantity {
public:
    std::string getDisplayValueString() override {
        auto value = getValue();
        return value < .5 ? "Bypassed" : "Engaged";
    }
};

class SideChainQuantity2 : public rack::engine::ParamQuantity {
public:
    std::string getDisplayValueString() override {
        auto value = getValue();
        return value < .5 ? "Inactive" : "Engaged";
    }
};