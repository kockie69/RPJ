#include "rack.hpp"
#include "AudioFilter.hpp"

using namespace rack;

const int MODULE_WIDTH=6;

struct Gazpacho : Module {

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

		Gazpacho();
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		void onSampleRateChange() override; 
		AudioFilter LPFaudioFilter;
		AudioFilter HPFaudioFilter;
		void process(const ProcessArgs &) override;
		dsp::SchmittTrigger upTrigger,downTrigger;
		AudioFilterParameters LPFafp,HPFafp;
		biquadAlgorithm bqa;
};


