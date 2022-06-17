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

	enum outputTypes {
		LINEAR,
		LOG,	
	};

	BlindCurve();
	void onSampleRateChange() override;
	void process(const ProcessArgs &) override;
	void processChannel(int, Input&, Output&);
	AudioDetector<rack::simd::float_4> audioDetector[4];
	AudioDetectorParameters adp;
	bool clamping;
	outputTypes outputType; 
};

struct DetectModeQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;
};
