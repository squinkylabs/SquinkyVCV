
#include "Squinky.hpp"
#include "WidgetComposite.h"

#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"

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

private:
    typedef float T;

    std::vector< std::shared_ptr<ThreadClient> > threads;

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
    for (int i=0; i<2; ++i) {
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
    printf("about to ditch\n");
    threads.resize(0);
     printf("about to ditched\n");
}


void pModule::step()
{
 
}

////////////////////
// module widget
////////////////////

struct pWidget : ModuleWidget
{
    pWidget(pModule *);
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

