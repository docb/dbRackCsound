#include "plugin.hpp"
#include "DBCSound.hpp"
#include "textfield.hpp"

struct CsoundModule : Module {
	enum ParamId {
    COMPILE_BUTTON,KNOB_PARAM,PARAMS_LEN=KNOB_PARAM+4
	};
	enum InputId {
    GATE_INPUT,VOCT_INPUT,PHS_INPUT,IN1,IN2,IN3,IN4,INPUTS_LEN
	};
	enum OutputId {
    LEFT_OUTPUT,RIGHT_OUTPUT,OUTPUTS_LEN
	};
	enum LightId {
    ERROR_LIGHT,OK_LIGHT,LIGHTS_LEN
	};

  DBCsound cs;
  MYFLT *phs[16];
  MYFLT *vo[16];
  MYFLT *in1[16];
  MYFLT *in2[16];
  MYFLT *in3[16];
  MYFLT *in4[16];
  MYFLT *p[4];
  std::vector<std::string> events={"i 1.01 0 -1 1","i 1.02 0 -1 2","i 1.03 0 -1 3","i 1.04 0 -1 4","i 1.05 0 -1 5",
                                   "i 1.06 0 -1 6","i 1.07 0 -1 7","i 1.08 0 -1 8","i 1.09 0 -1 9","i 1.1 0 -1 10",
                                   "i 1.11 0 -1 11","i 1.12 0 -1 12","i 1.13 0 -1 13","i 1.14 0 -1 14","i 1.15 0 -1 15","i 1.16 0 -1 16"};

  int NCHNLS=1;

  std::string instrument="instr 1\n"
                         "kfreq chnget sprintf(\"FREQ%d\",p4)\n"
                         "ao vco2 0.7,kfreq\n"
                         "aenv madsr 0.001,0,1,0.02\n"
                         "ao*=aenv \n"
                         "outs ao,ao\n"
                         "endin\n";

  int compileError=1;
  bool on[16]={};
  float blinkPhase=0.f;
  bool dirty=false;
  int pos=0;
  int main=1;

	CsoundModule(int nch=1) {
    NCHNLS=nch;
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(COMPILE_BUTTON,"Compile");
    configInput(GATE_INPUT,"GATE");
    configInput(IN1,"IN 1");
    configInput(IN2,"IN 2");
    configInput(IN3,"IN 3");
    configInput(IN4,"IN 4");
    configInput(PHS_INPUT,"PHS");
    configInput(VOCT_INPUT,"V/O");
    if(NCHNLS==16) {
      configOutput(LEFT_OUTPUT,"Polyphonic Left");
      configOutput(RIGHT_OUTPUT,"Polyphonic Right");
    } else {
      configOutput(LEFT_OUTPUT,"Left");
      configOutput(RIGHT_OUTPUT,"Right");
    }
    for(int i=0;i<4;i++) {
      configParam(KNOB_PARAM+i,-10,10,0,"P"+std::to_string(i+1));
    }
    cs.initCsound(32,NCHNLS*2,APP->engine->getSampleRate());
    bindChannels();
    dirty=true;
    compile();
	}

