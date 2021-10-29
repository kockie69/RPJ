#include "rack.hpp"
#include "AudioFilter.hpp"

using namespace rack;

const int MODULE_WIDTH=6;
		
struct Essence : Module {

	enum ParamIds {
		PARAM_FC,
		PARAM_CVFC,
		PARAM_Q,
		PARAM_CVQ,
		PARAM_BOOSTCUT_DB,
		PARAM_CVB,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
		INPUT_CVQ,
		INPUT_CVB,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_MAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		Essence();
		AudioFilter audioFilter;
		void process(const ProcessArgs &) override;
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		void onSampleRateChange() override; 
		AudioFilterParameters afp;
		biquadAlgorithm bqa;
};


