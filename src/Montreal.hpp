#include "rack.hpp"
#include "WDF.hpp"

using namespace rack;

struct Montreal : Module {

	enum ParamIds {
		PARAM_FC,
		PARAM_Q,
		NUM_PARAMS,
	};

	enum InputIds {
		INPUT_MAIN,
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
		WDFIdealRLCLPF wdfIdealRLCLPF;
		WDFIdealRLCHPF wdfIdealRLCHPF;
		WDFIdealRLCBPF wdfIdealRLCBPF;
		WDFIdealRLCBSF wdfIdealRLCBSF;
		double sampleRate;
		WDFParameters wdfp;
};


