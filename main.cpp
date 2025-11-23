#include "DataUpdater.h"

IMod* BMLEntry(IBML* bml) {
	return new DataUpdater(bml);
}

void DataUpdater::OnLoad() {
	GetConfig()->SetCategoryComment("Main", "Main settings.");

	prop_enabled = GetConfig()->GetProperty("Main", "Enabled");
	prop_enabled->SetDefaultBoolean(true);
	prop_enabled->SetComment("Enabled to use this mod");
	enabled = prop_enabled->GetBoolean();
}

void DataUpdater::OnStartLevel() {
	m_bml->SendIngameMessage("test");
	m_bml->SendIngameMessage("test");
}

void DataUpdater::OnLoadScript(const char* filename, CKBehavior* script) {
	if (!enabled) return;
	if (!strcmp(script->GetName(), "Gameplay_Ingame")) {
		for (int i = 0; i < script->GetLocalParameterCount(); ++i) {
			CKParameter* param = script->GetLocalParameter(i);
			if (!strcmp(param->GetName(), "ActiveBall")) {
				m_ActiveBall = param;
				break;
			}
		}
	}
}

void DataUpdater::OnProcess() {
	if (!enabled) return;
}