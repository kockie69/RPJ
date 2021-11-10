#include "RPJ.hpp"
#include "BridgeOut.hpp"
#include <regex>

BridgeOut::BridgeOut() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	connected = 0;
	id = 0;
	sourceId = 0;
	modwid = NULL;
}

void BridgeOut::remove() {
	json_t *rootJ;

	if (id != 0) {
		if (modwid) {
			if (modwid->getModule()) {
				rootJ= modwid->getModule()->dataToJson();
				json_t *nConnJ = json_object_get(rootJ, JSON_OUT_CONNECTED);
				if (nConnJ) {
					int nrConns = json_integer_value(nConnJ) - 1;
					json_object_set(rootJ, JSON_OUT_CONNECTED, json_integer(nrConns));
					modwid->getModule()->dataFromJson(rootJ);
				}
			}
		}
	}
}

void BridgeOut::onRemove(const RemoveEvent & e) {
	remove();
	Module::onRemove(e);
}

void BridgeOut::findSource() {
	json_t *rootJ;
	auto rack = APP->scene->rack;
	sourceId = 0;
	connected = 0;
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
							// Tell source I am connected
							connected=1;
							sourceId = modwid->getModule()->getId();
							json_t *nConnJ = json_object_get(rootJ, JSON_OUT_CONNECTED);
							int nrConns = 0;
							if (nConnJ) 
								nrConns = json_integer_value(nConnJ) + 1;
							json_object_set(rootJ, JSON_OUT_CONNECTED, json_integer(nrConns));
							modwid->getModule()->dataFromJson(rootJ);
							break;	
						}
					}
				}
			}
		}
	}
}

void BridgeOut::process(const ProcessArgs &args) {
	simd::float_4 output;
	int channels;

	if (id != 0) {
		if (oldId==id) {
			if (sourceId) {
				if (modwid->getModule()) {
					std::vector<PortWidget *> inputList = modwid->getInputs();
					for (int i=0;i<8;i++) {
						if (inputList[i]->getPort()->isConnected() && outputs[i].isConnected()) {
							channels = std::max(inputList[i]->getPort()->getChannels(), 1);
							outputs[i].setChannels(channels);
							for (int c=0;c<channels;c+=4) {
								output = simd::float_4::load(inputList[i]->getPort()->getVoltages(c));
								output.store(outputs[i].getVoltages(c));
							}
						}
					}
				}
				else 
					findSource();
			}
			else {
				findSource();
			}
		}
		else { 
			findSource();
			oldId=id;
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

void nBridgeIdMenuItem::onSelectKey(const SelectKeyEvent & e) {
if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		// Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::max(cursor - 1, 0);
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Backspace
		if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToPrevWord();
			}
			insertText("");
			e.consume(this);
		}
		// Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == 0) {
			if (cursor == selection) {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			insertText("");
			e.consume(this);
		}
		// Ctrl+Delete
		if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			if (cursor == selection) {
				cursorToNextWord();
			}
			insertText("");
			e.consume(this);
		}
		// Left
		if (e.key == GLFW_KEY_LEFT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToPrevWord();
			}
			else {
				cursor = std::max(cursor - 1, 0);
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Right
		if (e.key == GLFW_KEY_RIGHT) {
			if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
				cursorToNextWord();
			}
			else {
				cursor = std::min(cursor + 1, (int) text.size());
			}
			if (!(e.mods & GLFW_MOD_SHIFT)) {
				selection = cursor;
			}
			e.consume(this);
		}
		// Up (placeholder)
		if (e.key == GLFW_KEY_UP) {
			e.consume(this);
		}
		// Down (placeholder)
		if (e.key == GLFW_KEY_DOWN) {
			e.consume(this);
		}
		// Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = 0;
			e.consume(this);
		}
		// Shift+Home
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = 0;
			e.consume(this);
		}
		// End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == 0) {
			selection = cursor = text.size();
			e.consume(this);
		}
		// Shift+End
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			cursor = text.size();
			e.consume(this);
		}
		// Ctrl+V
		if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			pasteClipboard();
			e.consume(this);
		}
		// Ctrl+X
		if (e.keyName == "x" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			cutClipboard();
			e.consume(this);
		}
		// Ctrl+C
		if (e.keyName == "c" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			copyClipboard();
			e.consume(this);
		}
		// Ctrl+A
		if (e.keyName == "a" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			selectAll();
			e.consume(this);
		}
		// Enter
		if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && (e.mods & RACK_MOD_MASK) == 0) {
			if (multiline) {
				insertText("\n");
			}
			else {
				ActionEvent eAction;
				onAction(eAction);
			}
			e.consume(this);
			if(regex_match(getText(), std::regex("[0-9]+"))) {
				if (stoi(getText()) >  0) {
					module->oldId = module->id;
					module->id = stoi(getText());
					module->remove();
					getAncestorOfType<ui::MenuOverlay>()->requestDelete();
				}
			}
		}
		// Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == 0) {
			if (nextField)
				APP->event->setSelectedWidget(nextField);
			e.consume(this);
		}
		// Shift-Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			if (prevField)
				APP->event->setSelectedWidget(prevField);
			e.consume(this);
		}
		// Consume all printable keys
		if ( e.keyName >= "0" && e.keyName <= "9") {
			e.consume(this);
		}

		assert(0 <= cursor);
		assert(cursor <= (int) text.size());
		assert(0 <= selection);
		assert(selection <= (int) text.size());
	}
}


struct BridgeOutModuleWidget : ModuleWidget {
	BridgeOutModuleWidget(BridgeOut* module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BridgeOut.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		box.size = Vec(MODULE_WIDTH*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);			
		{
			BridgeOutDisplay * idd = new BridgeOutDisplay(Vec(21,315));
			idd->module = module;
			addChild(idd);
		}
		addOutput(createOutput<PJ301MPort>(Vec(10, 55), module, BridgeOut::POLYOUTPUT_A));
		addOutput(createOutput<PJ301MPort>(Vec(10, 90), module, BridgeOut::POLYOUTPUT_B));
		addOutput(createOutput<PJ301MPort>(Vec(10, 125), module, BridgeOut::POLYOUTPUT_C));
		addOutput(createOutput<PJ301MPort>(Vec(10, 160), module, BridgeOut::POLYOUTPUT_D));
		addOutput(createOutput<PJ301MPort>(Vec(10, 195), module, BridgeOut::POLYOUTPUT_E));
		addOutput(createOutput<PJ301MPort>(Vec(10, 230), module, BridgeOut::POLYOUTPUT_F));
		addOutput(createOutput<PJ301MPort>(Vec(10, 265), module, BridgeOut::POLYOUTPUT_G));
		addOutput(createOutput<PJ301MPort>(Vec(10, 300), module, BridgeOut::POLYOUTPUT_H));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(Vec(17, 30), module, BridgeOut::RGB_LIGHT));
	}

	void appendContextMenu(Menu *menu) override {
		BridgeOut *module = dynamic_cast<BridgeOut*>(this->module);

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Bridge ID"));
		nBridgeIdMenuItem *nBridgeIdItem = new nBridgeIdMenuItem();
		nBridgeIdItem->box.size = Vec(120, 20);
		nBridgeIdItem->multiline = false;
		nBridgeIdItem->text = std::to_string(module->id);
		nBridgeIdItem->module = module;
		nBridgeIdItem->menu = menu;
		menu->addChild(nBridgeIdItem);
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
		id = (json_integer_value(nIdJ));
	}
}

Model * modelBridgeOut = createModel<BridgeOut, BridgeOutModuleWidget>("BridgeOut");