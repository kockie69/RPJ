#include "rack.hpp"

struct VuMeterAllDual {
    
    void reset(std::vector<Light> lights) {

	    for (int i=0;i<8;i++)
            lights[i].setBrightness(0.f);
    }

    void process(std::vector<Light> &lights, float left, float right) {    
        lights[0].value = (left >= 0.7) ? .8f : .2f;
        lights[1].value = (left >= 0.35) ? .8f : .2f;
        lights[2].value = (left >= 0.175) ? .8f : .2f;
        lights[3].value = (left >= 0.087) ? .8f : .2f;
    }
};