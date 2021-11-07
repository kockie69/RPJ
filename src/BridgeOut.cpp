#include "RPJ.hpp"
#include "BridgeOut.hpp"

BridgeOut::BridgeOut() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	connected = false;
	id = 0;
	menuId = 0;
	modwid = NULL;
}

void BridgeOut::findSource() {
	json_t *rootJ;
	auto rack = APP->scene->rack;
    for (::rack::widget::Widget* w2 : rack->getModuleContainer()->children) {		
        modwid = dynamic_cast<ModuleWidget*>(w2);
        if (modwid) {
            Model* model = modwid->model;
            if (model->slug == "BridgeIn") {
				if(modwid->getModule()) {
					rootJ= modwid->getModule()->dataToJson();
					json_t *nIdJ = json_object_get(rootJ, JSON_OUT_ID);
					if (nIdJ) {
						if (id == (json_integer_value(nIdJ))) {
							// Led should go green
							connected=true;
							break;						
						}
					}
				}
			}
		}
	}
	if (connected) {
		lights[RGB_LIGHT + 0].setBrightness(0.f);
		lights[RGB_LIGHT + 1].setBrightness(1.f);
		lights[RGB_LIGHT + 2].setBrightness(0.f);
	}
	else {
		lights[RGB_LIGHT + 0].setBrightness(1.f);
		lights[RGB_LIGHT + 1].setBrightness(0.f);
		lights[RGB_LIGHT + 2].setBrightness(0.f);
	}
}

void BridgeOut::process(const ProcessArgs &args) {
	simd::float_4 output;
	int channels = 16;

	if (id != 0) {
		if (modwid) {
			std::vector<PortWidget *> inputList = modwid->getInputs();
			for (int i=0;i<8;i++) {
				if (inputList[i]->getPort()->isConnected() && outputs[i].isConnected()) {
					outputs[i].setChannels(channels);
					for (int c=0;c<channels;c+=4) {
						output = simd::float_4::load(inputList[i]->getPort()->getVoltages(c));
						output.store(outputs[i].getVoltages(c));
					}
				}
			}			
		}
		findSource();
	}
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
		addChild(createLight<MediumLight<RedGreenBlueLight>>(Vec(17, 330), module, BridgeOut::RGB_LIGHT));
	}

	void appendContextMenu(Menu *menu) override {
		BridgeOut *module = dynamic_cast<BridgeOut*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createIndexPtrSubmenuItem("Bridge ID", {"", "1", "2", "3", "4", "5", "6", "7"}, &module->id));
	}
};

json_t *BridgeOut::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_OUT_ID, json_integer(id));
	return rootJ;
}

void BridgeOut::dataFromJson(json_t *rootJ) {
	json_t *nIdJ = json_object_get(rootJ, JSON_OUT_ID);
	if (nIdJ) {
	//	id = (json_integer_value(nIdJ));
	}
}

Model * modelBridgeOut = createModel<BridgeOut, BridgeOutModuleWidget>("BridgeOut");