
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"

#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"
#include "ThreadPriority.h"


/** The following block of constants control what
 * this plugin does. Change them and re-build
 */
static const int numLoadThreads = 3;
static const int drawMillisecondSleep = 100;
static bool doNormalBoost = true;


static std::atomic<bool> drawIsSleeping;

/**
 * Implementation class for BootyModule
 */
struct pModule : Module
{
    pModule();
    ~pModule();

    /**
     * Overrides of Module functions
     */
    void step() override;

    int stepsWhileDrawing=0;
private:
    typedef float T;
    std::vector< std::shared_ptr<ThreadClient> > threads;
    bool boosted = false;
};

class PServer : public ThreadServer
{
public:
    PServer(std::shared_ptr<ThreadSharedState> state) 
      : ThreadServer(state)
    {

    }
    virtual void threadFunction () override;

    ~PServer() {
        printf("dtor PServer\n");
    }
private:
    bool didRun = false;
    double dummy = 0;
};

 void PServer::threadFunction() 
 {
    printf("oh, no, got the thread func\n"); fflush(stdout);
    sharedState->serverRunning = true;
    for (bool done = false; !done; ) {
        if (sharedState->serverStopRequested.load()) {
            done = true;
        } else {
          // now kill a lot of time
            for (int i=0; i< 10000; ++i) {
                dummy += std::log(rand()) * std::sin(rand());
            }

        }
    }

    thread->detach();
    sharedState->serverRunning = false;
 }

pModule::pModule() : Module(0,0,0,0)
{
    for (int i=0; i<numLoadThreads; ++i) {
        printf("starting a thread\n");
        std::shared_ptr<ThreadSharedState> state = std::make_shared<ThreadSharedState>();
        std::unique_ptr<ThreadServer> server(new PServer(state));
        threads.push_back( 
            std::make_shared<ThreadClient>(
                state,
                std::move(server)));
    }
    
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
    //shifter.init();
}

pModule::~pModule()
{
    threads.resize(0);
}


void pModule::step()
{
    if (drawIsSleeping) {
        stepsWhileDrawing++;
    }
    
    if (doNormalBoost && !boosted) {
        boosted = true;
        ThreadPriority::boostNormal();
    }
}

////////////////////
// module widget
////////////////////

struct pWidget : ModuleWidget
{
    pWidget(pModule *);
    void draw(NVGcontext *vg) override
    {
        const pModule* pMod = static_cast<const pModule*>(module);
        std::stringstream s;
        s << pMod->stepsWhileDrawing;
        steps->text = s.str();
 
        ModuleWidget::draw(vg);
        if (drawMillisecondSleep) {
            drawIsSleeping = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(drawMillisecondSleep));
            drawIsSleeping = false;
        }
    }
    Label* steps;
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
pWidget::pWidget(pModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    Label* label=new Label();
    label->box.pos = Vec(10, 100);
    label->text = "SleepSteps";
    label->color = COLOR_BLACK;
    addChild(label);

    steps=new Label();
    steps->box.pos = Vec(10, 140);
    steps->text = "";
    steps->color = COLOR_BLACK;
    addChild(steps);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelPModule = Model::create<pModule, pWidget>("Squinky Labs",
    "squinkylabs-p",
    "p", EFFECT_TAG);

