#include "DataUpdater.h"

#include <BML/Bui.h>

IMod* BMLEntry(IBML* bml) {
	return new DataUpdater(bml);
}

void DataUpdater::OnLoad() {
	GetConfig()->SetCategoryComment("General", "General settings");

	prop_enabled = GetConfig()->GetProperty("General", "Enabled");
	prop_enabled->SetDefaultBoolean(true);
	prop_enabled->SetComment("Enabled to use this mod");
	enabled = prop_enabled->GetBoolean();

	GetConfig()->SetCategoryComment("Update", "Update settings");

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

	GetConfig()->SetCategoryComment("Data", "Stored data.");

	prop_update_direction = GetConfig()->GetProperty("Data", "UpdateDirection");
	prop_update_direction->SetDefaultString("+x");
	prop_update_direction->SetComment("Direction to update data (+x, -x, +y, -y, +z, -z)");
	update_direction = prop_update_direction->GetString();

	prop_frame_of_data = GetConfig()->GetProperty("Data", "FrameOfData");
	prop_frame_of_data->SetDefaultInteger(0);
	prop_frame_of_data->SetComment("Frame number of the stored data");
	frame_of_data = prop_frame_of_data->GetInteger();

	prop_data_pos = GetConfig()->GetProperty("Data", "PositionData");
	prop_data_pos->SetDefaultFloat(0.0f);
	prop_data_pos->SetComment("Position data");
	data_pos = prop_data_pos->GetFloat();

	prop_data_vel = GetConfig()->GetProperty("Data", "VelocityData");
	prop_data_vel->SetDefaultFloat(0.0f);
	prop_data_vel->SetComment("Velocity data");
	data_vel = prop_data_vel->GetFloat();

	prop_preserved_data = GetConfig()->GetProperty("Data", "PreservedData");
	prop_preserved_data->SetDefaultString("#0:+x,pos=0.000,vel=0.000");
	prop_preserved_data->SetComment("Preserved data string(can be updated by using \"/datupd upd p\")");
	preserved_data = prop_preserved_data->GetString();

	GetConfig()->SetCategoryComment("CompareRule", "Comparison rules for data.\n(if current_pos>pos or (current_pos>pos-[DeltaPosition] and current_vel>vel+[DeltaVelocity]) then update)");

	prop_dlt_pos = GetConfig()->GetProperty("CompareRule", "DeltaPosition");
	prop_dlt_pos->SetDefaultFloat(0.1f);
	prop_dlt_pos->SetComment("Allowed delta for position comparison(default:0.1)");
	dlt_pos = prop_dlt_pos->GetFloat();

	prop_dlt_vel = GetConfig()->GetProperty("CompareRule", "DeltaVelocity");
	prop_dlt_vel->SetDefaultFloat(1.0f);
	prop_dlt_vel->SetComment("Allowed delta for velocity comparison(default:1.0)");
	dlt_vel = prop_dlt_vel->GetFloat();

	//other initializations
	input_manager = m_bml->GetInputManager();

	InitPhysicsMethodPointers();

	m_IpionManager = (CKIpionManager*)m_BML->GetCKContext()->GetManagerByGuid(CKGUID(0x6bed328b, 0x141f5148));
}

void DataUpdater::OnPostLoadLevel() {

}

void DataUpdater::OnStartLevel() {
	ShowData();
}

void DataUpdater::OnPreExitLevel() {
	HideData();
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

void DataUpdater::ShowData() {
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
}

void DataUpdater::HideData() {
	if (bg) {
		bg.reset();
	}
	if (sprite) {
		sprite.reset();
	}
}

bool DataUpdater::cmp(int frame, VxVector cur_pos, VxVector cur_vel) {
	float cur_value = 0.0f;
	float data_value = 0.0f;
	if (update_direction == "+x") {
		cur_value = cur_pos.x;
		data_value = data_pos;
	}
	else if (update_direction == "-x") {
		cur_value = -cur_pos.x;
		data_value = -data_pos;
	}
	else if (update_direction == "+y") {
		cur_value = cur_pos.y;
		data_value = data_pos;
	}
	else if (update_direction == "-y") {
		cur_value = -cur_pos.y;
		data_value = -data_pos;
	}
	else if (update_direction == "+z") {
		cur_value = cur_pos.z;
		data_value = data_pos;
	}
	else if (update_direction == "-z") {
		cur_value = -cur_pos.z;
		data_value = -data_pos;
	}
	else {
		return false;
	}
	if (cur_value > data_value) {
		return true;
	}
	else if (cur_value > data_value - dlt_pos) {
		float cur_vel_value = 0.0f;
		if (update_direction == "+x" || update_direction == "-x") {
			cur_vel_value = cur_vel.x;
		}
		else if (update_direction == "+y" || update_direction == "-y") {
			cur_vel_value = cur_vel.y;
		}
		else if (update_direction == "+z" || update_direction == "-z") {
			cur_vel_value = cur_vel.z;
		}
		if (cur_vel_value > data_vel + dlt_vel) {
			return true;
		}
	}
	return false;
}

void DataUpdater::update_data(int frame, VxVector cur_pos, VxVector cur_vel) {
	float cur_value = 0.0f;
	if (update_direction == "+x") {
		cur_value = cur_pos.x;
	}
	else if (update_direction == "-x") {
		cur_value = -cur_pos.x;
	}
	else if (update_direction == "+y") {
		cur_value = cur_pos.y;
	}
	else if (update_direction == "-y") {
		cur_value = -cur_pos.y;
	}
	else if (update_direction == "+z") {
		cur_value = cur_pos.z;
	}
	else if (update_direction == "-z") {
		cur_value = -cur_pos.z;
	}
	else {
		return;
	}
	data_pos = cur_value;
	data_vel = 0.0f;
	if (update_direction == "+x" || update_direction == "-x") {
		data_vel = cur_vel.x;
	}
	else if (update_direction == "+y" || update_direction == "-y") {
		data_vel = cur_vel.y;
	}
	else if (update_direction == "+z" || update_direction == "-z") {
		data_vel = cur_vel.z;
	}
	frame_of_data = frame;
	prop_data_pos->SetFloat(data_pos);
	prop_data_vel->SetFloat(data_vel);
	prop_frame_of_data->SetInteger(frame_of_data);
	char buf[128];
	std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_of_data, update_direction.c_str(), data_pos, data_vel);
	preserved_data = buf;
	prop_preserved_data->SetString(preserved_data.c_str());
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
		
		char txt[128];
		std::snprintf(txt, sizeof(txt), "frame cnt = %d", frame_cnt);
		sprite->SetText(txt);

		if (cmp(frame_cnt, pos, vel)) {
			update_data(frame_cnt, pos, vel);
		}
		else {

		}
	}

}