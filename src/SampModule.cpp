
#include <sstream>

#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _SAMP
#include <osdialog.h>

#include "Samp.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Samp<WidgetComposite>;

/**
 */
struct SampModule : Module {
public:
    SampModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> samp;

    void setNewSamples(const std::string& s) {
        samp->setNewSamples(s);
    }

    void setSamplePath(const std::string& s) {
        samp->setSamplePath(s);
    }

private:
};

void SampModule::onSampleRateChange() {
}

SampModule::SampModule() {
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    samp = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);

    onSampleRateChange();
    samp->init();
}

void SampModule::process(const ProcessArgs& args) {
    samp->process(args);
}

////////////////////
// module widget
////////////////////

static const char* helpUrl = "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md";

struct SampWidget : ModuleWidget {
    SampWidget(SampModule* m);
    //  DECLARE_MANUAL("Samp Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md");
    void appendContextMenu(Menu* theMenu) override {
        ::rack::ui::MenuLabel* spacerLabel = new ::rack::ui::MenuLabel();
        theMenu->addChild(spacerLabel);
        ManualMenuItem* manual = new ManualMenuItem("Samp manual", helpUrl);
        theMenu->addChild(manual);

        {
            SqMenuItem* sfile = new SqMenuItem(
                []() { return false; },
                [this]() { this->loadSamplerFile(); });
            sfile->text = "Load Sample file";
            theMenu->addChild(sfile);
        }
        {
            SqMenuItem* spath = new SqMenuItem(
                []() { return false; },
                [this]() { this->getRootFolder(); });
            spath->text = "Set default sample path";
            theMenu->addChild(spath);
        }
    }

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK) {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void loadSamplerFile();
    void getRootFolder();
    void addJacks(SampModule* module, std::shared_ptr<IComposite> icomp);
    void setSamplePath(const std::string& s) {
        _module->setSamplePath(s);
        pathLabel->text = s;
    }

    SampModule* _module;
    Label* pathLabel = nullptr;
};

void SampWidget::loadSamplerFile() {
    static const char SMF_FILTERS[] = "Standard Sfz file (.sfz):sfz";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename;

    // std::string dir = _module->sequencer->context->settings()->getMidiFilePath();
    std::string dir = "";
    DEFER({
        osdialog_filters_free(filters);
    });

    char* pathC = osdialog_file(OSDIALOG_OPEN, dir.c_str(), filename.c_str(), filters);

    if (!pathC) {
        // Fail silently
        return;
    }
    DEFER({
        std::free(pathC);
    });
    printf("got %s\n", pathC);
    fflush(stdout);

    FATAL("finish file load");
    if (pathC) {
        _module->setNewSamples(pathC);
    }
}

void SampWidget::getRootFolder() {
    static const char SMF_FILTERS[] = "Standard Sfz file (.sfz):sfz";
    osdialog_filters* filters = osdialog_filters_parse(SMF_FILTERS);
    std::string filename;

    // std::string dir = _module->sequencer->context->settings()->getMidiFilePath();
    std::string dir = "";
    DEFER({
        osdialog_filters_free(filters);
    });

    char* pathC = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), filename.c_str(), filters);

    if (!pathC) {
        // Fail silently
        return;
    }
    DEFER({
        std::free(pathC);
    });
    SQINFO("got %s", pathC);
    this->setSamplePath(pathC);
}

void SampWidget::addJacks(SampModule* module, std::shared_ptr<IComposite> icomp) {
    float jacksY = 340;
    float jacksX = 15;
    float jacksDx = 40;
    float labelY = jacksY - 25;

    addLabel(
        Vec(jacksX + 0 * jacksDx - 5, labelY),
        "Out");
    addOutput(createOutput<PJ301MPort>(
        Vec(jacksX + 0 * jacksDx, jacksY),
        module,
        Comp::AUDIO_OUTPUT));

    addLabel(
        Vec(jacksX + 1 * jacksDx - 10, labelY),
        "V/Oct");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 1 * jacksDx, jacksY),
        module,
        Comp::PITCH_INPUT));

    addLabel(
        Vec(jacksX + 2 * jacksDx - 10, labelY),
        "Gate");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 2 * jacksDx, jacksY),
        module,
        Comp::GATE_INPUT));

    addLabel(
        Vec(jacksX + 3 * jacksDx - 6, labelY),
        "Vel");
    addInput(createInput<PJ301MPort>(
        Vec(jacksX + 3 * jacksDx, jacksY),
        module,
        Comp::VELOCITY_INPUT));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

SampWidget::SampWidget(SampModule* module) {
    setModule(module);
    _module = module;
    SqHelper::setPanel(this, "res/blank_panel.svg");

    addLabel(Vec(100, 50), "Sssssss");
    pathLabel = addLabel(Vec(50, 70), "");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model* modelSampModule = createModel<SampModule, SampWidget>("squinkylabs-samp");
#endif
