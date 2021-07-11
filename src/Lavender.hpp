#include "rack.hpp"
#include "AudioFilter.hpp"

using namespace rack;

struct Lavender : Module {

	enum ParamIds {
		PARAM_UP,
		PARAM_DOWN,
		PARAM_FC,
		PARAM_CVFC,
		PARAM_Q,
		PARAM_CVQ,
		PARAM_BOOSTCUT_DB,
		PARAM_DRY,
		PARAM_WET,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
		INPUT_CVQ,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_LPFMAIN,
		OUTPUT_HPFMAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		Lavender();
		AudioFilter LPFaudioFilter;
		AudioFilter HPFaudioFilter;
		void process(const ProcessArgs &) override;
		dsp::SchmittTrigger upTrigger,downTrigger;
		AudioFilterParameters LPFafp,HPFafp;
};


