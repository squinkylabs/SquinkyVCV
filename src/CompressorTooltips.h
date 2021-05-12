
#pragma once

#include "Compressor.h"
#include "Compressor2.h"
#include "SqStream.h"
#include "WidgetComposite.h"
#include "ctrl/SqTooltips.h"

class LambdaQuantity : public SqTooltips::SQParamQuantity {
public:
    LambdaQuantity(const ParamQuantity& other) : SqTooltips::SQParamQuantity(other) {
    }

    std::string getDisplayValueString() override {
        auto value = getValue();
        auto expValue = expFunction(value);
        SqStream str;
        str.precision(2);
        str.add(expValue);
        if (!suffix.empty()) {
            str.add(suffix);
        }
        return str.str();
    }

protected:
    std::function<double(double)> expFunction;
    std::string suffix;
};

class AttackQuantity : public LambdaQuantity {
public:
    AttackQuantity(const ParamQuantity& other) : LambdaQuantity(other) {
        // expFunction = Comp::getSlowAttackFunction();
        auto func = Compressor<WidgetComposite>::getSlowAttackFunction();
        expFunction = [func](double x) {
            auto y = func(x);
            if (y < .1) {
                y = 0;
            }
            return y;
        };
        suffix = " mS";
    }
};

class AttackQuantity2 : public rack::engine::ParamQuantity {
public:
    AttackQuantity2() : func(Compressor2<WidgetComposite>::getSlowAttackFunction()) {
    }
    
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

private:
    std::function<double(double)> const func;
};


class ReleaseQuantity : public LambdaQuantity {
public:
    ReleaseQuantity(const ParamQuantity& other) : LambdaQuantity(other) {
        expFunction = Compressor<WidgetComposite>::getSlowReleaseFunction();
        suffix = " mS";
    }
};

class ReleaseQuantity2 : public rack::engine::ParamQuantity {
public:
    ReleaseQuantity2() : func(Compressor2<WidgetComposite>::getSlowReleaseFunction()) {
    }
    
    std::string getDisplayValueString() override {
        auto value = getValue();
        auto mappedValue = func(value);
        SqStream str;
        str.precision(2);
        str.add(mappedValue);
       
        str.add(" mS");
        return str.str();
    }

private:
    std::function<double(double)> const func;
};

class ThresholdQuantity : public LambdaQuantity {
public:
    ThresholdQuantity(const ParamQuantity& other) : LambdaQuantity(other) {
        expFunction = Compressor<WidgetComposite>::getSlowThresholdFunction();
        suffix = " V";
    }
};


class ThresholdQuantity2 :  public rack::engine::ParamQuantity {
public:
    ThresholdQuantity2() : func(Compressor2<WidgetComposite>::getSlowThresholdFunction()) {
    }
    
    std::string getDisplayValueString() override {
        auto value = getValue();
        auto mappedValue = func(value);
        SqStream str;
        str.precision(2);
        str.add(mappedValue);
       
        str.add(" V");
        return str.str();
    }
private:
    std::function<double(double)> const func;
};

class MakeupGainQuantity : public LambdaQuantity {
public:
    MakeupGainQuantity(const ParamQuantity& other) : LambdaQuantity(other) {
        expFunction = [](double x) {
            return x;
        };
        suffix = " dB";
    }
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

class WetdryQuantity : public LambdaQuantity {
public:
    WetdryQuantity(const ParamQuantity& other) : LambdaQuantity(other) {
        expFunction = [](double x) {
            return (x + 1) * 50;
        };
        suffix = " % wet";
    }
};

class RatiosQuantity : public SqTooltips::SQParamQuantity {
public:
    RatiosQuantity(const ParamQuantity& other) : SqTooltips::SQParamQuantity(other) {
    }

    std::string getDisplayValueString() override {
        auto value = getValue();
        int index = value;
        std::string ratio = Compressor<WidgetComposite>::ratiosLong()[index];
        return ratio;
    }

protected:
    std::function<double(double)> expFunction;
    std::string suffix;
};

class BypassQuantity : public SqTooltips::SQParamQuantity {
public:
    BypassQuantity(const ParamQuantity& other) : SqTooltips::SQParamQuantity(other) {
    }
    std::string getDisplayValueString() override {
        auto value = getValue();
        return value < .5 ? "Bypassed" : "Normal";
    }
};
