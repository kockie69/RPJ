#include "RPJ.hpp"
#include "BridgeOut.hpp"

BridgeOut::BridgeOut() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	id=2;
}

void BridgeOut::processChannel(Output& out) {
	
}

void BridgeOut::process(const ProcessArgs &args) {

	simd::float_4 output;
	int channels = 16;
	json_t *rootJ;

	auto rack = APP->scene->rack;
    for (::rack::widget::Widget* w2 : rack->getModuleContainer()->children) {		
        ModuleWidget* modwid = dynamic_cast<ModuleWidget*>(w2);
        if (modwid) {
            Model* model = modwid->model;
            if (model->slug == "BridgeIn") {
				if(modwid->getModule()) {
					rootJ= modwid->getModule()->dataToJson();
					json_t *nIdJ = json_object_get(rootJ, JSON_OUT_ID);
					if (nIdJ) {
						if (id == (json_integer_value(nIdJ))) {
							for (int i=0;i<8;i++) {
								std::vector<PortWidget *> inputList = modwid->getInputs();
								outputs[i].setChannels(channels);
								for (int c=0;c<channels;c+=4) {
									output = simd::float_4::load(inputList[i]->getPort()->getVoltages(c));
									output.store(outputs[i].getVoltages(c));
								}
							}
						}
					}
				}
			}
		}
	}

	// if inputBridge(id) is still there
	// for (int i=0; i<8;i++)
	//     for (int c=0;c<inputBridge(id).numChannels;c+=4)
	// 			output[c/4].voltage=inputBridge(id)[c/4].voltage
}

struct BridgeOutModuleWidget : ModuleWidget {
	BridgeOutModuleWidget(BridgeOut* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BridgeOut.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			

		addOutput(createOutput<PJ301MPort>(Vec(10, 55), module, BridgeOut::POLYOUTPUT_A));
		addOutput(createOutput<PJ301MPort>(Vec(10, 90), module, BridgeOut::POLYOUTPUT_B));
		addOutput(createOutput<PJ301MPort>(Vec(10, 125), module, BridgeOut::POLYOUTPUT_C));
		addOutput(createOutput<PJ301MPort>(Vec(10, 160), module, BridgeOut::POLYOUTPUT_D));
		addOutput(createOutput<PJ301MPort>(Vec(10, 195), module, BridgeOut::POLYOUTPUT_E));
		addOutput(createOutput<PJ301MPort>(Vec(10, 230), module, BridgeOut::POLYOUTPUT_F));
		addOutput(createOutput<PJ301MPort>(Vec(10, 265), module, BridgeOut::POLYOUTPUT_G));
		addOutput(createOutput<PJ301MPort>(Vec(10, 300), module, BridgeOut::POLYOUTPUT_H));
	}
};

Model * modelBridgeOut = createModel<BridgeOut, BridgeOutModuleWidget>("BridgeOut");