  void bindChannels() {
    for(int i=0;i<16;i++) {
      std::string cname="PHS"+std::to_string(i+1);
      cs.bindAudioChannel(cname.c_str(),&phs[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="FREQ"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&vo[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="IN1C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in1[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="IN2C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in2[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="IN3C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in3[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="IN4C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in4[i]);
    }
    for(int i=0;i<4;i++) {
      std::string cname="P"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&p[i]);
    }
  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override {
    //cs.resetCsound(32,2,APP->engine->getSampleRate());
  }
  void compile() {
    std::string orc=instrument+"\n";
    compileError=cs.compile(orc);

  }

  void setText(std::string text) {
    instrument=text;
  }

  void setParameters() {
    bool voctConnected=inputs[VOCT_INPUT].isConnected();
    bool in1Connected=inputs[IN1].isConnected();
    bool in2Connected=inputs[IN2].isConnected();
    bool in3Connected=inputs[IN3].isConnected();
    bool in4Connected=inputs[IN4].isConnected();
    if(voctConnected) {
      int channels=inputs[VOCT_INPUT].getChannels();
      for(int chn=0;chn<channels;chn++) {
        float freq=dsp::FREQ_C4*dsp::approxExp2_taylor5(inputs[VOCT_INPUT].getVoltage(chn)+30.f)/std::pow(2.f,30.f);
        vo[chn][0]=freq;
      }
    }
    if(in1Connected) {
      int channels=inputs[IN1].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in1[chn][0]=inputs[IN1].getVoltage(chn);
      }
    }
    if(in2Connected) {
      int channels=inputs[IN2].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in2[chn][0]=inputs[IN2].getVoltage(chn);
      }
    }
    if(in3Connected) {
      int channels=inputs[IN3].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in3[chn][0]=inputs[IN3].getVoltage(chn);
      }
    }
    if(in4Connected) {
      int channels=inputs[IN4].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in4[chn][0]=inputs[IN4].getVoltage(chn);
      }
    }
    for(int i=0;i<4;i++) {
      p[i][0]=params[KNOB_PARAM+i].getValue();
    }
  }

	void process(const ProcessArgs& args) override {

    int channels=0;
    if(inputs[GATE_INPUT].isConnected()) {
      channels=inputs[GATE_INPUT].getChannels();
      for(int chn=0;chn<channels;chn++) {
        float g=inputs[GATE_INPUT].getVoltage(chn);
        if(g&&!on[chn]) {
          cs.readScoreEvent(events[chn].c_str());
          on[chn]=true;
        }
        if(g==0.f&&on[chn]) {
          cs.killInstr(1.0+(chn+1)*0.01);
          on[chn]=false;
        }
      }
    }
    if(compileError==0) {
      if(pos==0) {
        setParameters();
        cs.perform();
      }

      if(inputs[PHS_INPUT].isConnected()) {
        for(int k=0;k<16;k++) {
          phs[k][pos]=inputs[PHS_INPUT].getVoltage(k)*0.1f+0.5f;
        }
      } else {
        for(int k=0;k<16;k++) {
          phs[k][pos]=0;
        }
      }
      if(NCHNLS==16) {
        for(int k=0;k<channels;k++) {
          outputs[LEFT_OUTPUT].setVoltage(cs.out[pos*cs.getNChnls()+2*k]*5.f,k);
          outputs[RIGHT_OUTPUT].setVoltage(cs.out[pos*cs.getNChnls()+2*k+1]*5.f,k);
        }
      } else {
        outputs[LEFT_OUTPUT].setVoltage(cs.out[pos*2]*5.f);
        outputs[RIGHT_OUTPUT].setVoltage(cs.out[pos*2+1]*5.f);
      }
      pos++;
      if(pos>=cs.getKsmps()) {
        pos=0;
      }
    }

    blinkPhase+=args.sampleTime;
    if(blinkPhase>=1.0f)
      blinkPhase-=1.0f;
    if(compileError==0) {
      lights[OK_LIGHT].value=0;
      lights[ERROR_LIGHT].value=1;
    } else {
      lights[OK_LIGHT].value=(blinkPhase<0.5f)?1.0f:0.0f;
      lights[ERROR_LIGHT].value=0;
    }
    if(NCHNLS==16) {
      outputs[LEFT_OUTPUT].setChannels(channels);
      outputs[RIGHT_OUTPUT].setChannels(channels);
    }
	}

  json_t *dataToJson() override {
    json_t *rootJ=json_object();
    json_object_set_new(rootJ,"csd",json_string(instrument.c_str()));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *textJ=json_object_get(rootJ,"csd");
    if(textJ)
      instrument=json_string_value(textJ);
    compile();
    dirty=true;
  }

  void onReset(const ResetEvent& e) override {
    cs.resetCsound(32,NCHNLS*2,APP->engine->getSampleRate());
    bindChannels();
    compile();
  }

};

struct CsoundModule16 : CsoundModule {
  CsoundModule16() : CsoundModule(16) {}
};
template<typename M>
struct CSoundTextField : MTextField {
  M *module;

