#include "RPJ.hpp"
#include "Montreal.hpp"
#include "ctrl/RPJKnobs.hpp"
#include "ctrl/RPJPorts.hpp"

Montreal::Montreal() {
	const float minFreq = (std::log2(dsp::FREQ_C4 / 20480.f) + 5) / 10;
	const float maxFreq = (std::log2(20480.f / dsp::FREQ_C4) + 5) / 10;
	const float defaultFreq = (0.f + 5) / 10;
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam(PARAM_FC, minFreq, maxFreq, defaultFreq, "fc"," Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
	configParam(PARAM_CVFC, -1.f, 1.0f, 0.0f, "Cutoff frequency CV", "%", 0.f, 100.f);
	configParam(PARAM_Q, 0.707f, 20.0f, 0.707f, "Q");
	configParam(PARAM_CVQ, -1.f, 1.0f, 0.0f, "CV Q");
	configInput(INPUT_MAIN,"Main");
	configInput(INPUT_CVFC, "Cutoff CV");
	configInput(INPUT_CVQ, "Quality CV");
	configOutput(OUTPUT_LPF, "Low Pass Filter");
	configOutput(OUTPUT_HPF, "High Pass Filter");
	configOutput(OUTPUT_BPF, "Band Pass Filter");
	configOutput(OUTPUT_BSF, "Band Stop Filter");
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

void Montreal::processChannel(int c, Input& in, Output& lpfOut, Output& hpfOut, Output& bpfOut, Output& bsfOut) {

	simd::float_4 v = in.getPolyVoltageSimd<simd::float_4>(c);

	if (lpfOut.isConnected()) {
		wdfIdealRLCLPF[c/4].setParameters(wdfp);
		lpfOut.setVoltageSimd(simd::clamp(wdfIdealRLCLPF[c/4].processAudioSample(v),-5.f,5.f),c);
	}

	if (hpfOut.isConnected()) {
		wdfIdealRLCHPF[c/4].setParameters(wdfp);
		hpfOut.setVoltageSimd(simd::clamp(wdfIdealRLCHPF[c/4].processAudioSample(v),-5.f,5.f),c);
	}

	if (bpfOut.isConnected()) {
		wdfIdealRLCBPF[c/4].setParameters(wdfp);
		bpfOut.setVoltageSimd(simd::clamp(wdfIdealRLCBPF[c/4].processAudioSample(v),-5.f,5.f),c);
	}

	if (bsfOut.isConnected()) {
		wdfIdealRLCBSF[c/4].setParameters(wdfp);
		bsfOut.setVoltageSimd(simd::clamp(wdfIdealRLCBSF[c/4].processAudioSample(v),-5.f,5.f),c);
	}
}

void Montreal::process(const ProcessArgs &args) {

	if ((outputs[OUTPUT_LPF].isConnected() || 
		outputs[OUTPUT_HPF].isConnected() || outputs[OUTPUT_BPF].isConnected() || 
		outputs[OUTPUT_BSF].isConnected()) && inputs[INPUT_MAIN].isConnected()) {
		
		int channels = std::max(inputs[INPUT_MAIN].getChannels(), 1);
		outputs[OUTPUT_LPF].setChannels(channels);
		outputs[OUTPUT_HPF].setChannels(channels);
		outputs[OUTPUT_BPF].setChannels(channels);
		outputs[OUTPUT_BSF].setChannels(channels);

		for (int c = 0; c < channels; c += 4) {
			if (params[PARAM_FC].getValue() != wdfp.fc || params[PARAM_Q].getValue() !=  wdfp.Q)  {
				
				float freqParam = params[PARAM_FC].getValue();
				// Rescale for backward compatibility
				freqParam = freqParam * 10.f - 5.f;
				float freqCvParam = params[PARAM_CVFC].getValue();
				// Get pitch
				simd::float_4 pitch = freqParam + inputs[INPUT_CVFC].getPolyVoltageSimd<simd::float_4>(c) * freqCvParam;
				// Set cutoff
				simd::float_4 cutoff = dsp::FREQ_C4 * simd::pow(2.f, pitch);

				cutoff = clamp(cutoff, 20.f, args.sampleRate * 0.46f);
				wdfp.fc=cutoff.v[0];

				double cvq = 0.f;			
				if (inputs[INPUT_CVQ].isConnected())
					cvq = inputs[INPUT_CVQ].getVoltage()/10.f;

				wdfp.Q=clamp((params[PARAM_CVQ].getValue() * cvq * 20.f) + params[PARAM_Q].getValue(),0.707f, 20.0f);

				processChannel(c,inputs[INPUT_MAIN],outputs[OUTPUT_LPF],outputs[OUTPUT_HPF],outputs[OUTPUT_BPF],outputs[OUTPUT_BSF]);
			}
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

		// First do the knobs
		const float knobX1 = 11.4;
		const float knobX2 = 36;
		const float knobX3 = 47;
		const float knobX4 = 88.2;

		const float knobY1 = 27.5;
		const float knobY2 = 98;
		const float knobY3 = 205;

		addParam(createParam<RPJKnobBig>(Vec(knobX2, knobY1), module, Montreal::PARAM_FC));
		addParam(createParam<RPJKnobSmall>(Vec(knobX1, knobY3), module, Montreal::PARAM_CVFC));
		addParam(createParam<RPJKnob>(Vec(knobX3, knobY2), module, Montreal::PARAM_Q));
		addParam(createParam<RPJKnobSmall>(Vec(knobX4, knobY3), module, Montreal::PARAM_CVQ));

		// Next do the Jacks
		const float jackX1 = 5.5;
		const float jackX2 = 10;		
		const float jackX3 = 34;
		const float jackX4 = 49;
		const float jackX5 = 63;
		const float jackX6 = 87;
		const float jackX7 = 92;

		const float jackY1 = 233;
		const float jackY2 = 272;
		const float jackY3 = 311;

		addInput(createInput<RPJPJ301MPort>(Vec(jackX2, jackY1), module, Montreal::INPUT_CVFC));
		addInput(createInput<RPJPJ301MPort>(Vec(jackX6, jackY1), module, Montreal::INPUT_CVQ));
		addInput(createInput<RPJPJ301MPort>(Vec(jackX4, jackY2), module, Montreal::INPUT_MAIN));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX1, jackY3), module, Montreal::OUTPUT_LPF));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX3, jackY3), module, Montreal::OUTPUT_HPF));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX5, jackY3), module, Montreal::OUTPUT_BPF));
		addOutput(createOutput<RPJPJ301MPort>(Vec(jackX7, jackY3), module, Montreal::OUTPUT_BSF));
	}

};

Model * modelMontreal = createModel<Montreal, MontrealModuleWidget>("Montreal");