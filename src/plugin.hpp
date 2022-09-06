#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

struct MLED : public SvgSwitch {
  MLED() {
    momentary=false;
    //latch=true;
    addFrame(Svg::load(asset::plugin(pluginInstance,"res/RButton0.svg")));
    addFrame(Svg::load(asset::plugin(pluginInstance,"res/RButton1.svg")));
  }
};
struct MLEDM : public SvgSwitch {
  MLEDM() {
    momentary=true;
    //latch=true;
    addFrame(Svg::load(asset::plugin(pluginInstance,"res/RButton0.svg")));
    addFrame(Svg::load(asset::plugin(pluginInstance,"res/RButton1.svg")));
  }
};
struct TrimbotWhite : SvgKnob {
  TrimbotWhite() {
    minAngle=-0.8*M_PI;
    maxAngle=0.8*M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/TrimpotWhite.svg")));
  }
};

struct SmallPort : app::SvgPort {
  SmallPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/SmallPort.svg")));
  }
};
#define MHEIGHT 128.5f