  CSoundTextField() : MTextField() {
    //bgColor=nvgRGB(0x30,0x30,0x30);
  }

  void setModule(M *module) {
    this->module=module;
  }

  void onChange(const ChangeEvent &e) override {
    if(module) {
      module->setText(getText());
    }
  }

  void draw(const DrawArgs &args) override {
    if(module&&(module->dirty)) {
      int cursorSave=cursor;
      setText(module->instrument);
      cursor=selection=cursorSave;
      if(cursor<0||cursor>(int)
        text.size()) selection=cursor=0;
      module->dirty=false;
    }
    MTextField::draw(args);
  }
};

struct CSTextScrollWidget : ScrollWidget {
  void onHover(const HoverEvent &e) override {
    ScrollWidget::onHover(e);
    if(!isScrolling()) {
      APP->event->setSelectedWidget(container->children.front());
    }
  }

};

template<typename M>
struct CompileButton : MLEDM {
  M *module;

  void onChange(const ChangeEvent &e) override {
    SvgSwitch::onChange(e);
    if(module) {
      if(module->params[CsoundModule::COMPILE_BUTTON].getValue()>0)
        module->compile();
    }

  }
};


struct CsoundModuleWidget : ModuleWidget {
	CsoundModuleWidget(CsoundModule* module,const std::string &svgFile="res/CsoundModule.svg") {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, svgFile)));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    //if(module && module->NCHNLS==16) {
    //  auto text = new DBTextWidget("***",mm2px(Vec(56.f,1.9f)),Vec(30.f,12.f));
    //  addChild(text);
    //}

    ScrollWidget *scrollWidget=new CSTextScrollWidget();
    scrollWidget->box.size=mm2px(Vec(85,68));
    scrollWidget->box.pos=mm2px(Vec(3.5f,MHEIGHT-54-68));
    addChild(scrollWidget);
    auto textField=createWidget<CSoundTextField<CsoundModule>>(Vec(0,0));
    textField->setModule(module);
    textField->box.size=mm2px(Vec(200,500));
    textField->multiline=true;
    textField->scroll=scrollWidget;
    scrollWidget->container->addChild(textField);

    float x=8;
    float y=80;
    auto gen=createParam<CompileButton<CsoundModule>>(mm2px(Vec(x,y)),module,CsoundModule::COMPILE_BUTTON);
    gen->module=module;
    addParam(gen);
    addChild(createLightCentered<LargeLight<GreenRedLight>>(mm2px(Vec(35,83.14f)),module,CsoundModule::ERROR_LIGHT));
    //addParam(createParam<MLED>(mm2px(Vec(x+24,y)),module,CsoundModule::ON_PARAM));

    y=96;
    addInput(createInput<SmallPort>(mm2px(Vec(x,y)),module,CsoundModule::IN1));
    addInput(createInput<SmallPort>(mm2px(Vec(x+10,y)),module,CsoundModule::IN2));
    addInput(createInput<SmallPort>(mm2px(Vec(x+20,y)),module,CsoundModule::IN3));
    addInput(createInput<SmallPort>(mm2px(Vec(x+30,y)),module,CsoundModule::IN4));
    for(int i=0;i<4;i++) {
      addParam(createParam<TrimbotWhite>(mm2px(Vec(x+40+i*10,y)),module,CsoundModule::KNOB_PARAM+i));
    }
    y=112;
    addInput(createInput<SmallPort>(mm2px(Vec(x,y)),module,CsoundModule::VOCT_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(x+15,y)),module,CsoundModule::GATE_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(x+30,y)),module,CsoundModule::PHS_INPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(x+60,y)),module,CsoundModule::LEFT_OUTPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(x+70,y)),module,CsoundModule::RIGHT_OUTPUT));

	}
};
struct CsoundModuleWidget16 : CsoundModuleWidget {
  CsoundModuleWidget16(CsoundModule* module) : CsoundModuleWidget(module,"res/CsoundModule16.svg") {
  }
};

