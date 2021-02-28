
#pragma once

#include "ctrl/SqTooltips.h"
#include "Compressor.h"

class LambdaQuantity : public SqTooltips::SQParamQuantity {
public:
    LambdaQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
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
    AttackQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
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

class ReleaseQuantity : public LambdaQuantity {
public:
    ReleaseQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = Compressor<WidgetComposite>::getSlowReleaseFunction();
        suffix = " mS";
    }
};

class ThresholdQuantity : public LambdaQuantity {
public:
    ThresholdQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = Compressor<WidgetComposite>::getSlowThresholdFunction();
        suffix = " V";
    }
};

class MakeupGainQuantity : public LambdaQuantity {
public:
    MakeupGainQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = [](double x) {
            return x;
        };
        suffix = " dB";
    }
};

class WetdryQuantity : public LambdaQuantity {
public:
    WetdryQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = [](double x) {
            return (x + 1) * 50;
        };
        suffix = " % wet";
    }
};

class RatiosQuantity : public SqTooltips::SQParamQuantity {
public:
    RatiosQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
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

class BypassQuantity :  public SqTooltips::SQParamQuantity {
public:
    BypassQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
    }
    std::string getDisplayValueString() override {
        auto value = getValue();
        return value < .5 ? "Bypassed" : "Normal";
    }
};
