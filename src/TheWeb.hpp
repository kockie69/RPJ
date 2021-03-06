#include "rack.hpp"
#include "AudioFilter.hpp"

using namespace rack;

const int MODULE_WIDTH=8;

struct TheWeb : Module {

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
		OUTPUT_BPFMAIN,
		OUTPUT_BSFMAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		TheWeb();
		void processChannel(int, Input&, Output&, Output&, Output&, Output&);
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		AudioFilter<rack::simd::float_4> LPFaudioFilter[4],HPFaudioFilter[4],BPFaudioFilter[4],BSFaudioFilter[4];
		void process(const ProcessArgs &) override;
		void onSampleRateChange() override;
		dsp::SchmittTrigger upTrigger,downTrigger;
		AudioFilterParameters LPFafp,HPFafp,BPFafp,BSFafp;
		biquadAlgorithm bqaUI;
};