struct CsoundModuleFX : Module {
  enum ParamId {
    COMPILE_BUTTON,KNOB_PARAM,ON_PARAM=KNOB_PARAM+4,PARAMS_LEN
  };
  enum InputId {
    LEFT_INPUT,RIGHT_INPUT,IN1,IN2,IN3,IN4,INPUTS_LEN
  };
  enum OutputId {
    LEFT_OUTPUT,RIGHT_OUTPUT,OUTPUTS_LEN
  };
  enum LightId {
    ERROR_LIGHT,OK_LIGHT,LIGHTS_LEN
  };

  DBCsound cs;
  MYFLT *in1[16];
  MYFLT *in2[16];
  MYFLT *in3[16];
  MYFLT *in4[16];
  MYFLT *p[4];
  int NCHNLS=1;
  std::vector<std::string> events={"i 1.01 0 -1 1","i 1.02 0 -1 2","i 1.03 0 -1 3","i 1.04 0 -1 4","i 1.05 0 -1 5",
                                   "i 1.06 0 -1 6","i 1.07 0 -1 7","i 1.08 0 -1 8","i 1.09 0 -1 9","i 1.1 0 -1 10",
                                   "i 1.11 0 -1 11","i 1.12 0 -1 12","i 1.13 0 -1 13","i 1.14 0 -1 14","i 1.15 0 -1 15","i 1.16 0 -1 16"};

  std::string instrument16="instr 1\n"
                          "  ainL inch p4*2-1\n"
                          "  ainR inch p4*2\n"
                          "  ; CODE .... \n"
                          "  aoutL = ainL\n"
                          "  aoutR = ainR\n"
                          "  outch p4*2-1,aoutL\n"
                          "  outch p4*2,aoutR\n"
                         "endin\n";
  std::string instrument1="instr 1\n"
                          "  ainL,ainR inch 1,2\n"
                          "  ; CODE .... \n"
                          "  aoutL = ainL\n"
                          "  aoutR = ainR\n"
                          " outs aoutL,aoutR\n"
                           "endin\n";
  std::string instrument;
  int compileError=1;
  float blinkPhase=0.f;
  bool dirty=false;
  int pos=0;
  bool op=false;


  CsoundModuleFX(int nch=1) {
    NCHNLS=nch;
    config(PARAMS_LEN,INPUTS_LEN,OUTPUTS_LEN,LIGHTS_LEN);
    configButton(COMPILE_BUTTON,"Compile");
    configButton(ON_PARAM,"ON");
    configInput(IN1,"IN 1");
    configInput(IN2,"IN 2");
    configInput(IN3,"IN 3");
    configInput(IN4,"IN 4");
    configInput(LEFT_INPUT,"Left");
    configInput(RIGHT_INPUT,"Right");
    if(NCHNLS==16) {
      configInput(LEFT_INPUT,"Polyphonic Left");
      configInput(RIGHT_INPUT,"Polyphonic Right");
      configOutput(LEFT_OUTPUT,"Polyphonic Left");
      configOutput(RIGHT_OUTPUT,"Polyphonic Right");
      instrument=instrument16;
    } else {
      configOutput(LEFT_OUTPUT,"Left");
      configOutput(RIGHT_OUTPUT,"Right");
      configInput(LEFT_INPUT,"Left");
      configInput(RIGHT_INPUT,"Right");
      instrument=instrument1;
    }
    for(int i=0;i<4;i++) {
      configParam(KNOB_PARAM+i,-10,10,0,"P"+std::to_string(i+1));
    }
    cs.initCsound(32,NCHNLS*2,APP->engine->getSampleRate());
    bindChannels();
    dirty=true;
    compile();
  }

  void bindChannels() {
    for(int i=0;i<16;i++) {
      std::string cname="IN1C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in1[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="IN2C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in2[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="IN3C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in3[i]);
    }
    for(int i=0;i<16;i++) {
      std::string cname="IN4C"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&in4[i]);
    }
    for(int i=0;i<4;i++) {
      std::string cname="P"+std::to_string(i+1);
      cs.bindControlChannel(cname.c_str(),&p[i]);
    }
  }

  void onReset(const ResetEvent& e) override {
    cs.resetCsound(32,NCHNLS*2,APP->engine->getSampleRate());
    bindChannels();
    compile();
  }

