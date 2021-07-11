#include "RPJ.hpp"
Plugin *pluginInstance;
void init(Plugin *p) {
    pluginInstance = p;
    p->addModel(modelLFO);
    p->addModel(modelLadyNina);
    p->addModel(modelLavender);
    p->addModel(modelEaster);
}

