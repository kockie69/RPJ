#include "RPJ.hpp"
#include "BridgeIn.hpp"


BridgeIn::BridgeIn() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	id=0;
	connected = 0;
}

int BridgeIn::getId() {

	std::vector<int> v;
	json_t *rootJ;
	auto rack = APP->scene->rack;

    for (::rack::widget::Widget* w2 : rack->getModuleContainer()->children) {		
        modwid = dynamic_cast<ModuleWidget*>(w2);
        if (modwid) {
            Model* model = modwid->model;
            if (model->slug == "BridgeIn") {
				if(modwid->getModule()) {
					rootJ= modwid->getModule()->dataToJson();
					json_t *nIdJ = json_object_get(rootJ, JSON_IN_ID);
					if (nIdJ) 
						v.push_back(json_integer_value(nIdJ));			
				}
			}
		}
	}
	std::sort(v.begin(),v.end());
	for (std::vector<int>::iterator it=v.begin(); it!=v.end(); ++it) {
		if (id < *it)
			break;
		id++;
	}
	return id;
}

void BridgeIn::onRemove(const RemoveEvent & e) {
	json_t *rootJ;

	if (id != 0) {
		if (modwid) {
			if (modwid->getModule()) {
				rootJ= modwid->getModule()->dataToJson();
				json_t *nConnJ = json_object_get(rootJ, JSON_IN_CONNECTED);
				if (nConnJ) {
					int nrConns = json_integer_value(nConnJ) - 1;
					json_object_set(rootJ, JSON_IN_CONNECTED, json_integer(nrConns));
					modwid->getModule()->dataFromJson(rootJ);
				}
			}
		}
	}
	Module::onRemove(e);
}

void BridgeIn::findDestination() {
	json_t *rootJ;
	auto rack = APP->scene->rack;
	connected = false;

    for (::rack::widget::Widget* w2 : rack->getModuleContainer()->children) {		
        modwid = dynamic_cast<ModuleWidget*>(w2);
        if (modwid) {
            Model* model = modwid->model;
            if (model->slug == "BridgeOut") {
				if(modwid->getModule()) {
					rootJ= modwid->getModule()->dataToJson();
					json_t *nIdJ = json_object_get(rootJ, JSON_IN_ID);
					if (nIdJ) {
						if (id == (json_integer_value(nIdJ)))
							connected += 1;			
					}
				}
			}
		}
	}
}

void BridgeIn::process(const ProcessArgs &args) {
	if (id==0) {
		id = getId();
		findDestination();
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

struct BridgeInModuleWidget : ModuleWidget {
	BridgeInModuleWidget(BridgeIn* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BridgeIn.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			
		{
			BridgeInDisplay * idd = new BridgeInDisplay(Vec(21,315));
			idd->module = module;
			addChild(idd);
		}
		addInput(createInput<PJ301MPort>(Vec(10, 55), module, BridgeIn::POLYINPUT_A));
		addInput(createInput<PJ301MPort>(Vec(10, 90), module, BridgeIn::POLYINPUT_B));
		addInput(createInput<PJ301MPort>(Vec(10, 125), module, BridgeIn::POLYINPUT_C));
		addInput(createInput<PJ301MPort>(Vec(10, 160), module, BridgeIn::POLYINPUT_D));
		addInput(createInput<PJ301MPort>(Vec(10, 195), module, BridgeIn::POLYINPUT_E));
		addInput(createInput<PJ301MPort>(Vec(10, 230), module, BridgeIn::POLYINPUT_F));
		addInput(createInput<PJ301MPort>(Vec(10, 265), module, BridgeIn::POLYINPUT_G));
		addInput(createInput<PJ301MPort>(Vec(10, 300), module, BridgeIn::POLYINPUT_H));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(Vec(17, 30), module, BridgeIn::RGB_LIGHT));
	}
};

json_t *BridgeIn::dataToJson() {
	json_t *rootJ=json_object();
	json_object_set_new(rootJ, JSON_IN_ID, json_integer(id));
	json_object_set_new(rootJ, JSON_IN_CONNECTED, json_integer(connected));
	return rootJ;
}

void BridgeIn::dataFromJson(json_t *rootJ) {
	json_t *nIdJ = json_object_get(rootJ, JSON_IN_ID);
	json_t *nConnJ = json_object_get(rootJ, JSON_IN_CONNECTED);
	if (nIdJ) {
		id = (json_integer_value(nIdJ));
	}
	if (nConnJ) {
		connected = (json_integer_value(nConnJ));
	}
}

Model * modelBridgeIn = createModel<BridgeIn, BridgeInModuleWidget>("BridgeIn");