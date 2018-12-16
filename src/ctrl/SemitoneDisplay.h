
#pragma once

#include "rack.hpp"

#include <vector>
#include <string>
#include <sstream>

class SemitoneDisplay
{
public:
    SemitoneDisplay(rack::Module* module, int parameterId) : 
        module(module),
        parameterId(parameterId)
    {

    }
    void step();
   

    /**
     * pass in the label component that will be displaying semitones
     */
    void setLabel(rack::Label* l)
    {
        label = l;
        x = l->box.pos.x;
    }
private:
    rack::Module* module;
    rack::Label* label=nullptr;
    float x=0;
    const int parameterId;
    int lastSemi = 100;

const std::vector<std::string> names = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "D"
    };
};

inline void SemitoneDisplay::step()
{
    const int semi = (int) std::round(module->params[parameterId].value);
    if (semi != lastSemi) {
        lastSemi = semi;
        std::stringstream so;
        int semi = lastSemi;
        if (semi < 0) {
            semi += 12;
        }

        so << "Semi: " << names[semi];
        label->text = so.str();
        //label->box.pos.x = x + semi_offsets[semi];
    }
}