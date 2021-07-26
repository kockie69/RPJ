#include "rack.hpp"
#include "ZVA.hpp"

using namespace rack;

const int MODULE_WIDTH=7;

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
		ZVAFilter LPFaudioFilter,HPFaudioFilter,BPFaudioFilter,BSFaudioFilter;
		void process(const ProcessArgs &) override;
		ZVAFilterParameters LPFafp,HPFafp,BPFafp,BSFafp;
		bool gain, nlp, osc, match;
};

/* Context Menu Item for changing the Gain Compensation setting */
struct nGainCompensationMenuItem : MenuItem {
	LadyNina *module;
	bool Gain;
	void onAction(const event::Action &) override;
};

/* Context Menu Item for changing the NLP setting */
struct nNLPMenuItem : MenuItem {
	LadyNina *module;
	bool NLP;
	void onAction(const event::Action &) override;
};

/* Context Menu Item for changing the Self Oscilation setting */
struct nOscMenuItem : MenuItem {
	LadyNina *module;
	bool Osc;
	void onAction(const event::Action &) override;
};

/* Context Menu Item for changing the Match Analog gain at Nyquist setting */
struct nMatchMenuItem : MenuItem {
	LadyNina *module;
	bool Match;
	void onAction(const event::Action &) override;
};