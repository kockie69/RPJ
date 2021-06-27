#include "rack.hpp"

using namespace rack;

template <typename T>
struct SVF {
	private:
		T hp, bp, lp, phi, gamma;
		T fc, damp;

	public:
		SVF(T, T); 
		void setCoeffs(T, T);
		void reset();
		void process(T , T* , T* , T* );
};

struct RPJMMSVFilter : Module {
	enum ParamIds {
		ATT_CUTOFF,
		PARAM_CUTOFF,
		ATT_DAMP,
		PARAM_DAMP,
		NUM_PARAMS,
	};

	enum InputIds {
		MAIN_IN,
		CUTOFF_IN,
		DAMP_IN,
		NUM_INPUTS,
	};

	enum OutputIds {
		LPF_OUT,
		BPF_OUT,
		HPF_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		NUM_LIGHTS,
	};
	private:
		float hpf, bpf, lpf;
		SVF<float> * filter;
		float fc;
		float dampCV;
	public:
		RPJMMSVFilter();
		void process(const ProcessArgs &) override;
};

struct knobSmall : SvgKnob  {
	knobSmall() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_16px.svg")));
		minAngle = -0.7*M_PI;
		maxAngle = 0.7*M_PI;

	}
};
