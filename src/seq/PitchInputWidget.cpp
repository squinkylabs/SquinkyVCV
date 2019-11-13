#include "InputScreen.h"
#include "PitchInputWidget.h"
#include "UIPrefs.h"

using Vec = ::rack::math::Vec;
//using Button = ::rack::ui::Button;
using Widget = ::rack::widget::Widget;
using Label = ::rack::ui::Label;

PitchInputWidget::PitchInputWidget(
    const ::rack::math::Vec& pos, 
    const ::rack::math::Vec& siz,
    const std::string& label, 
    bool relative) : relative(relative)
{

    box.pos = pos;
    // TODO: we should set our own height
    box.size = siz;

    int row = 0;

    addMainLabel(label, Vec(InputScreen::centerColumn, row * InputScreen::controlRowSpacing));
    addOctaveControl(Vec(InputScreen::centerColumn, row * InputScreen::controlRowSpacing));

    auto semiPos = Vec(InputScreen::centerColumn + 80, row * InputScreen::controlRowSpacing);
    addChromaticSemisControl(semiPos);
    addScaleDegreesControl(semiPos);

    ++row;
    addScaleRelativeControl(Vec(InputScreen::centerColumn, row * InputScreen::controlRowSpacing));
}

void PitchInputWidget::setChromatic(bool mode)
{
    if (mode == chromatic) {
        return;
    }

    chromatic = mode;
    if (chromatic) {
        this->removeChild(scaleDegreesInput);
        this->addChild(chromaticPitchInput);
    } else {
        this->removeChild(chromaticPitchInput);
        this->addChild(scaleDegreesInput);
    }


    if (chromaticCb) {
        chromaticCb();
    }
}


void PitchInputWidget::addMainLabel(const std::string& labelText, const ::rack::math::Vec& pos)
{
    auto labelCtrl = addLabel(Vec(0, pos.y), labelText.c_str());
    DEBUG("adding label at %.2f,%.2f", labelCtrl->box.pos.x, labelCtrl->box.pos.y);

    labelCtrl->box.size.x = pos.x - 10;
    DEBUG(" label text = %s, size=%.2f,%.2f", labelText.c_str(), labelCtrl->box.size.x, labelCtrl->box.size.y); 
    labelCtrl->alignment = Label::RIGHT_ALIGNMENT;
}

static std::vector<std::string> octavesRel = {
    "+7 oct", "+6 oct", "+5 oct",
    "+4 oct", "+3 oct", "+2 oct", "+1 oct",
    "+0 oct",
    "-1 oct", "-2 oct", "-3 oct", "-4 oct",
    "-5 oct", "-6 oct", "-7 oct"
};

static std::vector<std::string> semisRel = {
    "+12 semi", "+11 semi", "+10 semi","+9 semi",
    "+8 semi", "+7 semi", "+6 semi","+5 semi",
    "+4 semi", "+3 semi", "+2 semi","+1 semi",
     "+0 semi", 
     "-1 semi", "-2 semi", "-3 semi","-4 semi",
     "-5 semi", "-6 semi", "-7 semi","-8 semi",
     "-9 semi","-10 semi","-11 semi", "-12 semi"
};

static std::vector<std::string> scaleDegreesRel = {
    "+7 steps", "+6 steps", "+5 steps",
    "+4 steps", "+3 steps", "+2 steps","+1 step",
     "+0 steps", 
     "-1 step", "-2 steps", "-3 steps","-4 steps",
     "-5 steps", "-6 steps", "-7 steps"
};

static std::vector<std::string> semis = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static std::vector<std::string> octaves = {
    "7", "6", "5", "4", "3", "2", "1", "0", "-1", "-2", "-3"
};

static std::vector<std::string> scaleDegrees = {
    "step 7", "step 6", "step 5",
    "step 4", "step 3", "step 2","step 1",
     "root", 
     "step -1", "step -2", "step -3","step -4",
     "step -5", "step -6", "step -7"
};


void PitchInputWidget::addOctaveControl(const ::rack::math::Vec& pos)
{
    DEBUG("adding octaves at %.2f,%.2f", pos.x, pos.y);
    std::vector<std::string>& oct = relative ? octavesRel : octaves;
    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels(oct);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(pos);
    pop->text = oct[7];
    this->addChild(pop);
    octaveInput = pop;

    //inputControls.push_back(pop);
}

