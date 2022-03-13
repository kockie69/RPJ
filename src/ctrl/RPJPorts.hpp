#include "rack.hpp"

struct RPJPJ301MPort : app::SvgPort {
	RPJPJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301M_Blue.svg")));
	}
};