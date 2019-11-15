#pragma once

//#include "DiatonicUtils.h"
#include "Scale.h"

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
      const std::string& title,
      std::function<void(bool)> dismisser);
   ~InputScreen();


   /**
    * Execute the editing function.
    * Called after user accepts input.
    */
   virtual void execute() = 0;
   void draw(const Widget::DrawArgs &args) override;

   std::vector<float> getValues() const;
   float getValue(int index) const;
   bool getValueBool(int index) const;
   int getValueInt(int index) const;

   /**
    * Input layout style constants
    */
   static constexpr float firstControlRow = 70.f;
   static constexpr float controlRowSpacing = 30.f;
   static constexpr float centerColumn = 170;
   static constexpr float centerGutter = 10;
   
   static float controlRow(int index) { return firstControlRow + index * controlRowSpacing; }
   static constexpr float okCancelY = 260.f;

protected:
   MidiSequencerPtr sequencer;
   std::function<void(bool)> dismisser = nullptr;
   std::vector<InputControl*> inputControls;

   std::pair<int, Scale::Scales> getKeysig(int index);

   /**
    * Helpers for building up screens
    */
   void addPitchInput(
      const ::rack::math::Vec& pos, 
      const std::string& label, 
      std::function<void(void)>callback);
   void addPitchOffsetInput(
      const ::rack::math::Vec& pos,
      const std::string& label, 
      std::function<void(void)>callback);

   void addKeysigInput(const ::rack::math::Vec& pos, std::pair<int, Scale::Scales> keysig);
   void addOkCancel();
   void addTitle(const std::string& title);
   ::rack::ui::Label* addLabel(const ::rack::math::Vec& v, const char* str, const NVGcolor& color);
   void addChooser(const ::rack::math::Vec& v, const std::string& title, const std::vector<std::string>& choices);

   void addNumberChooserInt(const ::rack::math::Vec& v , const char* str, int nMin, int nMax);
   
   /**
    * gets keysig from index, saves it into Sequencer.
    */
   void saveKeysig(int index);
};