  void onSampleRateChange(const SampleRateChangeEvent &e) override {
    //cs.resetCsound(32,2,APP->engine->getSampleRate());
  }

  void compile() {
    getParamQuantity(ON_PARAM)->setValue(0);
    compileError=1;
    std::string orc=instrument+"\n";
    compileError=cs.compile(orc);
  }

  void setText(std::string text) {
    instrument=text;
  }

  void setParameters() {
    bool in1Connected=inputs[IN1].isConnected();
    bool in2Connected=inputs[IN2].isConnected();
    bool in3Connected=inputs[IN3].isConnected();
    bool in4Connected=inputs[IN4].isConnected();
    if(in1Connected) {
      int channels=inputs[IN1].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in1[chn][0]=inputs[IN1].getVoltage(chn);
      }
    }
    if(in2Connected) {
      int channels=inputs[IN2].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in2[chn][0]=inputs[IN2].getVoltage(chn);
      }
    }
    if(in3Connected) {
      int channels=inputs[IN3].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in3[chn][0]=inputs[IN3].getVoltage(chn);
      }
    }
    if(in4Connected) {
      int channels=inputs[IN4].getChannels();
      for(int chn=0;chn<channels;chn++) {
        in4[chn][0]=inputs[IN4].getVoltage(chn);
      }
    }
    for(int i=0;i<4;i++) {
      p[i][0]=params[KNOB_PARAM+i].getValue();
    }
  }
  void process(const ProcessArgs &args) override {
     if(NCHNLS==16) {
       processPoly(args);
     } else {
       process1(args);
     }
    blinkPhase+=args.sampleTime;
    if(blinkPhase>=1.0f)
      blinkPhase-=1.0f;
    if(compileError==0) {
      lights[OK_LIGHT].value=0;
      lights[ERROR_LIGHT].value=1;
    } else {
      lights[OK_LIGHT].value=(blinkPhase<0.5f)?1.0f:0.0f;
      lights[ERROR_LIGHT].value=0;
    }
  }
  void processPoly(const ProcessArgs &args) {
    int channels=inputs[LEFT_INPUT].getChannels();
    bool g=params[ON_PARAM].getValue()>0.f;
    if(g&&!op) {
      for(int k=0;k<channels;k++) {
        cs.readScoreEvent(events[k].c_str());
      }
      op=true;
    }
    if(g==0.f&&op) {
      cs.killAllInstr(1.0);
      op=false;
    }
    if(compileError==0) {
      if(pos==0) {
        setParameters();
        cs.perform();
      }
      for(int k=0;k<channels;k++) {
        cs.in[pos*cs.getNChnls()+2*k]=inputs[LEFT_INPUT].getVoltage(k)*0.2f;
        cs.in[pos*cs.getNChnls()+2*k+1]=inputs[RIGHT_INPUT].getVoltage(k)*0.2f;
      }
      for(int k=0;k<channels;k++) {
        outputs[LEFT_OUTPUT].setVoltage(cs.out[pos*cs.getNChnls()+2*k]*5.f,k);
        outputs[RIGHT_OUTPUT].setVoltage(cs.out[pos*cs.getNChnls()+2*k+1]*5.f,k);
      }
      pos++;
      if(pos>=cs.getKsmps()) {
        pos=0;
      }
    }
    outputs[LEFT_OUTPUT].setChannels(channels);
    outputs[RIGHT_OUTPUT].setChannels(channels);

  }

  void process1(const ProcessArgs &args)  {
    bool g=params[ON_PARAM].getValue()>0.f;
    if(g&&!op) {
      cs.readScoreEvent("i 1 0 -1 1");
      op=true;
    }
    if(g==0.f&&op) {
      cs.killInstr(1.0);
      op=false;
    }
    if(compileError==0) {
      if(pos==0) {
        setParameters();
        cs.perform();
      }

      cs.in[pos*2]=inputs[LEFT_INPUT].getVoltage()*0.2f;
      cs.in[pos*2+1]=inputs[RIGHT_INPUT].getVoltage()*0.2f;
      outputs[LEFT_OUTPUT].setVoltage(cs.out[pos*2]*5.f);
      outputs[RIGHT_OUTPUT].setVoltage(cs.out[pos*2+1]*5.f);

      pos++;
      if(pos>=cs.getKsmps()) {
        pos=0;
      }
    }
  }

  json_t *dataToJson() override {
    json_t *rootJ=json_object();
    json_object_set_new(rootJ,"csd",json_string(instrument.c_str()));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *textJ=json_object_get(rootJ,"csd");
    if(textJ)
      instrument=json_string_value(textJ);
    compile();
    dirty=true;
  }

};

