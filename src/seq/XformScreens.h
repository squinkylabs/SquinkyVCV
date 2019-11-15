#pragma once

#include "InputScreen.h"

class XformInvert : public InputScreen
{
public:
    XformInvert(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void(bool)> _dismisser);
    void execute() override;
};

class XformTranspose : public InputScreen
{
public:
    XformTranspose(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void(bool)> _dismisser);
    void execute() override;
};

class XformReversePitch : public InputScreen
{
public:
    XformReversePitch(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void(bool)> _dismisser);
    void execute() override;   
};

class XformChopNotes : public InputScreen
{
public:
    XformChopNotes(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void(bool)> _dismisser);
    void execute() override;   
};

class XFormQuantizePitch : public InputScreen
{
public:
    XFormQuantizePitch(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void(bool)> _dismisser);
    void execute() override;   
};
