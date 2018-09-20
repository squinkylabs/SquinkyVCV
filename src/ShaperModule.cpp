
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "Shaper.h"

/**
 */
struct ShaperModule : Module
{
public:
    ShaperModule();
    /**
     *
     *
     * Overrides of Module functions
     */
    void step() override;

    Shaper<WidgetComposite> shaper;
private:
};

ShaperModule::ShaperModule()
    : Module(shaper.NUM_PARAMS,
    shaper.NUM_INPUTS,
    shaper.NUM_OUTPUTS,
    shaper.NUM_LIGHTS),
    shaper(this)
{
}

void ShaperModule::step()
{
    shaper.step();
}

////////////////////
// module widget
////////////////////

struct ShaperWidget : ModuleWidget
{
    ShaperWidget(ShaperModule *);

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

    void step() override;
private:
    Label* shapeLabel=nullptr;
    ParamWidget* shapeParam = nullptr;
    Shaper<WidgetComposite>::Shapes curShape = Shaper<WidgetComposite>::Shapes::Invalid;
    ShaperModule* const module;
    void addSelector(ShaperModule* module);
};

void ShaperWidget::step()
{
    ModuleWidget::step();
    const int iShape = (int) std::round(shapeParam->value);
    const Shaper<WidgetComposite>::Shapes shape = Shaper<WidgetComposite>::Shapes(iShape);
    if (shape != curShape) {
        curShape = shape;
        const char* shapeString = Shaper<WidgetComposite>::getString(shape);
        shapeLabel->text = shapeString;
    }
}

void ShaperWidget::addSelector(ShaperModule* module)
{
    const float x = 80;
    const float y = 100;
    auto p = createParamCentered<Rogan1PSBlue>(
        Vec(x, y),
        module, Shaper<WidgetComposite>::PARAM_SHAPE,
        0,
        float(Shaper<WidgetComposite>::Shapes::Invalid)-1,
        0);
    p->snap = true;
	p->smooth = false;
    addParam(p);
    shapeLabel = addLabel(Vec(x-30, y-40), "");
    shapeParam = p;
}



/**
 * Global coordinate constants
 */
/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
ShaperWidget::ShaperWidget(ShaperModule *module) :
    ModuleWidget(module),
    module(module)
{
    box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }
    addLabel(Vec(30, 20), "Wave Shaper");

    addSelector(module);

    addInput(createInputCentered<PJ301MPort>(
            Vec(40,340),
            module,
            Shaper<WidgetComposite>::INPUT_AUDIO));

    addOutput(createOutputCentered<PJ301MPort>(
            Vec(120,340),
            module,
            Shaper<WidgetComposite>::OUTPUT_AUDIO));

    
    const float labelDeltaY = 25;
    const float gainX = 40;
    const float offsetX = 95;
    const float labelDeltaX = -20;
    const float y = 170;
    const float symmetryX = 150;

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(gainX, y),
        module, Shaper<WidgetComposite>::PARAM_GAIN, -5, 5, 0));
    addLabel(Vec(gainX+labelDeltaX, y + labelDeltaY), "gain");

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(offsetX, y),
        module, Shaper<WidgetComposite>::PARAM_OFFSET, -5, 5, 0));
    addLabel(Vec(offsetX+labelDeltaX,  y + labelDeltaY), "offset");

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(symmetryX, y),
        module, Shaper<WidgetComposite>::PARAM_SYMMETRY, 0, 1, 0));
    addLabel(Vec(symmetryX+labelDeltaX,  y + labelDeltaY), "sym");

    const float deltaYTrim = 60;
    const float deltaYInput = 90;
    addParam(createParamCentered<Trimpot>(
        Vec(gainX, y + deltaYTrim),
        module, Shaper<WidgetComposite>::PARAM_GAIN_TRIM, -1, 1, 0));
    addParam(createParamCentered<Trimpot>(
        Vec(offsetX, y + deltaYTrim),
        module, Shaper<WidgetComposite>::PARAM_OFFSET_TRIM, -1, 1, 0));
    addInput(createInputCentered<PJ301MPort>(
            Vec(gainX,y + deltaYInput),
            module,
            Shaper<WidgetComposite>::INPUT_GAIN));
    addInput(createInputCentered<PJ301MPort>(
            Vec(offsetX,y + deltaYInput),
            module,
            Shaper<WidgetComposite>::INPUT_OFFSET));

    
    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 
}

Model *modelShaperModule = Model::create<ShaperModule,
    ShaperWidget>("Squinky Labs",
    "squinkylabs-shp",
    "shap", EFFECT_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG);

