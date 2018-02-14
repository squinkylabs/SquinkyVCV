
#include "Squinky.hpp"
#include "SinOscillator.h"
#include "BiquadParams.h"
#include "BiquadFilter.h"
#include "BiquadState.h"
#include "HilbertFilterDesigner.h"

/**
 * Implementation class for BootyWidget
 */
struct BootyModule : Module {
    enum ParamIds {
		PITCH_PARAM,      // the big pitch knob
		NUM_PARAMS
	};
    
	enum InputIds {
		AUDIO_INPUT,
        CV_INPUT,
		NUM_INPUTS
	};
    
	enum OutputIds {
		SIN_OUTPUT,
        COS_OUTPUT,
		NUM_OUTPUTS
	};
    
   enum LightIds { 
		NUM_LIGHTS
	};

    BootyModule();
    
    /**
     * Overrides of Module functions
     */
	void step() override;
    json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
    void onSampleRateChange() override;
 
private: 
    typedef float T;
    SinOscillatorParams<T> oscParams;
	SinOscillatorState<T> oscState;
    BiquadParams<T, 3> hilbertFilterParamsSin;
	BiquadParams<T, 3> hilbertFilterParamsCos;
    BiquadState<T, 3> hilbertFilterStateSin;
	BiquadState<T, 3> hilbertFilterStateCos;
    
    LookupTableParams<T> exponential;
    
    float reciprocolSampleRate;
    public:
    float freqRange = 5;
    ChoiceButton *rangeChoice;
};

extern float values[];
extern const char* ranges[];

BootyModule::BootyModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    
    // TODO: can we assume onSampleRateChange() gets called first, so this is unneccessary?
    onSampleRateChange();
    
    // Force lazy load of lookup table with this extra call
    SinOscillator<T, true>::setFrequency(oscParams, .01);
    
    // Make 128 entry table to do exp x={-5..5} y={2..2000}
    std::function<double(double)> expFunc = AudioMath::makeFunc_Exp(-5, 5, 2, 2000);
    LookupTable<T>::init(exponential, 128, -5, 5, expFunc);
}

void BootyModule::onSampleRateChange() {
    T rate = engineGetSampleRate();
    reciprocolSampleRate = 1/rate;
    HilbertFilterDesigner<T>::design(rate, hilbertFilterParamsSin, hilbertFilterParamsCos);
}

json_t *BootyModule::toJson() {
    json_t *rootJ = json_object();
    int rg = freqRange;
    json_object_set_new(rootJ, "range", json_integer(rg));
    return rootJ;
}

void BootyModule::fromJson(json_t *rootJ)  {

    json_t *driverJ = json_object_get(rootJ, "range");
    if (driverJ) {
        const int rg = json_number_value(driverJ);
        
        // TODO: should we be more robust about float <> int issues?
        //need to tell the control what text to display
        for (int i=0; i<5; ++i) {
            if (rg == values[i]) {
               rangeChoice->text = ranges[i]; 
            }
        }
        freqRange = rg;
        fflush(stdout);
    }
}


void BootyModule::step() {
    // add the knob and the CV
    T freqHz;
    T cvTotal = params[PITCH_PARAM].value + inputs[CV_INPUT].value;
    if (cvTotal > 5) {
        cvTotal = 5;
    }
    if (cvTotal < -5) {
        cvTotal = -5;
    }
    if (freqRange > .2) {
        cvTotal *= freqRange;
        cvTotal *= 1. / 5.;
        freqHz = cvTotal;
    } else {
        freqHz = LookupTable<T>::lookup(exponential, cvTotal);
    }

    SinOscillator<float, true>::setFrequency(oscParams, freqHz * reciprocolSampleRate);
    
    // Generate the quadrature sin oscillators.
    T x, y;
    SinOscillator<T, true>::runQuadrature(x, y, oscState, oscParams);
   
    // Filter the input through th quadrature filter
    const T input = inputs[AUDIO_INPUT].value;
    const T hilbertSin = BiquadFilter<T>::run(input, hilbertFilterStateSin, hilbertFilterParamsSin);
    const T hilbertCos = BiquadFilter<T>::run(input, hilbertFilterStateCos, hilbertFilterParamsCos);
    
    // Cross modulate the two sections.
    x *= hilbertSin;
    y *= hilbertCos;
   
    // And combine for final SSB output.
    outputs[SIN_OUTPUT].value = x + y;
    outputs[COS_OUTPUT].value = x - y;
}

/***********************************************************************************
 *
 * RangeChoice selector widget
 *
 ***********************************************************************************/

const char* ranges[5] = {
    "5 hz",
    "50 hz",
    "500 hz",
    "5 khz",
    "exp"
};

float values[5] = {
    5,
    50,
    500,
    5000,
    0
};

struct RangeItem : MenuItem {
    RangeItem(int index, float * output, ChoiceButton * inParent) :
        rangeIndex(index), rangeOut(output), rangeChoice( inParent) {
            text = ranges[index];
    }

    const int rangeIndex;
    float * const rangeOut;
    ChoiceButton* const rangeChoice;
 
    void onAction(EventAction &e) override {
        rangeChoice->text = ranges[rangeIndex];
        *rangeOut = values[rangeIndex];
    }
};

struct RangeChoice : ChoiceButton {
  
    RangeChoice(float * out, const Vec& pos, float width) : output(out) {
        assert(*output == 5);
        this->text = std::string(ranges[0]);
        this->box.pos = pos;
        this->box.size.x = width;
    }
    float * const output;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();

        menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;    
		{
            menu->addChild( new RangeItem(0, output, this));
            menu->addChild( new RangeItem(1, output, this));
            menu->addChild( new RangeItem(2, output, this));
            menu->addChild( new RangeItem(3, output, this));
            menu->addChild( new RangeItem(4, output, this));
		}
	}
};

/**
 * Widget constructor will describe my implementation struct and
 * provide metadata.
 * This is not shared by all modules in the dll, just one
 */
BootyWidget::BootyWidget() {
    BootyModule *module = new BootyModule();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/booty_panel.svg")));
		addChild(panel);
	}
    
    const int leftInputX = 11;
    const int rightInputX = 55;
    
    const int row0 = 45;
    const int row1 = 102;
    static int row2 = 186;
   
    // Inputs on Row 0
    addInput(createInput<PJ301MPort>(Vec(leftInputX, row0), module, BootyModule::AUDIO_INPUT));
    addInput(createInput<PJ301MPort>(Vec(rightInputX, row0), module, BootyModule::CV_INPUT));
    
    // shift Range on row 2
    const float margin = 16; 
    float xPos = margin;
	float width =  6 * RACK_GRID_WIDTH - 2 * margin; 
   
    module->rangeChoice = new RangeChoice(&module->freqRange, Vec(xPos, row2), width); 
    addChild(module->rangeChoice);


    // knob on row 1
    addParam(createParam<Rogan3PSBlue>(Vec(18, row1), module, BootyModule::PITCH_PARAM, -5.0, 5.0, 0.0));

    const float row3 = 317.5;
    
    // Outputs on row 3
    const float leftOutputX = 9.5;
    const float rightOutputX = 55.5;
	addOutput(createOutput<PJ301MPort>(Vec(leftOutputX, row3), module, BootyModule::SIN_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(rightOutputX, row3), module, BootyModule::COS_OUTPUT));
    
    // screws
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}