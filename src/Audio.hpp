#include "rack.hpp"
#include "dr_wav.h"

using namespace std;

enum PanningType {SIMPLEPAN, CONSTPOWER };

struct PanPos {
	double left;
	double right;
};

struct AudioParameters {
    AudioParameters();
    float dB;
	PanningType panningType;
    float panningValue;  
    float rackSampleRate;
    bool stop;
};

struct Audio {
    Audio();
    void setParameters(const AudioParameters& );
  	void loadSample(char *);
    PanPos panning(PanningType, double);
    void processAudioSample();
    vector<vector<float>> playBuffer;
    float peak;
    float scaleFac;
    float * pSampleData;
	float samplePos;
    PanningType panningType;
    unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalPCMFrameCount;
    char * fileName;
    bool start=false,stop=false,loading=false,fileLoaded=false,play=false;
    float left,right;
    float dB;
    float panningValue; 
    float rackSampleRate;
};

