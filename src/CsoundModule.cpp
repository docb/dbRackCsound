#include "plugin.hpp"
#include "DBCSound.hpp"
#include "textfield.hpp"
struct CsoundModule : Module {
	enum ParamId {
    COMPILE_BUTTON,PARAMS_LEN
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

  std::string trigString="\ninstr trig\n"
                         "  insno nstrnum \"main\"\n"
                         "  if p4 == 1 then\n"
                         "    event \"i\",insno+(p5*0.01),0,-1,p5\n"
                         "  else\n"
                         "    turnoff2 insno+(p5*0.01),4,1\n"
                         "  endif\n"
                         "  turnoff\n"
                         "\nendin\n";

  std::string instrument="instr 1\n"
                         "kfreq chnget sprintf(\"FREQ%d\",p4)\n"
                         "ao vco2 0.7,kfreq\n"
                         "aenv madsr 0.001,0,1,0.02\n"
                         "ao*=aenv \n"
                         "outs ao,ao\n"
                         "endin\n";

  int compileError=1;
  bool on[16]={};
  bool op=false;
  float blinkPhase=0.f;
  bool dirty=false;
  int pos=0;

	CsoundModule() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configButton(COMPILE_BUTTON,"Compile");
    configInput(GATE_INPUT,"GATE");
    configInput(IN1,"IN 1");
    configInput(IN2,"IN 2");
    configInput(IN3,"IN 3");
    configInput(IN4,"IN 4");
    configInput(PHS_INPUT,"PHS");
    configInput(VOCT_INPUT,"V/O");
    configOutput(LEFT_OUTPUT,"Left");
    configOutput(RIGHT_OUTPUT,"Right");

    cs.initCsound(32,2,APP->engine->getSampleRate());
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
    dirty=true;
    compile();
	}

  void onSampleRateChange(const SampleRateChangeEvent& e) override {
    //cs.resetCsound(32,2,APP->engine->getSampleRate());
  }
  void compile() {
    std::string orc=instrument+"\n"+trigString;
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
        float freq=0.5f*dsp::FREQ_C4*dsp::approxExp2_taylor5(inputs[VOCT_INPUT].getVoltage(chn)+30.f)/std::pow(2.f,30.f);
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
  }

	void process(const ProcessArgs& args) override {
    float out1=0.0f,out2=0.0f;
    int channels=0;
    if(inputs[GATE_INPUT].isConnected()) {
      channels=inputs[GATE_INPUT].getChannels();
      for(int chn=0;chn<channels;chn++) {
        float g=inputs[GATE_INPUT].getVoltage(chn);
        if(g&&!on[chn]) {
          std::string scoreEventStr="i \"trig\" 0 0.1 1 "+std::to_string(chn+1);
          cs.readScoreEvent(scoreEventStr.c_str());
          on[chn]=true;
        }
        if(g==0.f&&on[chn]) {
          std::string scoreEventStr="i \"trig\" 0 0.1 0 "+std::to_string(chn+1);
          cs.readScoreEvent(scoreEventStr.c_str());
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
          phs[k][pos/2]=inputs[PHS_INPUT].getVoltage(k)*0.1f+0.5f;
        }
      } else {
        for(int k=0;k<16;k++) {
          phs[k][pos/2]=0;
        }
      }

      out1=cs.out[pos++];
      out2=cs.out[pos++];
      if(pos>=cs.getKsmps()*cs.getNChnls()) {
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
    outputs[LEFT_OUTPUT].setVoltage(out1*5.f);
    outputs[RIGHT_OUTPUT].setVoltage(out2*5.f);
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

struct CSoundTextField : MTextField {
  CsoundModule *module;

  CSoundTextField() : MTextField() {
    //bgColor=nvgRGB(0x30,0x30,0x30);
  }

  void setModule(CsoundModule *module) {
    this->module=module;
  }

  void onChange(const event::Change &e) override {
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

struct CompileButton : MLEDM {
  CsoundModule *module;

  void onChange(const ChangeEvent &e) override {
    SvgSwitch::onChange(e);
    if(module) {
      if(module->params[CsoundModule::COMPILE_BUTTON].getValue()>0)
        module->compile();
    }

  }
};


struct CsoundModuleWidget : ModuleWidget {
	CsoundModuleWidget(CsoundModule* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/CsoundModule.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    ScrollWidget *scrollWidget=new CSTextScrollWidget();
    scrollWidget->box.size=mm2px(Vec(85,68));
    scrollWidget->box.pos=mm2px(Vec(3.5f,MHEIGHT-54-68));
    //INFO("%f",scrollWidget->box.size.y);
    addChild(scrollWidget);
    auto textField=createWidget<CSoundTextField>(Vec(0,0));
    textField->setModule(module);
    textField->box.size=mm2px(Vec(200,500));
    textField->multiline=true;
    textField->scroll=scrollWidget;
    scrollWidget->container->addChild(textField);

    float x=8;
    float y=80;
    auto gen=createParam<CompileButton>(mm2px(Vec(x,y)),module,CsoundModule::COMPILE_BUTTON);
    gen->module=module;
    addParam(gen);
    addChild(createLightCentered<LargeLight<GreenRedLight>>(mm2px(Vec(35,84)),module,CsoundModule::ERROR_LIGHT));
    //addParam(createParam<MLED>(mm2px(Vec(x+24,y)),module,CsoundModule::ON_PARAM));

    y=96;
    addInput(createInput<SmallPort>(mm2px(Vec(x,y)),module,CsoundModule::IN1));
    addInput(createInput<SmallPort>(mm2px(Vec(x+10,y)),module,CsoundModule::IN2));
    addInput(createInput<SmallPort>(mm2px(Vec(x+20,y)),module,CsoundModule::IN3));
    addInput(createInput<SmallPort>(mm2px(Vec(x+30,y)),module,CsoundModule::IN4));
    y=112;
    addInput(createInput<SmallPort>(mm2px(Vec(x,y)),module,CsoundModule::VOCT_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(x+15,y)),module,CsoundModule::GATE_INPUT));
    addInput(createInput<SmallPort>(mm2px(Vec(x+30,y)),module,CsoundModule::PHS_INPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(x+60,y)),module,CsoundModule::LEFT_OUTPUT));
    addOutput(createOutput<SmallPort>(mm2px(Vec(x+70,y)),module,CsoundModule::RIGHT_OUTPUT));

	}
};


Model* modelCsoundModule = createModel<CsoundModule, CsoundModuleWidget>("CsoundModule");