#include "rack.hpp"
#include "ZVA.hpp"

using namespace rack;

const int MODULE_WIDTH=6;

const char *JSON_GAIN_KEY="Gain";
const char *JSON_NLP_KEY="Nlp";
const char *JSON_OSCILLATOR_KEY="Oscillator";
const char *JSON_MATCH_KEY="Match";

struct LadyNina : Module {

	enum ParamIds {
		PARAM_UP,
		PARAM_DOWN,
		PARAM_FC,
		PARAM_Q,
		PARAM_BOOSTCUT_DB,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
		INPUT_CVQ,
		INPUT_CVBCDB,
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

		LadyNina();
		json_t *dataToJson() override;
		void dataFromJson(json_t *) override;
		void onSampleRateChange() override;
		ZVAFilter LPFaudioFilter[4],HPFaudioFilter[4],BPFaudioFilter[4],BSFaudioFilter[4];
		void process(const ProcessArgs &) override;
		void processChannel(Input&, Output&, Output&, Output&, Output&);
		ZVAFilterParameters LPFafp,HPFafp,BPFafp,BSFafp;
		bool gain, nlp, osc, match;
};
