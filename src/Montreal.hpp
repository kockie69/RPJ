#include "rack.hpp"
#include "WDF.hpp"

using namespace rack;

const int MODULE_WIDTH=8;

struct Montreal : Module {

	enum ParamIds {
		PARAM_FC,
		PARAM_CVFC,
		PARAM_Q,
		PARAM_CVQ,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
		INPUT_CVFC,
		INPUT_CVQ,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_LPF,
		OUTPUT_HPF,
		OUTPUT_BPF,
		OUTPUT_BSF,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};

		Montreal();
		void process(const ProcessArgs &) override;
		void onSampleRateChange() override;
		void processChannel(int, Input&, Output&, Output&, Output&, Output&);
		WDFIdealRLCLPF wdfIdealRLCLPF[4];
		WDFIdealRLCHPF wdfIdealRLCHPF[4];
		WDFIdealRLCBPF wdfIdealRLCBPF[4];
		WDFIdealRLCBSF wdfIdealRLCBSF[4];
		double sampleRate;
		WDFParameters wdfp;
		float fc;
};