void PitchInputWidget::addChromaticSemisControl(const ::rack::math::Vec& pos)
{
 // DEBUG("adding semis, rel = %d")
    std::vector<std::string>& sem = relative ? semisRel : semis;
    auto pop = new InputPopupMenuParamWidget();
    pop->setLabels(sem);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(pos);

    const int initIndex = relative ? 12 : 0;
    pop->text = sem[0];
    pop->setValue(initIndex);
    if (chromatic) {
        // TODO: other one will leak
        this->addChild(pop);
    }
    chromaticPitchInput = pop;
}

void PitchInputWidget::addScaleDegreesControl(const ::rack::math::Vec& pos)
{
    auto pop = new InputPopupMenuParamWidget();
    std::vector<std::string>& deg = relative ? scaleDegreesRel : scaleDegrees; 
    pop->setLabels(deg);
    pop->box.size.x = 76;    // width
    pop->box.size.y = 22;     // should set auto like button does
    pop->setPosition(pos);
    pop->text = deg[7];
    if (!chromatic) {
        this->addChild(pop);  
    }
    scaleDegreesInput = pop;
}

void PitchInputWidget::addScaleRelativeControl(const ::rack::math::Vec& pos)
{
    auto check = new CheckBox();
    check->box.pos = pos;
    check->box.size = Vec(17, 17);
    this->addChild(check);

    auto l = addLabel(Vec(0, pos.y), "Relative to scale");
    l->box.size.x = InputScreen::centerColumn - InputScreen::centerGutter;
    l->alignment = Label::RIGHT_ALIGNMENT;  
    check->setCallback([this, check]() {
        // TODO: also call back to host so can flip keysig on and off
        DEBUG("in scale relative callback. must flip checkValue = %.2f\n", check->getValue());
        this->setChromatic(check->getValue() < .5f);
    });
    DEBUG("add check, value =  %.2f\n", check->getValue());
    this->keepInScale = check;
}

Label* PitchInputWidget::addLabel(const Vec& v, const char* str, const NVGcolor& color)
{
    Label* label = new Label();
    label->box.pos = v;
    label->text = str;
    label->color = color;
    this->addChild(label);
    return label;
}

bool PitchInputWidget::isChromaticMode() const
{
    return chromatic;
}

int PitchInputWidget::transposeOctaves() const
{
    const int middleOctaveIndex = 7;
    assert(octavesRel[middleOctaveIndex] == "+0 oct");
    int octaveOffset = middleOctaveIndex - int(std::round(octaveInput->getValue()));
    return octaveOffset;
}
int PitchInputWidget::transposeDegrees() const
{
    const int middleDegreeIndex = 7;
    assert(scaleDegreesRel[middleDegreeIndex] == "+0 steps");
    int degreesOffset = middleDegreeIndex - int(std::round(scaleDegreesInput->getValue()));
    return degreesOffset;
}

int PitchInputWidget::transposeSemis() const
{
    const int middleSemiIndex = 12;
    assert(semisRel[middleSemiIndex] == "+0 semi");
    int semiOffset = middleSemiIndex - int(std::round(chromaticPitchInput->getValue()));
    return semiOffset;
}

int PitchInputWidget::absoluteSemis() const
{
  //  const int middleSemiIndex = 12;
  //  assert(semis[middleSemiIndex] == "+0 semi");
    const int absSemis = int(std::round(chromaticPitchInput->getValue()));
    DEBUG("returning abs semis = %d", absSemis);
    return absSemis;
}

int PitchInputWidget::absoluteDegrees() const
{
    const int middleDegreeIndex = 7;
    assert(scaleDegrees[middleDegreeIndex] == "root");
    const int absDegrees = middleDegreeIndex - int(std::round(scaleDegreesInput->getValue()));
    DEBUG("returning abs degress %d", absDegrees);
    return absDegrees;
}
int PitchInputWidget::absoluteOctaves() const
{
    const int middleOctaveIndex = 7;
    assert(octavesRel[middleOctaveIndex] == "+0 oct");
    const int absOctave = middleOctaveIndex - int(std::round(octaveInput->getValue()));
    return absOctave;
}

float PitchInputWidget::getValue() const
{
    return keepInScale->getValue();
}

void PitchInputWidget::setValue(float) 
{

}

void PitchInputWidget::enable(bool enabled) 
{

}


void PitchInputWidget::setCallback(std::function<void(void)> cb) 
{
    chromaticCb = cb;
}

  