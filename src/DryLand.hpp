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
		PARAM_DRIVE,
		PARAM_CVDRIVE,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
		INPUT_CVDRIVE,
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
		AudioFilter<rack::simd::float_4> LPFaudioFilter[4];
		AudioFilter<rack::simd::float_4> HPFaudioFilter[4];
		void process(const ProcessArgs &) override;
		void processChannel(int, Input&, Output&, Output&);
		void onSampleRateChange() override; 
		AudioFilterParameters LPFafp,HPFafp;
		biquadAlgorithm bqaUI;
};


