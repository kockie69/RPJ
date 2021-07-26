#include "rack.hpp"
#include "EnvelopeFollower.hpp"

using namespace rack;

const int MODULE_WIDTH=6;
		
struct Gaza : Module {

	enum ParamIds {
		PARAM_REL,
		PARAM_ATK,
        PARAM_FC,
        PARAM_Q,
        PARAM_THRES,
        PARAM_SENS,
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
		Gaza();
		EnvelopeFollower envelopeFollower;
		void process(const ProcessArgs &) override;
		EnvelopeFollowerParameters efp;
};

struct DetectModeQuantity : public rack::engine::ParamQuantity {
	public:
    	std::string getDisplayValueString() override;

	private:
		int value;
		std::string v;
};
