
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "Gray.h"

/**
 */
struct GrayModule : Module
{
public:
    GrayModule();
    /**
     *
     *
     * Overrides of Module functions
     */
    void step() override;

    Gray<WidgetComposite> gray;
private:
};

GrayModule::GrayModule()
    : Module(gray.NUM_PARAMS,
    gray.NUM_INPUTS,
    gray.NUM_OUTPUTS,
    gray.NUM_LIGHTS),
    gray(this)
{
}

void GrayModule::step()
{
    gray.step();
}

////////////////////
// module widget
////////////////////

struct GrayWidget : ModuleWidget
{
    GrayWidget(GrayModule *);

    /**
     * Helper to add a text label to this widget
     */
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }


private:
    void addBits(GrayModule *module);

    GrayModule* const module;
};

const float jackCol = 100;
const float ledCol = 69;
const float vertSpace = 31;  // 31.4
const float firstBitY = 64;

inline void GrayWidget::addBits(GrayModule *module)
{
    printf("add bits\n"); fflush(stdout);
    for (int i=0; i<8; ++i) {
        const Vec v(jackCol, firstBitY + i * vertSpace);
        addOutput(createOutputCentered<PJ301MPort>(
            v,
            module,
            Gray<WidgetComposite>::OUTPUT_0 + i));
        #if 1
        addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
            Vec(ledCol, firstBitY + i * vertSpace - 6),
            module,
            Gray<WidgetComposite>::LIGHT_0+i));
        #else

        const Vec vl(ledCol, firstBitY + i * vertSpace);
        addChild(
            createLightCentered<GreenLight>(
                vl, 
                module,
                Gray<WidgetComposite>::LIGHT_0));
        #endif
    }
}

/**
 * Global coordinate contstants
 */
/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
GrayWidget::GrayWidget(GrayModule *module) :
    ModuleWidget(module),
    module(module)
{
    box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/gray.svg")));
        addChild(panel);
    }

    addBits(module);
    addInput(createInputCentered<PJ301MPort>(
            Vec(22, 339),
            module,
            Gray<WidgetComposite>::INPUT_CLOCK));

     addParam(createParamCentered<CKSS>(
        Vec(71,33),
        module,
        Gray<WidgetComposite>::PARAM_CODE,
        0.0f, 1.0f, 0.0f));
    addLabel(Vec(2, 27), "Balanced");
   // addLabel(Vec(85, 110), "Norm");

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(100, 339),
        module,
        Gray<WidgetComposite>::OUTPUT_MIXED));
   

 
    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 
}

Model *modelGrayModule = Model::create<GrayModule,
    GrayWidget>("Squinky Labs",
    "squinkylabs-gry",
    "Gray Code", EFFECT_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG);

