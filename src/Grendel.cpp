
#include "Grendel.hpp"

template <typename T>
LFO<T>::LFO() {
	phase = 0;
	frequency = 1;
	pw = 0.5f;
	amplitude=1.f;
	resetState = T::mask();
}
template <typename T>
void LFO<T>::setReset(T reset) {
	reset = simd::rescale(reset, 0.1f, 2.f, 0.f, 1.f);
	T on = (reset >= 1.f);
	T off = (reset <= 0.f);
	T triggered = ~resetState & on;
	resetState = simd::ifelse(off, 0.f, resetState);
	resetState = simd::ifelse(on, T::mask(), resetState);
	if (triggered.v[0])
		phase = 0;
}
template <typename T>
void LFO<T>::setPitch(T pitch) {
	pitch = simd::fmin(pitch, 10.f);
	frequency = dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
}
template <typename T>
void LFO<T>::setPhase(T phase0, T dPhase) {
	phase = phase0.v[0] + dPhase.v[0];

	if (phase >=1)
		phase -= 1;
}
template <typename T>
void LFO<T>::setPhase(T phase0) {
	phase = phase0.v[0];

	if (phase >=1)
		phase -= 1;
}
template <typename T>
T LFO<T>::getPhase() {
	return phase;
}
template <typename T>
void LFO<T>::setAmplitude(T ampl) {
	amplitude = ampl;
}
template <typename T>
void LFO<T>::setFrequency(T freq) {
	frequency = freq;
}
template <typename T>
T LFO<T>::getAmplitude() {
	return amplitude;
}
template <typename T>
T LFO<T>::getFrequency() {
	return frequency;
}
template <typename T>
void LFO<T>::step(float dt) {
	T deltaPhase = simd::fmin(frequency * dt, 0.5f);
	phase = phase + deltaPhase.v[0];

	if (phase >=1)
		phase -= 1;
}
template <typename T>
T LFO<T>::sin() {
	T p = phase;
	T v = simd::sin(2 * M_PI * p);
	return v;
}
template <typename T>
T LFO<T>::tri() {
	T p = phase;
	T v = 4.f * simd::fabs(p - simd::round(p)) - 1.f;
	return v;
}
template <typename T>
T LFO<T>::sawup() {
	T p = phase;
	T v = 2.f * (p - simd::round(p));
	return v;
}
template <typename T>
T LFO<T>::sawdown() {
	T p = phase;
	T v = -2.f * (p - simd::round(p));
	return v;
}
template <typename T>
T LFO<T>::sqr() {
	T v = simd::ifelse(phase < pw, 1.f, -1.f);
	
	return v;
}
template <typename T>
T LFO<T>::light() {
	return simd::sin(2 * T(M_PI) * phase);
}
template <typename T>
void LFO<T>::process(float_4 sampleTime) {
}

std::string FreqQuantity::getDisplayValueString() {
	if ( module == NULL) 
		v = "";
	else {
		if ( module->params[RPJLFO::MODE_PARAM].getValue() == RPJLFO::FREE_MODE)
			v = std::to_string(pow(2,getValue()));
		if ( module->params[RPJLFO::MODE_PARAM].getValue() == RPJLFO::QUAD_MODE)
			v = std::to_string(getValue());
		if ( module->params[RPJLFO::MODE_PARAM].getValue() == RPJLFO::PHASE_MODE)
			v = std::to_string(getValue());
		if ( module->params[RPJLFO::MODE_PARAM].getValue() == RPJLFO::DIVIDE_MODE)
			v = std::to_string(getValue());
	}
	return v; 
}

std::string ModeQuantity::getDisplayValueString() {
    value = static_cast<RPJLFO::ModeIds>(getValue());

	switch (value) {
		case RPJLFO::FREE_MODE: 
			v = "Free";
			break;
		case RPJLFO::QUAD_MODE: 
			v = "Quad";
			break;
		case RPJLFO::PHASE_MODE: 
			v = "Phase";
			break;
		case RPJLFO::DIVIDE_MODE: 
			v = "Divide";
			break;
		default:
			break;
	}
	return v;
}


RPJLFO::RPJLFO() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(FREQ1_PARAM, -7.f, 7.f, 1.f, "Frequency", " Hz", 2, 1,0);
	configParam<FreqQuantity>(FREQ2_PARAM, -7.f, 7.f, 1.f, "Frequency", " Hz", 2, 1,0);
	configParam<FreqQuantity>(FREQ3_PARAM, -7.f, 7.f, 1.f, "Frequency", " Hz", 2, 1,0);
	configParam<FreqQuantity>(FREQ4_PARAM, -7.f, 7.f, 1.f, "Frequency", " Hz", 2, 1,0);
	configParam<ModeQuantity>(MODE_PARAM, 0.0, 3.0, 0.0, "Mode");
	lightDivider.setDivision(16);
	parameter[0] = dynamic_cast<ParamQuantity*>(paramQuantities[FREQ1_PARAM]);
	parameter[1] = dynamic_cast<ParamQuantity*>(paramQuantities[FREQ2_PARAM]);
	parameter[2] = dynamic_cast<ParamQuantity*>(paramQuantities[FREQ3_PARAM]);
	parameter[3] = dynamic_cast<ParamQuantity*>(paramQuantities[FREQ4_PARAM]);
	prevMode = NUM_MODES;
	parameter[0]->module = this;
}

