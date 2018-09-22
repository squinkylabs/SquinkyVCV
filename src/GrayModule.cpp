
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

const float jackCol = 40;
const float ledCol = jackCol + 20;
const float vertSpace = 32;
const float firstBitY = 80;

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
        printf("added bits\n"); fflush(stdout);
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
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    addBits(module);

   
    addInput(createInputCentered<PJ301MPort>(
            Vec(100,20),
            module,
            Gray<WidgetComposite>::INPUT_CLOCK));

     addParam(createParamCentered<CKSS>(
        Vec(100, 100),
        module,
        Gray<WidgetComposite>::PARAM_CODE,
        0.0f, 1.0f, 0.0f));
    addLabel(Vec(85, 70), "Balanced");
    addLabel(Vec(85, 110), "Norm");
   

 
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

