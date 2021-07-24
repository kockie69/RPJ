#include "rack.hpp"
#include "EnvelopeFollower.hpp"

using namespace rack;
		
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
		AudioDetector audioDetector;
		void process(const ProcessArgs &) override;
		AudioDetectorParameters adp;
};

struct DetectModeQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;

	private:
		int value;
		std::string v;
};
