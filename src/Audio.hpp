#include "rack.hpp"
#include "dr_wav.h"
#include "dr_flac.h"
#include "dr_mp3.h"

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
    bool pause;
    bool play;
    bool stop;
    float startRatio,endRatio;
    bool repeat;
    float speed;
};

struct Audio {
    Audio();
    void setParameters(const AudioParameters& );
  	bool loadSample(char *);
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
    drmp3_config mp3config;
    char * fileName;
    bool start=false,pause=false,loading=false,fileLoaded=false,play=false,stop=false;
    float left,right;
    float dB;
    float speed;
    float panningValue; 
    float rackSampleRate;
    float beginRatio, endRatio;
    bool up,repeat;
};

