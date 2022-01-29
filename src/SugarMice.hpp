#include "rack.hpp"
#include "WDF.hpp"

using namespace rack;

const int MODULE_WIDTH=6;

struct SugarMice : Module {

	enum ParamIds {
		PARAM_FC,
		PARAM_CVFC,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
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
		void onSampleRateChange() override;
		void processChannel(int, Input&, Output&);
		WDFTunableButterLPF3 wdfButterLPF3[4];
		double sampleRate, fc;
		bool warp;
};

/* Context Menu Item for changing the Jumper settings */
struct nWarpSelectionMenuItem : MenuItem {
	SugarMice *module;
	bool Warp;
	void onAction(const event::Action &) override;
};
