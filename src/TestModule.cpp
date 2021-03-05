
#include <sstream>
#include <stdexcept>

#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _TESTM
#include "Blank.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Blank<WidgetComposite>;

/**
 */
struct TestModule : Module {
public:
    TestModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> blank;

private:
};

void TestModule::onSampleRateChange() {
}

TestModule::TestModule() {
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    blank = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);

    onSampleRateChange();
    blank->init();
}

void TestModule::process(const ProcessArgs& args) {
    blank->process(args);
}

////////////////////
// module widget
////////////////////

struct TestWidget : ModuleWidget {
    TestWidget(TestModule*);
    DECLARE_MANUAL("Blank Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK) {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void draw(const DrawArgs& args) override {
        INFO("TestModule::draw 66");
        ModuleWidget::draw(args);
        std::stringstream str;
        static int counter = 0;
        str << "draw ";
        str << counter;
        INFO(str.str().c_str());
        INFO("TestModule::draw 73");
        ++counter;
    #if 0
        try {
            std::invalid_argument ex("i am an exception");
            throw(ex);
        } catch (std::exception& e) {
            INFO("caught: %s", e.what());
        }

        const size_t size = 100000;
        int testArray[size];
        static bool init = false;
        if (!init) {
            srand(1234);
            init = true;
        }
        for (size_t i=0; i<size; ++i) {
            testArray[i] = rand();
        }
        for (size_t i=0; i<size; ++i) {
            testArray[i] += rand();
        }
        int sum = 0;
        for (size_t i=0; i<size; ++i) {
            sum += testArray[i];
        }
        INFO("sum = %d", sum);
#endif
    }
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

TestWidget::TestWidget(TestModule* module) {
    setModule(module);
    SqHelper::setPanel(this, "res/blank_panel.svg");

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model* modelTestModule = createModel<TestModule, TestWidget>("squinkylabs-testmodel");
#endif
