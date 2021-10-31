#include "RPJ.hpp"
#include "Montreal.hpp"
#include "ctrl/RPJKnobs.hpp"


Montreal::Montreal() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, 0.0909f, 1.f, 0.5f, "Frequency", " Hz", 2048, 10);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	fc = 1000;
	configBypass(INPUT_MAIN, OUTPUT_LPF);
	configBypass(INPUT_MAIN, OUTPUT_HPF);
	configBypass(INPUT_MAIN, OUTPUT_BPF);
	configBypass(INPUT_MAIN, OUTPUT_BSF);
	for (int i = 0;i<4;i++) {
		wdfIdealRLCLPF[i].reset(APP->engine->getSampleRate());
		wdfIdealRLCHPF[i].reset(APP->engine->getSampleRate());
		wdfIdealRLCBPF[i].reset(APP->engine->getSampleRate());
		wdfIdealRLCBSF[i].reset(APP->engine->getSampleRate());
	}
	wdfp.fc=1000;
}

void Montreal::onSampleRateChange() {
	for (int i = 0;i<4;i++) {
		wdfIdealRLCLPF[i].reset(APP->engine->getSampleRate());
		wdfIdealRLCHPF[i].reset(APP->engine->getSampleRate());
		wdfIdealRLCBPF[i].reset(APP->engine->getSampleRate());
		wdfIdealRLCBSF[i].reset(APP->engine->getSampleRate());
	}
}

void Montreal::processChannel(Input& in, Output& lpfOut, Output& hpfOut, Output& bpfOut, Output& bsfOut) {
		
	// Get input
	int channels = std::max(in.getChannels(), 1);
	simd::float_4 v[4];
	for (int c = 0; c < channels; c += 4) {
		v[c/4] = simd::float_4::load(in.getVoltages(c));
	}
		

	simd::float_4 output;
	if (outputs[OUTPUT_LPF].isConnected()) {
		lpfOut.setChannels(channels);
		for (int c = 0; c < channels; c += 4) {
			output = wdfIdealRLCLPF[c/4].processAudioSample(v[c/4]);
			output.store(lpfOut.getVoltages(c));
		}
	}
	if (outputs[OUTPUT_HPF].isConnected()) {
		hpfOut.setChannels(channels);
		for (int c = 0; c < channels; c += 4) {
			output = wdfIdealRLCHPF[c/4].processAudioSample(v[c/4]);
			output.store(hpfOut.getVoltages(c));
		}
	}
	if (outputs[OUTPUT_BPF].isConnected()) {
		bpfOut.setChannels(channels);
		for (int c = 0; c < channels; c += 4) {
			output = wdfIdealRLCBPF[c/4].processAudioSample(v[c/4]);
			output.store(bpfOut.getVoltages(c));
		}
	}
	if (outputs[OUTPUT_BSF].isConnected()) {
		bsfOut.setChannels(channels);
		for (int c = 0; c < channels; c += 4) {
			output = wdfIdealRLCBSF[c/4].processAudioSample(v[c/4]);
			output.store(bsfOut.getVoltages(c));
		}
	}
}

void Montreal::process(const ProcessArgs &args) {

	if ((outputs[OUTPUT_LPF].isConnected() || 
		outputs[OUTPUT_HPF].isConnected() || outputs[OUTPUT_BPF].isConnected() || 
		outputs[OUTPUT_BSF].isConnected()) && inputs[INPUT_MAIN].isConnected()) {
		
		if (params[PARAM_FC].getValue() != wdfp.fc || params[PARAM_Q].getValue() !=  wdfp.Q)  {
			wdfp.fc=pow(2048,params[PARAM_FC].getValue()) * 10;
			wdfp.Q=params[PARAM_Q].getValue();
			for (int i = 0;i < 4;i++) { 
				wdfIdealRLCLPF[i].setParameters(wdfp);
				wdfIdealRLCHPF[i].setParameters(wdfp);
				wdfIdealRLCBPF[i].setParameters(wdfp);
				wdfIdealRLCBSF[i].setParameters(wdfp);
			}
			processChannel(inputs[INPUT_MAIN],outputs[OUTPUT_LPF],outputs[OUTPUT_HPF],outputs[OUTPUT_BPF],outputs[OUTPUT_BSF]);
		}
	}
}

struct MontrealModuleWidget : ModuleWidget {
	MontrealModuleWidget(Montreal* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Montreal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		addInput(createInput<PJ301MPort>(Vec(10, 240), module, Montreal::INPUT_MAIN));
		addOutput(createOutput<PJ301MPort>(Vec(55, 240), module, Montreal::OUTPUT_LPF));
		addOutput(createOutput<PJ301MPort>(Vec(55, 280), module, Montreal::OUTPUT_HPF));
		addOutput(createOutput<PJ301MPort>(Vec(10, 320), module, Montreal::OUTPUT_BPF));
		addOutput(createOutput<PJ301MPort>(Vec(55, 320), module, Montreal::OUTPUT_BSF));

		addParam(createParam<RPJKnob>(Vec(30, 60), module, Montreal::PARAM_FC));
		addParam(createParam<RPJKnob>(Vec(30, 140), module, Montreal::PARAM_Q));
	}

};

Model * modelMontreal = createModel<Montreal, MontrealModuleWidget>("Montreal");