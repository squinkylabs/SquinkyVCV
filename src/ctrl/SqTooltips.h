
#pragma once
namespace SqTooltips
{

/**
 * out derivation of ParamQuantity
 *      overrides display string
 *      same size as ParamQuantity, so they can be switched at runtime
 */
class SQParamQuantity : public ParamQuantity {
public:
    /**
     * @param other is the ParamQuantity we are going to take over
     */ 
    SQParamQuantity( const ParamQuantity& other) {
        // I forget how this works. I think it copies the entire guts of the 
        // old param quantity into us, so we don't need to. but we still keep
        // our own vtable to override getDisplayValueString.
        ParamQuantity* base = this;
        *base = other;
    }
    std::string getDisplayValueString() override = 0;
};

class OnOffParamQuantity : public SQParamQuantity {
public:
    OnOffParamQuantity(const ParamQuantity& other) : SQParamQuantity(other) {}
    std::string getDisplayValueString() override {
        const bool b = getValue() > .5f;
        return b ? "on" : "off";
    }
};

/**
 * generic helper for substituting a custom ParamQuantity
 * for the default one.
 */
template <typename T>
static void changeParamQuantity(Module* module, int paramNumber) {
    auto orig = module->paramQuantities[paramNumber];
    auto p = new T(*orig);

    delete orig;
    module->paramQuantities[paramNumber] = p;
}


} 