#include "rack.hpp"
#include "AudioFilter.hpp"

using namespace rack;

const int MODULE_WIDTH=6;

struct DryLand : Module {

	enum ParamIds {
		PARAM_FC,
		PARAM_CVFC,
		PARAM_DRY,
		PARAM_WET,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
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

		DryLand();
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		AudioFilter LPFaudioFilter[4];
		AudioFilter HPFaudioFilter[4];
		void process(const ProcessArgs &) override;
		void processChannel(Input&, Output&, Output&);
		void onSampleRateChange() override; 
		AudioFilterParameters LPFafp,HPFafp;
		biquadAlgorithm bqa;
};


