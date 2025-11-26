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
	prop_preserved_data->SetComment("Preserved data string(can be updated back to current best data by using \"/datupd upd p\")");
	preserved_data = prop_preserved_data->GetString();

	GetConfig()->SetCategoryComment("CompareRule", "Comparison rules for data.(if current_pos>pos or (current_pos>pos-[DeltaPosition] and current_vel>vel+[DeltaVelocity]) then update)");

	prop_dlt_pos = GetConfig()->GetProperty("CompareRule", "DeltaPosition");
	prop_dlt_pos->SetDefaultFloat(0.1f);
	prop_dlt_pos->SetComment("Allowed delta for position comparison(default:0.1)");
	dlt_pos = prop_dlt_pos->GetFloat();

	prop_dlt_vel = GetConfig()->GetProperty("CompareRule", "DeltaVelocity");
	prop_dlt_vel->SetDefaultFloat(1.0f);
	prop_dlt_vel->SetComment("Allowed delta for velocity comparison(default:1.0)");
	dlt_vel = prop_dlt_vel->GetFloat();

	GetConfig()->SetCategoryComment("UI", "UI Settings");
	
	prop_UI_posx = GetConfig()->GetProperty("UI", "UIPosX");
	prop_UI_posx->SetDefaultFloat(0.6f);
	prop_UI_posx->SetComment("UI Position X (0.0 - 1.0)");
	UI_posx = prop_UI_posx->GetFloat();

	prop_UI_posy = GetConfig()->GetProperty("UI", "UIPosY");
	prop_UI_posy->SetDefaultFloat(0.0f);
	prop_UI_posy->SetComment("UI Position Y (0.0 - 1.0)");
	UI_posy = prop_UI_posy->GetFloat();

	prop_UI_font = GetConfig()->GetProperty("UI", "UIFont");
	prop_UI_font->SetDefaultString("Consolas");
	prop_UI_font->SetComment("UI Font Name");
	UI_font = prop_UI_font->GetString();

	prop_UI_font_size = GetConfig()->GetProperty("UI", "UIFontSize");
	prop_UI_font_size->SetDefaultInteger(18);
	prop_UI_font_size->SetComment("UI Font Size");
	UI_font_size = prop_UI_font_size->GetInteger();

	prop_UI_sizex = GetConfig()->GetProperty("UI", "UISizeX");
	prop_UI_sizex->SetDefaultFloat(0.4f);
	prop_UI_sizex->SetComment("UI Size X (0.0 - 1.0)");
	UI_sizex = prop_UI_sizex->GetFloat();

	prop_UI_sizey = GetConfig()->GetProperty("UI", "UISizeY");
	prop_UI_sizey->SetDefaultFloat(0.06f);
	prop_UI_sizey->SetComment("UI Size Y (0.0 - 1.0)");
	UI_sizey = prop_UI_sizey->GetFloat();
	
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
		bg->SetSize({ UI_sizex, UI_sizey });
		bg->SetPosition({ UI_posx, UI_posy });
		bg->SetZOrder(127);
		bg->SetColor({ 0, 0, 0, 175 });
	}

	if (!sprite_data) {
		sprite_data = std::make_unique<decltype(sprite_data)::element_type>("DataDisplay");
		sprite_data->SetSize({ UI_sizex, UI_sizey });
		sprite_data->SetPosition({ UI_posx, UI_posy });
		sprite_data->SetAlignment(CKSPRITETEXT_CENTER);
		sprite_data->SetTextColor(0xffffffff);
		sprite_data->SetZOrder(128);
		sprite_data->SetFont(UI_font.c_str(), UI_font_size, 400, false, false);

		char buf[128];
		std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_of_data, update_direction.c_str(), data_pos, data_vel);
		sprite_data->SetText(buf);
	}

	if (!sprite_cur_data) {
		sprite_cur_data = std::make_unique<decltype(sprite_cur_data)::element_type>("CurrentDataDisplay");
		sprite_cur_data->SetSize({ UI_sizex, UI_sizey });
		sprite_cur_data->SetPosition({ UI_posx, UI_posy + ITEM_Y_SHIFT });
		sprite_cur_data->SetAlignment(CKSPRITETEXT_CENTER);
		sprite_cur_data->SetTextColor(0xffffffff);
		sprite_cur_data->SetZOrder(128);
		sprite_cur_data->SetFont(UI_font.c_str(), UI_font_size, 400, false, false);
	}
}

void DataUpdater::HideData() {
	if (bg) {
		bg.reset();
	}
	if (sprite_data) {
		sprite_data.reset();
	}
	if (sprite_cur_data) {
		sprite_cur_data.reset();
	}
}

bool DataUpdater::cmp(VxVector cur_pos, VxVector cur_vel) const {
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
		float data_vel_value = 0.0f;
		if (update_direction == "+x" || update_direction == "-x") {
			cur_vel_value = cur_vel.x;
		}
		else if (update_direction == "+y" || update_direction == "-y") {
			cur_vel_value = cur_vel.y;
		}
		else if (update_direction == "+z" || update_direction == "-z") {
			cur_vel_value = cur_vel.z;
		}
		if (update_direction == "+x" || update_direction == "+y" || update_direction == "+z") {
			data_vel_value = data_vel;
		}
		else {
			data_vel_value = -data_vel;
		}
		if (cur_vel_value > data_vel_value + dlt_vel) {
			return true;
		}
	}
	return false;
}

void DataUpdater::update_data(int frame, VxVector cur_pos, VxVector cur_vel) {
	char buf[128];
	std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_of_data, update_direction.c_str(), data_pos, data_vel);
	preserved_data = buf;
	prop_preserved_data->SetString(preserved_data.c_str());

	float cur_value = 0.0f;
	if (update_direction == "+x" || update_direction == "-x") {
		cur_value = cur_pos.x;
	}
	else if (update_direction == "+y" || update_direction == "-y") {
		cur_value = cur_pos.y;
	}
	else if (update_direction == "+z" || update_direction == "-z") {
		cur_value = cur_pos.z;
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
}

void DataUpdater::OnProcess() {
	if (!enabled) return;

	frame_cnt++;

	if ((hotkey_enabled && input_manager->IsKeyPressed(hotkey)) || (update_frame != 0 && frame_cnt == update_frame)) {
		auto pos = get_ball_pos();
		auto vel = get_ball_vel();
		
		float curpos = 0.0f, curvel = 0.0f;
		if (update_direction == "+x" || update_direction == "-x") {
			curpos = pos.x;
			curvel = vel.x;
		}
		else if (update_direction == "+y" || update_direction == "-y") {
			curpos = pos.y;
			curvel = vel.y;
		}
		else if (update_direction == "+z" || update_direction == "-z") {
			curpos = pos.z;
			curvel = vel.z;
		}

		if (cmp(pos, vel)) {
			sprite_cur_data->SetTextColor(0xff00ff00);
			char buf[128];
			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_cnt, update_direction.c_str(), curpos, curvel);
			sprite_cur_data->SetText(buf);
			update_data(frame_cnt, pos, vel);
		}
		else {
			sprite_cur_data->SetTextColor(0xffff0000);
			char buf[128];
			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_cnt, update_direction.c_str(), curpos, curvel);
			sprite_cur_data->SetText(buf);
		}
	}

}