void RPJLFO::process(const ProcessArgs &args) {                                                                

	mode=static_cast<ModeIds>(params[MODE_PARAM].getValue());

	
	for (int i=0;i<4;i++) {
		freqParam = params[FREQ1_PARAM+i].getValue();
		if (inputs[FRQ_PH_DIV1_INPUT+i].isConnected())
		   cvInput = simd::clamp(inputs[FRQ_PH_DIV1_INPUT+i].getVoltage(),0.f,5.f);
		else cvInput=5.f;

		if (prevMode != mode) {
			if (i != 0 ) {
				switch (mode) {
					case FREE_MODE:
						parameter[i]->unit=" Hz";
						parameter[i]->label="Frequency";
						parameter[i]->minValue=-7.f;
						parameter[i]->maxValue=7.f;
						parameter[i]->displayBase=2;	
						break;
					case QUAD_MODE:
						parameter[i]->unit="";
						parameter[i]->label="Attenuation";
						parameter[i]->minValue=0;
						parameter[i]->maxValue=1;
						parameter[i]->displayBase=0;
						break;
					case PHASE_MODE:
						parameter[i]->unit=" Degrees";
						parameter[i]->label="Phase";
						parameter[i]->minValue=0;
						parameter[i]->maxValue=360;
						parameter[i]->displayBase=0;
						break;
					case DIVIDE_MODE:
						parameter[i]->unit="";
						parameter[i]->label="Ratio";
						parameter[i]->minValue=2;
						parameter[i]->maxValue=32;
						parameter[i]->displayBase=0;
						break;
					default:
						break;
				}
			}
		}
		switch (mode) {
		case FREE_MODE:
			oscillator[i].setAmplitude(1);
			pitch = freqParam * cvInput/5.f;
			oscillator[i].setPitch(pitch);
			break;
		case QUAD_MODE:
			if (i==0)
				pitch0 = freqParam*cvInput/5.f;
			else {
				oscillator[i].setPhase(oscillator[0].getPhase(), i * 0.25);
				oscillator[i].setAmplitude(freqParam * cvInput/5.f);
			}
			oscillator[i].setPitch(pitch0);
			break;
		case PHASE_MODE:
			oscillator[i].setAmplitude(1);
			if (i==0)
				pitch0 = freqParam*cvInput/5.f;
			else 
				oscillator[i].setPhase(oscillator[0].getPhase(), (-freqParam/360) * (cvInput/5));
			oscillator[i].setPitch(pitch0);
			break;
		case DIVIDE_MODE:
			oscillator[i].setAmplitude(1);
			if (i==0) {
				pitch0 = freqParam;
				oscillator[0].setPitch(pitch0);
			}
			else {
				v = simd::ifelse(cvInput,cvInput/5.f,1.f);
				oscillator[i].setFrequency(oscillator[0].getFrequency() / ((freqParam) * v));
			}
			break;
		default:
			break;
		}
		
		oscillator[i].step(args.sampleTime);
		oscillator[i].setReset(inputs[RESET1_INPUT+i].getVoltage());

		if (outputs[SIN_OUTPUT1+i].isConnected())
			outputs[SIN_OUTPUT1+i].setVoltageSimd(5.f * oscillator[i].getAmplitude() * oscillator[i].sin(),0);
		if (outputs[TRI_OUTPUT1+i].isConnected())
			outputs[TRI_OUTPUT1+i].setVoltageSimd(5.f * oscillator[i].getAmplitude() * oscillator[i].tri(),0);
		if (outputs[SAWUP_OUTPUT1+i].isConnected())
			outputs[SAWUP_OUTPUT1+i].setVoltageSimd(5.f * oscillator[i].getAmplitude() * oscillator[i].sawup(),0); 
		if (outputs[SAWDOWN_OUTPUT1+i].isConnected())
			outputs[SAWDOWN_OUTPUT1+i].setVoltageSimd(5.f * oscillator[i].getAmplitude() * oscillator[i].sawdown(),0); 
		if (outputs[SQR_OUTPUT1+i].isConnected())
			outputs[SQR_OUTPUT1+i].setVoltageSimd(5.f * oscillator[i].getAmplitude() * oscillator[i].sqr(),0);
		
		if (lightDivider.process()) {
			for (int i=0;i<4;i++) {
				for (int j=0;j<4;j++)
					lights[FREQ1_LIGHT + i*4 +j].setBrightness(0);
				float lightValue = oscillator[i].light().s[0];
				lights[FREQ1_LIGHT + i*4 + mode].setSmoothBrightness(lightValue, args.sampleTime * lightDivider.getDivision());
			}
		}
	}
	prevMode = mode;
}

BGKnob::BGKnob(int dim) {
	setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
	box.size = Vec(dim, dim);
	shadow->blurRadius = 2.0;
	// k->shadow->opacity = 0.15;
	shadow->box.pos = Vec(0.0, 3.0);
}