struct CsoundModuleFX16 : CsoundModuleFX {
  CsoundModuleFX16() : CsoundModuleFX(16) {}
};

struct CsoundModuleWidgetFX : ModuleWidget {

  CsoundModuleWidgetFX(CsoundModuleFX *module,const std::string &svgFile="res/CsoundModuleFX.svg") {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, svgFile)));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    if(module && module->NCHNLS==16) {
      auto text = new DBTextWidget("***",mm2px(Vec(56.f,1.9f)),Vec(30.f,12.f));
      addChild(text);
    }

    ScrollWidget *scrollWidget=new CSTextScrollWidget();
    scrollWidget->box.size=mm2px(Vec(85,68));
    scrollWidget->box.pos=mm2px(Vec(3.5f,MHEIGHT-54-68));
    //INFO("%f",scrollWidget->box.size.y);
    addChild(scrollWidget);
    auto textField=createWidget<CSoundTextField<CsoundModuleFX>>(Vec(0,0));
    textField->setModule(module);
    textField->box.size=mm2px(Vec(200,500));
    textField->multiline=true;
    textField->scroll=scrollWidget;
    scrollWidget->container->addChild(textField);

    float x=8;
    float y=80;
    auto gen=createParam<CompileButton<CsoundModuleFX>>(mm2px(Vec(x,y)),module,CsoundModuleFX::COMPILE_BUTTON);
    gen->module=module;
    addParam(gen);
    addChild(createLightCentered<LargeLight<GreenRedLight>>(mm2px(Vec(35,83.14)),module,CsoundModuleFX::ERROR_LIGHT));
    addParam(createParam<MLED>(mm2px(Vec(48,y)),module,CsoundModuleFX::ON_PARAM));

    y=96;
    addInput(createInput<SmallPort>(mm2px(Vec(x,y)),module,CsoundModuleFX::IN1));
    addInput(createInput<SmallPort>(mm2px(Vec(x+10,y)),module,CsoundModuleFX::IN2));
    addInput(createInput<SmallPort>(mm2px(Vec(x+20,y)),module,CsoundModuleFX::IN3));
    addInput(createInput<SmallPort>(mm2px(Vec(x+30,y)),module,CsoundModuleFX::IN4));
    for(int i=0;i<4;i++) {
      addParam(createParam<TrimbotWhite>(mm2px(Vec(x+40+i*10,y)),module,CsoundModuleFX::KNOB_PARAM+i));
    }
    y=112;
    addInput(createInput<SmallPort>(mm2px(Vec(x,y)),module,CsoundModuleFX::LEFT_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(x+10,y)),module,CsoundModuleFX::RIGHT_INPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(x+60,y)),module,CsoundModuleFX::LEFT_OUTPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(x+70,y)),module,CsoundModuleFX::RIGHT_OUTPUT));

  }
};
struct CsoundModuleWidget16FX : CsoundModuleWidgetFX {
  CsoundModuleWidget16FX(CsoundModuleFX* module) : CsoundModuleWidgetFX(module,"res/CsoundModule16FX.svg") {

  }
};

Model* modelCsoundModule = createModel<CsoundModule, CsoundModuleWidget>("CsoundModule");
Model* modelCsoundModule16 = createModel<CsoundModule16, CsoundModuleWidget16>("CsoundModule16");
Model* modelCsoundModuleFX = createModel<CsoundModuleFX, CsoundModuleWidgetFX>("CsoundModuleFX");
Model* modelCsoundModuleFX16 = createModel<CsoundModuleFX16, CsoundModuleWidget16FX>("CsoundModuleFX16");
