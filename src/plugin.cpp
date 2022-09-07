#include "plugin.hpp"

Plugin* pluginInstance;

extern Model* modelCsoundModule;
extern Model* modelCsoundModule16;
extern Model* modelCsoundModuleFX;
extern Model* modelCsoundModuleFX16;

void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(modelCsoundModule);
  p->addModel(modelCsoundModule16);
  p->addModel(modelCsoundModuleFX);
  p->addModel(modelCsoundModuleFX16);
}
