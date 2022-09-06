#include "plugin.hpp"

Plugin* pluginInstance;

extern Model* modelCsoundModule;

void init(Plugin* p) {
	pluginInstance = p;

	p->addModel(modelCsoundModule);
}
