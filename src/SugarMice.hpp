#include "rack.hpp"
#include "WDF.cpp"

using namespace rack;

struct WDFButterLPF3() {
    WDFButterLPF3();
    WdfSeriesAdapter seriesAdapter_L1;
    WdfParallelAdapter parallelAdapter_C1;
    WdfSeriesTerminatedAdapter seriesTerminatedAdapater_L2;
}

enum class wdfComponent {R, L, C, seriesLC, parallelLC,  seriesRL, parallelRL, seriesRC, parallelRC}; 

struct SugarMice : Module {

	enum ParamIds {
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
};


