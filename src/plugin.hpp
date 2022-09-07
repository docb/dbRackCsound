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

struct DBTextWidget : Widget {
  std::basic_string<char> fontPath;
  std::string label;
  DBTextWidget(const std::string &_label,Vec _pos,Vec _size) {
    box.pos=_pos;
    box.size=_size;
    label=_label;
    fontPath=asset::plugin(pluginInstance,"res/FreeMonoBold.ttf");
  }

  void drawLayer(const DrawArgs &args,int layer) override {
    if(layer==1) {
      _draw(args);
    }
    Widget::drawLayer(args,layer);
  }


  void _draw(const DrawArgs &args) {
    std::shared_ptr<Font> font=APP->window->loadFont(fontPath);
    nvgFillColor(args.vg,nvgRGB(255,255,128));
    nvgFontFaceId(args.vg,font->handle);
    nvgFontSize(args.vg,11);
    nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
    nvgText(args.vg,box.size.x/2,box.size.y/2,label.c_str(),NULL);
  }
};
#define MHEIGHT 128.5f