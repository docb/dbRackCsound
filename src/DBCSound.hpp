
#ifndef DBRACKCSOUND_DBCSOUND_HPP
#define DBRACKCSOUND_DBCSOUND_HPP
#include "csound.hpp"

struct DBCsound {
  MYFLT *in=nullptr;
  MYFLT *out=nullptr;
  Csound *csound;
  DBCsound() {
  }
  ~DBCsound() {
    csound->Stop();
    delete csound;
  }
  void initCsound(int ksmps,int nchnls,float sr) {
    csound=new Csound();
    csound->SetOption((char *)"-n");
    csound->SetOption((char *)"-dm0");
    auto *csoundParams=new CSOUND_PARAMS();
    csoundParams->sample_rate_override=sr;
    csoundParams->ksmps_override=ksmps;
    csoundParams->nchnls_override=nchnls;
    csoundParams->e0dbfs_override=1;
    csoundParams->displays=0;
    csound->SetParams(csoundParams);
    csound->SetHostImplementedAudioIO(1,0);
    csound->Start();
    out=csound->GetSpout();
    in=csound->GetSpin();
  }

  void resetCsound(int ksmps,int nchnls,float sr) {
    csound->Stop();
    delete csound;
    initCsound(ksmps,nchnls,sr);
  }

  void bindAudioChannel(const std::string &name,MYFLT **data) {
    csoundGetChannelPtr(csound->GetCsound(),data,name.c_str(),CSOUND_AUDIO_CHANNEL|CSOUND_INPUT_CHANNEL);
  }
  void bindControlChannel(const std::string &name,MYFLT **data) {
    csoundGetChannelPtr(csound->GetCsound(),data,name.c_str(),CSOUND_CONTROL_CHANNEL|CSOUND_INPUT_CHANNEL);
  }

  int compile(const std::string& orc) {
    int compileError=csound->CompileOrc(orc.c_str());
    if(!compileError) {
      INFO("csound compiled ksnps=%d nchnls=%d",csound->GetKsmps(),csound->GetNchnls());
    } else {
      INFO("compile error %d",compileError);
      INFO("%s",orc.c_str());
    }
    return compileError;
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
