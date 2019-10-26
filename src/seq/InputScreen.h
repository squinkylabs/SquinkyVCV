#pragma once

#include <functional>
#include <memory>

#include "rack.hpp"

#include <widget/FramebufferWidget.hpp>


class InputControl;
class MidiSequencer;
class InputScreen;
struct NVGcolor;

using InputControlPtr = std::shared_ptr<InputControl>;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;
using InputScreenPtr = std::shared_ptr<InputScreen>;

struct InputScreen : public ::rack::widget::OpaqueWidget
{
public:
   InputScreen(const ::rack::math::Vec& pos,
        const ::rack::math::Vec& size,
        MidiSequencerPtr seq,
        std::function<void(bool)> dismisser);
   ~InputScreen();

    
   /**
    * Execute the editing function.
    * Called after user accepts input.
    */
   virtual void execute() = 0;
   void draw(const Widget::DrawArgs &args) override;

   std::vector<float> getValues() const;

protected:
   MidiSequencerPtr sequencer;
   std::function<void(bool)> dismisser = nullptr;
   std::vector<InputControl*> inputControls;

   float getAbsPitchFromInput(int index);

   /**
    * Helpers for building up screens
    */
   void addPitchInput(const ::rack::math::Vec& pos, const std::string& label);
   ::rack::ui::Label* addLabel(const ::rack::math::Vec& v, const char* str, const NVGcolor& color);
   void addOkCancel();
   
   /**
    * Input layout style constants
    */
   static constexpr float firstControlRow = 170.f;
   static constexpr float controlRowSpacing = 30.f;
   static float controlRow(int index) { return firstControlRow + index * controlRowSpacing; }
   static constexpr float okCancelY = 260.f;
};


