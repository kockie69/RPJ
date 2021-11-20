#include "rack.hpp"
#include "EnvelopeFollower.hpp"

using namespace rack;

const int MODULE_WIDTH=6;	
	
struct BlindCurve : Module {

	enum ParamIds {
		PARAM_REL,
		PARAM_ATK,
		PARAM_MODE,
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
		BlindCurve();
		void onSampleRateChange() override;
		AudioDetector<rack::simd::float_4> audioDetector[4];
		void process(const ProcessArgs &) override;
		void processChannel(Input&, Output&);
		AudioDetectorParameters adp;
};

struct DetectModeQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;

	private:
		int value;
		std::string v;
};