Knob16::Knob16() : BGKnob(24) {
	shadow->box.pos = Vec(0.0, 2.5);
}

struct RPJLFOModuleWidget : ModuleWidget {
	RPJLFOModuleWidget(RPJLFO* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/myLFO.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		{
			auto w = createParam<Knob16>(Vec(73,35), module, RPJLFO::MODE_PARAM);
			auto k = dynamic_cast<SvgKnob*>(w);
			k->snap = true;
			k->minAngle = -0.75*M_PI;
			k->maxAngle = 0.75*M_PI;
			//k->speed = 3.0;
			addParam(w);
		}
	
		addChild(createLight<MediumLight<RedGreenBlueYellowLight>>(Vec(34, 95), module, RPJLFO::FREQ1_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueYellowLight>>(Vec(70, 95), module, RPJLFO::FREQ2_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueYellowLight>>(Vec(105, 95), module, RPJLFO::FREQ3_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueYellowLight>>(Vec(140, 95), module, RPJLFO::FREQ4_LIGHT));
	
		addParam(createParam<RoundSmallBlackKnob>(Vec(27, 110), module, RPJLFO::FREQ1_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(Vec(63, 110), module, RPJLFO::FREQ2_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(Vec(98, 110), module, RPJLFO::FREQ3_PARAM));
		addParam(createParam<RoundSmallBlackKnob>(Vec(134, 110), module, RPJLFO::FREQ4_PARAM));

		addInput(createInput<PJ301MPort>(Vec(26, 142), module, RPJLFO::FRQ_PH_DIV1_INPUT));
		addInput(createInput<PJ301MPort>(Vec(62, 142), module, RPJLFO::FRQ_PH_DIV2_INPUT));
		addInput(createInput<PJ301MPort>(Vec(97, 142), module, RPJLFO::FRQ_PH_DIV3_INPUT));
		addInput(createInput<PJ301MPort>(Vec(132, 142), module, RPJLFO::FRQ_PH_DIV4_INPUT));

		addInput(createInput<PJ301MPort>(Vec(26, 173), module, RPJLFO::RESET1_INPUT));
		addInput(createInput<PJ301MPort>(Vec(62, 173), module, RPJLFO::RESET2_INPUT));
		addInput(createInput<PJ301MPort>(Vec(97, 173), module, RPJLFO::RESET3_INPUT));
		addInput(createInput<PJ301MPort>(Vec(132, 173), module, RPJLFO::RESET4_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(26, 204), module, RPJLFO::SAWDOWN_OUTPUT1));
		addOutput(createOutput<PJ301MPort>(Vec(62, 204), module, RPJLFO::SAWDOWN_OUTPUT2));
		addOutput(createOutput<PJ301MPort>(Vec(97, 204), module, RPJLFO::SAWDOWN_OUTPUT3));
		addOutput(createOutput<PJ301MPort>(Vec(132, 204), module, RPJLFO::SAWDOWN_OUTPUT4));

		addOutput(createOutput<PJ301MPort>(Vec(26, 235), module, RPJLFO::SAWUP_OUTPUT1));
		addOutput(createOutput<PJ301MPort>(Vec(62, 235), module, RPJLFO::SAWUP_OUTPUT2));
		addOutput(createOutput<PJ301MPort>(Vec(97, 235), module, RPJLFO::SAWUP_OUTPUT3));
		addOutput(createOutput<PJ301MPort>(Vec(132, 235), module, RPJLFO::SAWUP_OUTPUT4));

		addOutput(createOutput<PJ301MPort>(Vec(26, 266), module, RPJLFO::SQR_OUTPUT1));
		addOutput(createOutput<PJ301MPort>(Vec(62, 266), module, RPJLFO::SQR_OUTPUT2));
		addOutput(createOutput<PJ301MPort>(Vec(97, 266), module, RPJLFO::SQR_OUTPUT3));
		addOutput(createOutput<PJ301MPort>(Vec(132, 266), module, RPJLFO::SQR_OUTPUT4));

		addOutput(createOutput<PJ301MPort>(Vec(26, 297), module, RPJLFO::TRI_OUTPUT1));
		addOutput(createOutput<PJ301MPort>(Vec(62, 297), module, RPJLFO::TRI_OUTPUT2));
		addOutput(createOutput<PJ301MPort>(Vec(97, 297), module, RPJLFO::TRI_OUTPUT3));
		addOutput(createOutput<PJ301MPort>(Vec(132, 297), module, RPJLFO::TRI_OUTPUT4));

		addOutput(createOutput<PJ301MPort>(Vec(26, 328), module, RPJLFO::SIN_OUTPUT1));
		addOutput(createOutput<PJ301MPort>(Vec(62, 328), module, RPJLFO::SIN_OUTPUT2));
		addOutput(createOutput<PJ301MPort>(Vec(97, 328), module, RPJLFO::SIN_OUTPUT3));
		addOutput(createOutput<PJ301MPort>(Vec(132, 328), module, RPJLFO::SIN_OUTPUT4));

	}
};

Model * modelLFO = createModel<RPJLFO, RPJLFOModuleWidget>("Grendel");