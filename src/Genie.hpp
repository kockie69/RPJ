#include "rack.hpp"

using namespace rack;

const int MODULE_WIDTH=19;
const int WIDTH=250;
const int HEIGHT=250;
const int EDGES=2;
const int MAXOBJECTS = 4;

enum GenieAlgorythms {
    PENDULUM,
    BUMPINGBALLS,
};

struct xpanderPairs {
	std::pair<double, double> edges[MAXOBJECTS][EDGES];
    int nrOfItems;
    GenieAlgorythms genieAlgorythm;
};

namespace dp {
	double g  = 9.81;
    double dt;

    struct state {
        std::pair<double, double> theta;
        std::pair<double, double> omega;
    };

    std::pair<double, double> operator+(const std::pair<double, double>& p1, const std::pair<double, double>& p2) noexcept {
        return {
            p1.first  + p2.first,
            p1.second + p2.second
        };
    }

    std::pair<double, double> operator*(const double d, const std::pair<double, double>& p) noexcept {
        return {
            d * p.first,
            d * p.second
        };
    }

    state operator+(const state& s1, const state& s2) noexcept {
        return {
            s1.theta + s2.theta,
            s1.omega + s2.omega
        };
    }

    state operator*(const double d, const state& s) noexcept {
        return {
            d * s.theta,
            d * s.omega
        };
    }

    struct system {
        std::pair<double, double> mass;
        std::pair<double, double> length;
    };

    state derive(const state& st, const system& ss) noexcept {
        const double delta = st.theta.second - st.theta.first;
        const double mass  = ss.mass.first + ss.mass.second;

        double s = sin(delta);
        double c = cos(delta);

        double denominator = mass * ss.length.first - 
            ss.mass.second * ss.length.first * c * c;

        state derivative{{st.omega.first, st.omega.second}, {0, 0}};

        derivative.omega.first 
            = ss.mass.second * ss.length.first * st.omega.first * st.omega.first
                * s * c
            + ss.mass.second * g * sin(st.theta.second) * c
            + ss.mass.second * ss.length.second * st.omega.second 
                * st.omega.second * s
            - mass * g * sin(st.theta.first);

        derivative.omega.first /= denominator;

        denominator *= ss.length.second / ss.length.first;           

        derivative.omega.second
            = - ss.mass.second * ss.length.second * st.omega.second
                * st.omega.second * s * c
            + mass * g * sin(st.theta.first) * c
            - mass * ss.length.first * st.omega.first * st.omega.first * s
            - mass * g * sin(st.theta.second);

        derivative.omega.second /= denominator;

        return derivative;
    }

    state rk4(const state& st, const system& ss) noexcept {
        state dydx = derive(st, ss);
        state k1   = dt * dydx;
        state yt   = st + 0.5 * k1;
        
              dydx = derive(yt, ss);
        state k2   = dt * dydx;
              yt   = st + 0.5 * k2;

              dydx = derive(yt, ss);
        state k3   = dt * dydx;
              yt   = st + k3;

              dydx = derive(yt, ss);
        state k4   = dt * dydx;
        
        return 
            st + (1.0 / 6) * k1 + (1.0 / 3) * k2 + 
                 (1.0 / 3) * k3 + (1.0 / 6) * k4;
    }

    state advance(const state& st, const system& ss, double sampleTime) noexcept {
        dt = sampleTime;
        state ret = rk4(st, ss);
        return ret;
    }
}

struct ball {
    private:
		std::pair<double,double> position;
    	int size;
		NVGcolor massColor;
	public:
    	void draw(NVGcontext *vg);
    	void setSize(int size);
    	void setPosition(std::pair<double,double> position);
		std::pair<double,double> getPosition(void);
    	void setColor(NVGcolor massColor);
};

struct Genie : Module {

	enum ParamIds {
        PARAM_TIMEMULT,
        PARAM_TIMEMULTCV,
        PARAM_LENGTHMULT,
        PARAM_LENGTHMULTCV,
        PARAM_MASS,
        PARAM_MASSCV,
        PARAM_RESET,
        PARAM_UNI,
		NUM_PARAMS,
	};

	enum InputIds {
        INPUT_TIMEMULTCV,
        INPUT_LENGTHMULTCV,
        INPUT_MASSCV,
		NUM_INPUTS,
	};

	enum OutputIds {
		OUTPUT_1_X,
        OUTPUT_1_Y,
        OUTPUT_2_X,
        OUTPUT_2_Y,
        OUTPUT_3_X,
        OUTPUT_3_Y,
        OUTPUT_4_X,
        OUTPUT_4_Y,
        OUTPUT_1_EDGE,
        OUTPUT_2_EDGE,
        OUTPUT_3_EDGE,
        OUTPUT_4_EDGE,
		NUM_OUTPUTS,
	};

	enum LightIds {
		NUM_LIGHTS
	};


	Genie();
	void process(const ProcessArgs &) override;
    void reset(void);
    json_t *dataToJson() override;
	void dataFromJson(json_t *) override;
    void doPendulum(const ProcessArgs &);
    void doBumpingBalls(const ProcessArgs &);
    GenieAlgorythms genieAlgorythm;
    std::pair<int, int> dim;

    std::pair<double, double> edges[MAXOBJECTS][EDGES];
    dp::state st[MAXOBJECTS];
    dp::system ss[MAXOBJECTS];
    double timeMult;
    double lengthMult;
    double len;
    double mass;
    ball bumpingBalls[MAXOBJECTS];
    xpanderPairs xpMsg[2];
    int nrOfPendulums;
    dsp::SchmittTrigger resetTrigger;
    bool uni;
};

struct GenieModuleWidget : ModuleWidget {
	GenieModuleWidget(Genie*);
    void appendContextMenu(Menu *) override;
};

