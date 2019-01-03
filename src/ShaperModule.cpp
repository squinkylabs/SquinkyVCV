
#include "Squinky.hpp"
#include "ctrl/ToggleButton.h"
#include "WidgetComposite.h"
#include "Shaper.h"

/**
 */
struct ShaperModule : Module
{
public:
    ShaperModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

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

void ShaperModule::onSampleRateChange()
{
    shaper.onSampleRateChange();
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
        label->fontSize = 16;
        addChild(label);
        return label;
    }

    void step() override;
private:
    Label* shapeLabel=nullptr;
    Label* shapeLabel2=nullptr;
    ParamWidget* shapeParam = nullptr;
    //ParamWidget* oversampleParam = nullptr;
    Shaper<WidgetComposite>::Shapes curShape = Shaper<WidgetComposite>::Shapes::Invalid;
    void addSelector(ShaperModule* module);
};

void ShaperWidget::step()
{
    ModuleWidget::step();
    const int iShape = (int) std::round(shapeParam->value);
    const Shaper<WidgetComposite>::Shapes shape = Shaper<WidgetComposite>::Shapes(iShape);
    if (shape != curShape) {
        curShape = shape;
        std::string shapeString = Shaper<WidgetComposite>::getString(shape);
        if (shapeString.length() > 8) {
            auto pos = shapeString.find(' ');
            if (pos != std::string::npos) {
                shapeLabel->text = shapeString.substr(0, pos);
                shapeLabel2->text = shapeString.substr(pos+1);
            } else {
                shapeLabel->text = "too";
                shapeLabel2->text = "big";
            }
        } else {
            shapeLabel->text = shapeString;
            shapeLabel2->text = "";
        }
    }
}

void ShaperWidget::addSelector(ShaperModule* module)
{
    const float x = 37;
    const float y = 80;
    auto p = createParamCentered<Rogan3PSBlue>(
        Vec(x, y),
        module, Shaper<WidgetComposite>::PARAM_SHAPE,
        0,
        float(Shaper<WidgetComposite>::Shapes::Invalid)-1,
        0);
    p->snap = true;
	p->smooth = false;
    addParam(p);
    shapeLabel = addLabel(Vec(70, 60), "");
    shapeLabel2 = addLabel(Vec(70, 60+18), "");
    shapeParam = p;
    shapeLabel->fontSize = 18;
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
ShaperWidget::ShaperWidget(ShaperModule *module) :
    ModuleWidget(module)
{
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/shaper.svg")));
        addChild(panel);
    }

    addSelector(module);

    const float gainX = 35;
    const float offsetX = 108;
    const float gainY = 232;
    const float offsetY = 147;

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(gainX, gainY),
        module, Shaper<WidgetComposite>::PARAM_GAIN, -5, 5, 0));
    addLabel(Vec(8, 191), "Gain");

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(offsetX, offsetY),
        module, Shaper<WidgetComposite>::PARAM_OFFSET, -5, 5, 0));
    addLabel(Vec(34, 135), "Offset");

    addParam(createParamCentered<Trimpot>(
        Vec(56, 275),
        module, Shaper<WidgetComposite>::PARAM_GAIN_TRIM, -1, 1, 0));
    addParam(createParamCentered<Trimpot>(
        Vec(81, 199),
        module, Shaper<WidgetComposite>::PARAM_OFFSET_TRIM, -1, 1, 0));

    const float jackY = 327;
    const float jackLabelY = jackY - 29;
    addInput(createInputCentered<PJ301MPort>(
            Vec(30,jackY),
            module,
            Shaper<WidgetComposite>::INPUT_AUDIO));
    addLabel(Vec(18, jackLabelY), "In")->fontSize = 12;

    addOutput(createOutputCentered<PJ301MPort>(
            Vec(127,jackY),
            module,
            Shaper<WidgetComposite>::OUTPUT_AUDIO));
    addLabel(Vec(109+1, jackLabelY+1), "Out", COLOR_WHITE)->fontSize = 12;

    addInput(createInputCentered<PJ301MPort>(
            Vec(62, jackY),
            module,
            Shaper<WidgetComposite>::INPUT_GAIN));
    addInput(createInputCentered<PJ301MPort>(
            Vec(95,jackY),
            module,
            Shaper<WidgetComposite>::INPUT_OFFSET));

    ToggleButton* tog = ParamWidget::create<ToggleButton>(
        Vec(125-16, 265 - 13),
        module,
        Shaper<WidgetComposite>::PARAM_ACDC,
        0.0f, 1, 0);
    tog->addSvg("res/AC.svg");
    tog->addSvg("res/DC.svg");
    addParam(tog);

    tog = ParamWidget::create<ToggleButton>(
        Vec(123-20, 226 - 13),
        module,
        Shaper<WidgetComposite>::PARAM_OVERSAMPLE,
        0.0f, 2, 0);
    tog->addSvg("res/16x-03.svg");
    tog->addSvg("res/16x-02.svg");
    tog->addSvg("res/16x-01.svg");
    addParam(tog);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 
}

Model *modelShaperModule = Model::create<ShaperModule,
    ShaperWidget>("Squinky Labs",
    "squinkylabs-shp",
    "Shaper: Precision Wave Shaper", WAVESHAPER_TAG, DISTORTION_TAG);

