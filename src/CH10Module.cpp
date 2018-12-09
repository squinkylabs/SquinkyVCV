
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

//#include "Blank.h"
#ifdef _CH10

#include "CH10.h"
#include "ctrl/ToggleButton.h"

/**
 */
struct CH10Module : Module
{
public:
    CH10Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    CH10<WidgetComposite> ch10;

private:

};

void CH10Module::onSampleRateChange()
{
}

CH10Module::CH10Module()
    : Module(CH10<WidgetComposite>::NUM_PARAMS,
    CH10<WidgetComposite>::NUM_INPUTS,
    CH10<WidgetComposite>::NUM_OUTPUTS,
    CH10<WidgetComposite>::NUM_LIGHTS),
    ch10(this)
{
    onSampleRateChange();
    ch10.init();
}

void CH10Module::step()
{
    ch10.step();
}

////////////////////
// module widget
////////////////////

struct CH10Widget : ModuleWidget
{
    CH10Widget(CH10Module *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void makeA(CH10Module *);
    void makeB(CH10Module *);
    void makeAB(CH10Module *);
    void addSwitch(float x, float y, int id);
};

const static float size = 28;
const static float col1 = 20;
const static float row1 = 300;

inline void CH10Widget::makeA(CH10Module *)
{
    for (int i=0; i<10; ++i) {
        const float x = col1;
        const float y = row1 - i* size;
        addSwitch(x, y, CH10<Widget>::A0_PARAM + i);
    }
}

inline void CH10Widget::makeB(CH10Module *)
{
    for (int i=0; i<10; ++i) {
        const float x = col1 + size * (i+1);
        const float y = row1 + size;
        addSwitch(x, y, CH10<Widget>::B0_PARAM + i);
    }
}

inline void CH10Widget::makeAB(CH10Module *)
{
    #if 0
    for (int i=0; i<10; ++i) {
        for (int j=0; j<10; ++j) {
            const float x = col1 + size * (i+1);
            const float y = row1 - (j) * size;
            addSwitch(x, y, CH10<Widget>::A0_PARAM + i);
        }
    }
    #endif
    for (int row=0; row<10; ++row) {
        for (int col=0; col<10; ++col) {
            float x = col1 + size * (col+1);
            float y = row1 - row * size;
            int id = CH10<Widget>::A0B0_PARAM +
                col + row * 10;
            addSwitch(x, y, id);
        }
    }

}

inline void CH10Widget::addSwitch(float x, float y, int id)
{ 
    ToggleButton* tog = ParamWidget::create<ToggleButton>(
        Vec(x, y),
        module,
        id,
        0.0f, 1, 0);

    tog->addSvg("res/BluePush_1.svg");
    tog->addSvg("res/BluePush_0.svg");
    addParam(tog);
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
CH10Widget::CH10Widget(CH10Module *module) : ModuleWidget(module)
{
    box.size = Vec(25 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/ch10_panel.svg")));
        addChild(panel);
    }

    makeA(module);
    makeB(module);
    makeAB(module);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


Model *modelCH10Module = Model::create<CH10Module,
    CH10Widget>("Squinky Labs",
    "squinkylabs-ch10",
    "-- ch10 --", RANDOM_TAG);
#endif

