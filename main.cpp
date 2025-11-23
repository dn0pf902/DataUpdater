#include "DataUpdater.h"

IMod* BMLEntry(IBML* bml) {
	return new DataUpdater(bml);
}

void DataUpdater::OnLoad() {
	GetConfig()->SetCategoryComment("General", "General settings.");

	prop_enabled = GetConfig()->GetProperty("General", "Enabled");
	prop_enabled->SetDefaultBoolean(true);
	prop_enabled->SetComment("Enabled to use this mod");
	enabled = prop_enabled->GetBoolean();

	GetConfig()->SetCategoryComment("Update", "Data Updater settings.");

	prop_key = GetConfig()->GetProperty("Update", "Hotkey");
	prop_key->SetDefaultKey(CKKEY_Q);
	prop_key->SetComment("Hotkey to update data");
	hotkey = prop_key->GetKey();

	input_manager = m_bml->GetInputManager();

	InitPhysicsMethodPointers();

	m_IpionManager = (CKIpionManager*)m_BML->GetCKContext()->GetManagerByGuid(CKGUID(0x6bed328b, 0x141f5148));
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
	if (input_manager->IsKeyPressed(hotkey)) {
		auto pos = get_ball_pos();
		auto vel = get_ball_vel();

		char buf[128];
		std::snprintf(buf, sizeof(buf), "Ball Velocity: X=%.3f Y=%.3f Z=%.3f", vel.x, vel.y, vel.z);
		m_bml->SendIngameMessage(buf);
		std::snprintf(buf, sizeof(buf), "Ball Position: X=%.3f Y=%.3f Z=%.3f", pos.x, pos.y, pos.z);
		m_bml->SendIngameMessage(buf);
	}
}