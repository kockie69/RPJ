#include "RPJ.hpp"
#include "BiQuad.hpp"

template <typename T>
bool Biquad<T>::reset(T _sampleRate) {
		memset(&stateArray[0], 0, sizeof(double)*numStates);
		return true;  // handled = true
}
template <typename T>
bool Biquad<T>::canProcessAudioFrame() { 
	return false; 
}
template <typename T>
void Biquad<T>::setCoeffs() {
		coeffArray[a0]=module->params[Filter::PARAM_A0].getValue();
		coeffArray[a1]=module->params[Filter::PARAM_A1].getValue();
		coeffArray[a2]=module->params[Filter::PARAM_A2].getValue();
		coeffArray[b1]=module->params[Filter::PARAM_B1].getValue();
		coeffArray[b2]=module->params[Filter::PARAM_B2].getValue();
}
template <typename T>
bool Biquad<T>::checkFloatUnderflow(T& value)
{
	bool retValue = false;
	if (value > 0.0 && value < kSmallestPositiveFloatValue)
	{
		value = 0;
		retValue = true;
	}
	else if (value < 0.0 && value > kSmallestNegativeFloatValue)
	{
		value = 0;
		retValue = true;
	}
	return retValue;
}

template <typename T>
T Biquad<T>::processAudioSample(T xn) {
	yn=0;
	if (module->params[Filter::PARAM_ALGO].getValue() == Filter::biquadAlgorithm::kDirect) {
		// --- 1)  form output y(n) = a0*x(n) + a1*x(n-1) + a2*x(n-2) - b1*y(n-1) - b2*y(n-2)
		yn = coeffArray[a0] * xn + 
					coeffArray[a1] * stateArray[x_z1] +
					coeffArray[a2] * stateArray[x_z2] -
					coeffArray[b1] * stateArray[y_z1] -
					coeffArray[b2] * stateArray[y_z2];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[x_z2] = stateArray[x_z1];
		stateArray[x_z1] = xn;

		stateArray[y_z2] = stateArray[y_z1];
		stateArray[y_z1] = yn;

		return yn;
	}
	else if (module->params[Filter::PARAM_ALGO].getValue() == Filter::biquadAlgorithm::kCanonical)
	{
		// --- 1)  form output y(n) = a0*w(n) + m_f_a1*stateArray[x_z1] + m_f_a2*stateArray[x_z2][x_z2];
		//
		// --- w(n) = x(n) - b1*stateArray[x_z1] - b2*stateArray[x_z2]
		double wn = xn - coeffArray[b1] * stateArray[x_z1] - coeffArray[b2] * stateArray[x_z2];

		// --- y(n):
		double yn = coeffArray[a0] * wn + coeffArray[a1] * stateArray[x_z1] + coeffArray[a2] * stateArray[x_z2];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[x_z2] = stateArray[x_z1];
		stateArray[x_z1] = wn;

		// --- return value
		return yn;
	}
	else if (module->params[Filter::PARAM_ALGO].getValue() == Filter::biquadAlgorithm::kTransposeDirect)
	{
		// --- 1)  form output y(n) = a0*w(n) + stateArray[x_z1]
		//
		// --- w(n) = x(n) + stateArray[y_z1]
		double wn = xn + stateArray[y_z1];

		// --- y(n) = a0*w(n) + stateArray[x_z1]
		double yn = coeffArray[a0] * wn + stateArray[x_z1];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- 3) update states
		stateArray[y_z1] = stateArray[y_z2] - coeffArray[b1] * wn;
		stateArray[y_z2] = -coeffArray[b2] * wn;

		stateArray[x_z1] = stateArray[x_z2] + coeffArray[a1] * wn;
		stateArray[x_z2] = coeffArray[a2] * wn;

		// --- return value
		return yn;
	}
	else if (module->params[Filter::PARAM_ALGO].getValue() == Filter::biquadAlgorithm::kTransposeCanonical)
	{
		// --- 1)  form output y(n) = a0*x(n) + stateArray[x_z1]
		double yn = coeffArray[a0] * xn + stateArray[x_z1];

		// --- 2) underflow check
		checkFloatUnderflow(yn);

		// --- shuffle/update
		stateArray[x_z1] = coeffArray[a1]*xn - coeffArray[b1]*yn + stateArray[x_z2];
		stateArray[x_z2] = coeffArray[a2]*xn - coeffArray[b2]*yn;

		// --- return value
		return yn;
	}
	return xn; // didn't process anything :(
}


std::string AlgoQuantity::getDisplayValueString() {
    value = static_cast<Filter::biquadAlgorithm>(getValue());

	switch (value) {
		case Filter::kDirect: 
			v = "kDirect";
			break;
		case Filter::kCanonical: 
			v = "kCanonical";
			break;
		case Filter::kTransposeDirect: 
			v = "TransposeDirect";
			break;
		case Filter::kTransposeCanonical: 
			v = "TransposeCanonical";
			break;
		default:
			break;
	}
	return v;
}

Filter::Filter() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(PARAM_A0, -1.5f, 1.0f, 0.0f, "A0");
	configParam(PARAM_A1, -1.5f, 1.0f, 0.0f, "A1");
	configParam(PARAM_A2, -1.5f, 1.0f, 0.0f, "A2");
	configParam(PARAM_B1, -1.5f, 1.0f, 0.0f, "B1");
	configParam(PARAM_B2, -1.5f, 1.0f, 0.0f, "B2");
	configParam(PARAM_C0, -1.5f, 1.0f, 0.0f, "C0");
	configParam(PARAM_D0, -1.5f, 1.0f, 0.0f, "D0");
	configParam<AlgoQuantity>(PARAM_ALGO, 0.0, 3.0, 0.0, "Algorithm");
	filter.module=this;
}

void Filter::process(const ProcessArgs &args) {

	filter.setCoeffs();

	float out = filter.processAudioSample(inputs[INPUT_MAIN].getVoltage());

	outputs[OUTPUT_MAIN].setVoltage(out);
}

struct BiQuadModuleWidget : ModuleWidget {
	BiQuadModuleWidget(Filter* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/myVCF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("BiQuad");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(18, 60));
			title->setText("A");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(65, 60));
			title->setText("B");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(13, 250));
			title->setText("IN");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 250));
			title->setText("OUT");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 280), module, Filter::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 230), module, Filter::OUTPUT_MAIN));

		{
			auto w = createParam<Knob16>(Vec(32,50), module, Filter::PARAM_ALGO);
			auto k = dynamic_cast<SvgKnob*>(w);
			k->snap = true;
			k->minAngle = -0.75*M_PI;
			k->maxAngle = 0.75*M_PI;
			addParam(w);
		}
		addParam(createParam<RoundBlackKnob>(Vec(8, 90), module, Filter::PARAM_A0));
		addParam(createParam<RoundBlackKnob>(Vec(8, 130), module, Filter::PARAM_A1));	
		addParam(createParam<RoundBlackKnob>(Vec(8, 170), module, Filter::PARAM_A2));
		addParam(createParam<RoundBlackKnob>(Vec(55, 90), module, Filter::PARAM_B1));
		addParam(createParam<RoundBlackKnob>(Vec(55, 130), module, Filter::PARAM_B2));
	}

};

Model * modelVCF = createModel<Filter, BiQuadModuleWidget>("BiQuad");