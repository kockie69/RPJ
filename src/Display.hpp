#include "rack.hpp"

using namespace rack;
using namespace std;

struct Display : TransparentWidget {
	Display(int);
	void setDisplayFont(Plugin *,std::string);
	void setDisplayWidth(int); 
    void setDisplayBuff(float, float, vector<vector<float>>);
    void setBegin(float);
	void setEnd(float);
	void setDisplayPos(float, float, float);
	void drawLayer(const DrawArgs &,int layer) override; 
	int frame;
	shared_ptr<Font> font;
	std::string fontPath;
    vector<double> displayBuff;
    float displayBegin;
	float displayEnd;
	float width;
	std::string fileDesc;
	float displayPos;
	float totalPCMFrameCount;
	int zoom;
	int start;
	float zoomDelta;
};