#include "RPJ.hpp"
#include "LadyNina.hpp"

template <typename T>
SVF<T>::SVF(T fc, T damp) {
		setCoeffs(fc, damp);
		reset();
	}
template <typename T>
void SVF<T>::setCoeffs(T fc, T damp) {
	if (this->fc != fc || this->damp != damp) {

		this->fc = fc;
		this->damp = damp;

		phi = clamp( fc * APP->engine->getSampleTime(),0.f, pow(2.f,2/3));

		gamma = clamp( damp, 0.f, ((4.f-pow(phi,3))/(2.f*phi)));
		}
}
template <typename T>
void SVF<T>::reset() {
	hp = bp = lp = 0.0;
}
template <typename T>
void SVF<T>::process(T xn, T* hpf, T* bpf, T* lpf) {
		lp = *lpf = lp+phi*phi*bp;
		hp = *hpf = xn - lp - gamma*bp;
		bp = *bpf = phi*hp + bp;

}




RPJMMSVFilter::RPJMMSVFilter() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(ATT_CUTOFF, 0.f, 1.f, 0.5f, "Cutoff Attenuator");
	configParam(PARAM_CUTOFF, 0.f, pow(2.f,2/3), 0.5f, "Cutoff");
	configParam(ATT_DAMP, 0.f, 1.f, 0.5f, "Damp Attenuator");
	configParam(PARAM_DAMP, 0.000001f, 4.f, 1.f);
	filter = new SVF<float>(100, 0.1);
	hpf = bpf = lpf = 0.f;
}

void RPJMMSVFilter::process(const ProcessArgs &args) {

	fc = args.sampleRate * params[PARAM_CUTOFF].getValue();

	fc += pow(inputs[CUTOFF_IN].getVoltage() * params[ATT_CUTOFF].getValue(),10);
	dampCV =1.f;
	if (inputs[DAMP_IN].isConnected())
		dampCV=(inputs[DAMP_IN].getVoltage()/5.f) * params[ATT_DAMP].getValue();
 	filter->setCoeffs(fc, clamp(params[PARAM_DAMP].getValue() + dampCV,0.1f, 4.0f));

	filter->process(inputs[MAIN_IN].getVoltageSum(), &hpf, &bpf, &lpf);

	outputs[LPF_OUT].setVoltage(lpf);
	outputs[BPF_OUT].setVoltage(bpf);
	outputs[HPF_OUT].setVoltage(hpf);

}

struct RPJMMSVFilterModuleWidget : ModuleWidget {
	RPJMMSVFilterModuleWidget(RPJMMSVFilter * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/myVCF.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("Lady Nina");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(20, 35));
			title->setText("CUTOFF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(30, 115));
			title->setText("DAMP");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(13, 250));
			title->setText("IN");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 200));
			title->setText("LPF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 250));
			title->setText("BPF");
			addChild(title);
		}
		{
			ATextLabel * title = new ATextLabel(Vec(55, 300));
			title->setText("HPF");
			addChild(title);
		}

		addInput(createInput<PJ301MPort>(Vec(10, 280), module, RPJMMSVFilter::MAIN_IN));

		addOutput(createOutput<PJ301MPort>(Vec(55, 230), module, RPJMMSVFilter::LPF_OUT));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, RPJMMSVFilter::BPF_OUT));
		addOutput(createOutput<PJ301MPort>(Vec(55, 330), module, RPJMMSVFilter::HPF_OUT));

		addParam(createParam<knobSmall>(Vec(8, 60), module, RPJMMSVFilter::ATT_CUTOFF));
		addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, RPJMMSVFilter::PARAM_CUTOFF));
		addInput(createInput<PJ301MPort>(Vec(10, 100), module, RPJMMSVFilter::CUTOFF_IN));
		
		addParam(createParam<knobSmall>(Vec(8, 140), module, RPJMMSVFilter::ATT_DAMP));
		addParam(createParam<RoundBlackKnob>(Vec(30, 150), module, RPJMMSVFilter::PARAM_DAMP));
		addInput(createInput<PJ301MPort>(Vec(10, 180), module, RPJMMSVFilter::DAMP_IN));
	}

};

Model * modelVCF = createModel<RPJMMSVFilter, RPJMMSVFilterModuleWidget>("LadyNina");