#include "DataUpdater.h"

#include <BML/Bui.h>

IMod* BMLEntry(IBML* bml) {
	return new DataUpdater(bml);
}

void DataUpdater::OnLoad() {
	GetConfig()->SetCategoryComment("General", "General settings.");

	prop_enabled = GetConfig()->GetProperty("General", "Enabled");
	prop_enabled->SetDefaultBoolean(true);
	prop_enabled->SetComment("Enabled to use this mod");
	enabled = prop_enabled->GetBoolean();

	GetConfig()->SetCategoryComment("Update", "Update settings.");

	prop_hotkey_enabled = GetConfig()->GetProperty("Update", "EnableHotkey");
	prop_hotkey_enabled->SetDefaultBoolean(false);
	prop_hotkey_enabled->SetComment("Enable hotkey to update data");
	hotkey_enabled = prop_hotkey_enabled->GetBoolean();

	prop_hotkey = GetConfig()->GetProperty("Update", "Hotkey");
	prop_hotkey->SetDefaultKey(CKKEY_Q);
	prop_hotkey->SetComment("Hotkey to update data");
	hotkey = prop_hotkey->GetKey();

	prop_update_frame = GetConfig()->GetProperty("Update", "UpdateFrame");
	prop_update_frame->SetDefaultInteger(0);
	prop_update_frame->SetComment("Frame to update data automatically (0 to disable)");
	update_frame = prop_update_frame->GetInteger();

	input_manager = m_bml->GetInputManager();

	InitPhysicsMethodPointers();

	m_IpionManager = (CKIpionManager*)m_BML->GetCKContext()->GetManagerByGuid(CKGUID(0x6bed328b, 0x141f5148));
}

void DataUpdater::OnPostLoadLevel() {

}

void DataUpdater::OnStartLevel() {

}

void DataUpdater::OnPreLoadLevel() {
	frame_cnt = -2;
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

void DataUpdater::OnPrintData() {
	if (!bg) {
		bg = std::make_unique<decltype(bg)::element_type>("DataBackground");
		bg->SetSize({ 0.6f, 0.12f });
		bg->SetPosition({ 0.2f, 0.9f });
		bg->SetZOrder(127);
		bg->SetColor({ 0, 0, 0, 175 });
	}

	if (!sprite) {
		sprite = std::make_unique<decltype(sprite)::element_type>("DataDisplay");
		sprite->SetSize({ 0.6f, 0.12f });
		sprite->SetPosition({ 0.2f, 0.9f });
		sprite->SetAlignment(CKSPRITETEXT_CENTER);
		sprite->SetTextColor(0xffffffff);
		sprite->SetZOrder(128);
		sprite->SetFont("Consolas", 18, 400, false, false);
	}

	char txt[128];
	std::snprintf(txt, sizeof(txt), "frame cnt = %d", frame_cnt);
	sprite->SetText(txt);
}

void DataUpdater::OnProcess() {
	if (!enabled) return;

	frame_cnt++;

	if ((hotkey_enabled && input_manager->IsKeyPressed(hotkey)) || (update_frame != 0 && frame_cnt == update_frame)) {
		auto pos = get_ball_pos();
		auto vel = get_ball_vel();

		char buf[128];
		std::snprintf(buf, sizeof(buf), "framecnt=%d", frame_cnt);
		m_bml->SendIngameMessage(buf);
		std::snprintf(buf, sizeof(buf), "ball_pos=%.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
		m_bml->SendIngameMessage(buf);
		std::snprintf(buf, sizeof(buf), "ball_vel=%.3f, %.3f, %.3f", vel.x, vel.y, vel.z);
		m_bml->SendIngameMessage(buf);
	}

	OnPrintData();
}