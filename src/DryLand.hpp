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
		AudioFilter LPFaudioFilter;
		AudioFilter HPFaudioFilter;
		void process(const ProcessArgs &) override;
		AudioFilterParameters LPFafp,HPFafp;
};


