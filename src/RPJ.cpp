#include "RPJ.hpp"
Plugin *pluginInstance;
void init(Plugin *p) {
    pluginInstance = p;
    p->addModel(modelLFO);
    p->addModel(modelLavender);
    p->addModel(modelEaster);
    p->addModel(modelDryLand);
    p->addModel(modelTheWeb);
    p->addModel(modelGazpacho);    
    p->addModel(modelEstonia);
    p->addModel(modelBrave);  
    p->addModel(modelEssence);
    p->addModel(modelLadyNina);
    p->addModel(modelSugarMice);
    p->addModel(modelMontreal);
    p->addModel(modelBlindCurve);
}

