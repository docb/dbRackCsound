
#ifndef DBRACKCSOUND_DBCSOUND_HPP
#define DBRACKCSOUND_DBCSOUND_HPP
#include "csound.hpp"
#include "OpcodeBase.hpp"
extern "C" {
extern int init_static_modules(CSOUND *);
};

struct SMT : csound::OpcodeBase<SMT> {
  MYFLT *out;
  MYFLT *kv;
  double onThresh;
  double offThresh;
  bool state=true;
  int init(CSOUND *csound) {
    state=true;
    onThresh=1.f;
    offThresh=0.f;
    return OK;
  }
  int kontrol(CSOUND *csound) {
    MYFLT val = *kv;
    if(state) {
      if(val<=offThresh) {
        state=false;
      }
    } else {
      if(val>=onThresh) {
        state=true;
        *out=1;
        return OK;
      }
    }
    *out=0;
    return OK;
  }
};


struct DBCsound {
  MYFLT *in=nullptr;
  MYFLT *out=nullptr;
  Csound *csound;
  DBCsound() {
    INFO("Init Csound 0");
    csoundSetOpcodedir("");
    csound=new Csound();
    INFO("created csound instance version:%d",csound->GetVersion());
  }
  ~DBCsound() {
    csound->Stop();
    delete csound;
  }
  void initCsound(int ksmps,int nchnls,float sr) {
    INFO("Init Csound 1");
    csound->SetOption((char *)"-n");
    csound->SetOption((char *)"-dm0");
#ifndef ARCH_WIN
    INFO("init static modules");
    init_static_modules(csound->GetCsound());
#endif
    INFO("init csound params");
    auto *csoundParams=new CSOUND_PARAMS();
    csoundParams->sample_rate_override=sr;
    csoundParams->ksmps_override=ksmps;
    csoundParams->nchnls_override=nchnls;
    csoundParams->e0dbfs_override=1;
    csoundParams->displays=0;
    csound->SetParams(csoundParams);
    csound->SetHostImplementedAudioIO(1,0);
    INFO("init smt opcode");
    csound->AppendOpcode("smt",sizeof(SMT),0,3,"k","k",(int (*)(CSOUND *, void *))SMT::init_,(int (*)(CSOUND *, void *))SMT::kontrol_,nullptr);
    INFO("Start Csound");
    csound->Start();
    out=csound->GetSpout();
    in=csound->GetSpin();
  }

  void resetCsound(int ksmps,int nchnls,float sr) {
    csound->Stop();
    csound->Reset();
    initCsound(ksmps,nchnls,sr);
  }

  void bindAudioChannel(const std::string &name,MYFLT **data) {
    int err=csoundGetChannelPtr(csound->GetCsound(),data,name.c_str(),CSOUND_AUDIO_CHANNEL|CSOUND_INPUT_CHANNEL);
    if(err) {
      INFO("%s get audio channel failed %d",name.c_str(),err);
    }
  }
  void bindControlChannel(const std::string &name,MYFLT **data) {
    csoundGetChannelPtr(csound->GetCsound(),data,name.c_str(),CSOUND_CONTROL_CHANNEL|CSOUND_INPUT_CHANNEL);
  }

  int compile(const std::string& orc) {
    INFO("start compiling");
    int compileError=csound->CompileOrc(orc.c_str());
    if(!compileError) {
      INFO("csound compiled ksnps=%d nchnls=%d",csound->GetKsmps(),csound->GetNchnls());
    } else {
      INFO("compile error %d",compileError);
      INFO("%s",orc.c_str());
    }
    return compileError;
  }

  void killInstr(double inst) {
    //INFO("Kill %f",inst);
    csoundKillInstance(csound->GetCsound(),inst,nullptr,4,1);
  }
  void killAllInstr(double inst) {
    csoundKillInstance(csound->GetCsound(),inst,nullptr,0,1);
  }

  void readScoreEvent(const std::string &e) {
    csound->ReadScore(e.c_str());
  }

  void perform() {
    csound->PerformKsmps();
  }

  int getKsmps() {
    return csound->GetKsmps();
  }

  int getNChnls() {
    return csound->GetNchnls();
  }



};


#endif //DBRACKCSOUND_DBCSOUND_HPP
