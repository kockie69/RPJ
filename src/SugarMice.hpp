#include "rack.hpp"
#include "WDF.hpp"

using namespace rack;

const int MODULE_WIDTH=3;

struct SugarMice : Module {

	enum ParamIds {
		PARAM_FC,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_MAIN,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		SugarMice();
		void process(const ProcessArgs &) override;
		WDFTunableButterLPF3 wdfButterLPF3;
		double sampleRate, fc;
		bool warp;
};

/* Context Menu Item for changing the Jumper settings */
struct nWarpSelectionMenuItem : MenuItem {
	SugarMice *module;
	bool Warp;
	void onAction(const event::Action &) override;
};
