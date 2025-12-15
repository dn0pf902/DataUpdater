#include "DataUpdater.h"

#include <BML/Bui.h>
#include <Windows.h>

IMod* BMLEntry(IBML* bml) {
	return new DataUpdater(bml);
}

void DataUpdater::GetUpdateRuleList() {
	memset(allowed_rules, false, sizeof(allowed_rules));
	size_t start = 0;
	size_t end = upd_rule_list.find(',');
	while (end != std::string::npos) {
		std::string rule = upd_rule_list.substr(start, end - start);
		int r = std::stoi(rule);
		if (r >= 1 && r <= 5) {
			allowed_rules[r] = true;
		}
		start = end + 1;
		end = upd_rule_list.find(',', start);
	}
	std::string rule = upd_rule_list.substr(start);
	int r = std::stoi(rule);
	if (r >= 1 && r <= 5) {
		allowed_rules[r] = true;
	}

	for (int i = 1; i <= 5; i++) {
		if (allowed_rules[i]) {
			char buf[128];
			std::snprintf(buf, sizeof(buf), "DataUpdater: Enable update rule %d.", i);
			m_bml->SendIngameMessage(buf);
		}
	}
}

void DataUpdater::OnLoad() {
	m_bml->RegisterCommand(new CommandDatupd(this));

	GetConfig()->SetCategoryComment("General", "General settings");

	prop_enabled = GetConfig()->GetProperty("General", "Enabled");
	prop_enabled->SetDefaultBoolean(true);
	prop_enabled->SetComment("Enabled to use this mod");
	enabled = prop_enabled->GetBoolean();

	GetConfig()->SetCategoryComment("Update", "Update settings");

	prop_update_enabled = GetConfig()->GetProperty("Update", "EnableUpdate");
	prop_update_enabled->SetDefaultBoolean(true);
	prop_update_enabled->SetComment("Enable updating data");
	update_enabled = prop_update_enabled->GetBoolean();

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
	if (update_direction != "+x" && update_direction != "-x" &&
		update_direction != "+y" && update_direction != "-y" &&
		update_direction != "+z" && update_direction != "-z") {
		m_bml->SendIngameMessage("DataUpdater: Invalid UpdateDirection in config, resetting to +x.");
		update_direction = "+x";
		prop_update_direction->SetString(update_direction.c_str());
	}

	prop_data_direction = GetConfig()->GetProperty("Data", "DataDirection");
	prop_data_direction->SetDefaultString("+x");
	prop_data_direction->SetComment("Direction of the stored data (+x, -x, +y, -y, +z, -z)");
	data_direction = prop_data_direction->GetString();
	if (data_direction != "+x" && data_direction != "-x" &&
		data_direction != "+y" && data_direction != "-y" &&
		data_direction != "+z" && data_direction != "-z") {
		m_bml->SendIngameMessage("DataUpdater: Invalid DataDirection in config, resetting to +x.");
		data_direction = "+x";
		prop_data_direction->SetString(data_direction.c_str());
	}

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

	GetConfig()->SetCategoryComment("SaveFile", "File saving settings");

	prop_autosave_enabled = GetConfig()->GetProperty("SaveFile", "EnableAutoSave");
	prop_autosave_enabled->SetDefaultBoolean(false);
	prop_autosave_enabled->SetComment("Enable automatic file saving when updating data");
	autosave_enabled = prop_autosave_enabled->GetBoolean();

	prop_tas_name = GetConfig()->GetProperty("SaveFile", "TASFileName");
	prop_tas_name->SetDefaultString("1");
	prop_tas_name->SetComment("TAS file name for automatic saving (without extension)");
	tas_filename = prop_tas_name->GetString();

	prop_save_path = GetConfig()->GetProperty("SaveFile", "SavePath");
	prop_save_path->SetDefaultString((BML_TAS_PATH + std::string("SavedFile\\")).c_str());
	prop_save_path->SetComment("Full path to save TAS files");
	save_path = prop_save_path->GetString();

	GetConfig()->SetCategoryComment("CompareRule", "Comparison rules for data. For specific details, please refer to [reference website].");

	prop_enable_upd_lim = GetConfig()->GetProperty("CompareRule", "EnableUpdateLimit");
	prop_enable_upd_lim->SetDefaultBoolean(true);
	prop_enable_upd_lim->SetComment("Enable update limit for comparison(default:true)");
	enable_upd_lim = prop_enable_upd_lim->GetBoolean();

	prop_upd_lim_pos = GetConfig()->GetProperty("CompareRule", "UpdateLimitPosition");
	prop_upd_lim_pos->SetDefaultFloat(0.1f);
	prop_upd_lim_pos->SetComment("Update limit for position comparison(default:0.1)");
	upd_lim_pos = prop_upd_lim_pos->GetFloat();

	prop_upd_lim_vel = GetConfig()->GetProperty("CompareRule", "UpdateLimitVelocity");
	prop_upd_lim_vel->SetDefaultFloat(1.0f);
	prop_upd_lim_vel->SetComment("Update limit for velocity comparison(default:1.0)");
	upd_lim_vel = prop_upd_lim_vel->GetFloat();

	prop_upd_rule_list = GetConfig()->GetProperty("CompareRule", "UpdateRuleList");
	prop_upd_rule_list->SetDefaultString("1");
	prop_upd_rule_list->SetComment("List of update rules to use, separated by commas(e.g., \"1,2,4\")");
	upd_rule_list = prop_upd_rule_list->GetString();
	if (upd_rule_list.empty()) {
		upd_rule_list = "1";
		prop_upd_rule_list->SetString(upd_rule_list.c_str());
	}
	GetUpdateRuleList();

	for (int i = 1;i <= 5;i++) {
		prop_dlt_pos[i] = GetConfig()->GetProperty("CompareRule", ("DeltaPosition" + std::to_string(i)).c_str());
		prop_dlt_pos[i]->SetDefaultFloat(0.1f);
		prop_dlt_pos[i]->SetComment(("Allowed delta for position comparison(default:0.1)" + std::string("Rule ") + std::to_string(i)).c_str());
		dlt_pos[i] = prop_dlt_pos[i]->GetFloat();

		prop_dlt_vel[i] = GetConfig()->GetProperty("CompareRule", ("DeltaVelocity" + std::to_string(i)).c_str());
		prop_dlt_vel[i]->SetDefaultFloat(1.0f);
		prop_dlt_vel[i]->SetComment(("Allowed delta for velocity comparison(default:1.0)" + std::string("Rule ") + std::to_string(i)).c_str());
		dlt_vel[i] = prop_dlt_vel[i]->GetFloat();
	}

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
	if (!enabled) return;
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
		sprite_cur_data->SetPosition({ UI_posx, UI_posy + ITEM_Y_SHIFT / 18 * UI_font_size });
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

void DataUpdater::SaveFile() {
	std::string tas_path = BML_TAS_PATH + tas_filename + ".tas";
	std::ifstream file(tas_path.c_str());
	if (!file.good()) {
		m_BML->SendIngameMessage(("DataUpdater: TAS file " + tas_filename + ".tas " "not found, automatic saving aborted.").c_str());
		return;
	}
	namespace fs = std::filesystem;
	fs::path target_dir = fs::path(save_path) / ("#" + std::to_string(frame_of_data) + " " + data_direction);

	std::error_code ec;
	if (!fs::create_directories(target_dir, ec)) {
		if (ec) {
			m_bml->SendIngameMessage(("DataUpdater: Failed to create directory " + target_dir.string() + " (err=" + std::to_string(ec.value()) + "), automatic saving aborted.").c_str());
			return;
		}
	}

	char buf[128];
	std::snprintf(buf, sizeof(buf), "%c=%.3f,v%c=%.3f", data_direction.at(1), data_pos, data_direction.at(1), data_vel);
	std::string data_str(buf);
	fs::path full_target_path = target_dir / (tas_filename + " " + data_str + ".tas");

	if (!CopyFile(tas_path.c_str(), full_target_path.string().c_str(), FALSE)) {
		m_bml->SendIngameMessage(("DataUpdater: Failed to copy TAS to " + full_target_path.string()).c_str());
		return;
	}
	m_bml->SendIngameMessage(("DataUpdater: Saved TAS file to " + target_dir.string()).c_str());
}

void DataUpdater::AutoSaveFile() {
	if (!autosave_enabled) {
		return;
	}
	SaveFile();
}

int DataUpdater::cmp(int frame, VxVector cur_pos, VxVector cur_vel) const {
	if (frame_of_data == 0) {
		return 1;
	}
	if (frame != frame_of_data || update_direction != data_direction) {
		return -1;
	}
	float cur_pos_value = 0.0f;
	float data_pos_value = 0.0f;
	if (update_direction == "+x") {
		cur_pos_value = cur_pos.x;
		data_pos_value = data_pos;
	}
	else if (update_direction == "-x") {
		cur_pos_value = -cur_pos.x;
		data_pos_value = -data_pos;
	}
	else if (update_direction == "+y") {
		cur_pos_value = cur_pos.y;
		data_pos_value = data_pos;
	}
	else if (update_direction == "-y") {
		cur_pos_value = -cur_pos.y;
		data_pos_value = -data_pos;
	}
	else if (update_direction == "+z") {
		cur_pos_value = cur_pos.z;
		data_pos_value = data_pos;
	}
	else if (update_direction == "-z") {
		cur_pos_value = -cur_pos.z;
		data_pos_value = -data_pos;
	}
	else {
		return -1;
	}

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
		cur_vel_value = -cur_vel_value;
	}

	//update_limit
	if (enable_upd_lim) {
		if (cur_pos_value < data_pos_value + upd_lim_pos && cur_vel_value < data_vel_value - upd_lim_vel) {
			return 0;
		}
	}

	if (cur_pos_value > data_pos_value + 0.001f) {
		return 1;
	}

	for (int i = 1; i <= 5; i++) {
		if (!allowed_rules[i]) continue;
		if (cur_pos_value > data_pos_value - dlt_pos[i] && cur_vel_value > data_vel_value + dlt_vel[i]) {
			return 1;
		}
	}
	return 0;
}

void DataUpdater::update_data(int frame, VxVector cur_pos, VxVector cur_vel) {
	if (!update_enabled) return;
	char buf[128];
	std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_of_data, update_direction.c_str(), data_pos, data_vel);
	preserved_data = buf;
	prop_preserved_data->SetString(preserved_data.c_str());

	float cur_pos_value = 0.0f;
	if (update_direction == "+x" || update_direction == "-x") {
		cur_pos_value = cur_pos.x;
	}
	else if (update_direction == "+y" || update_direction == "-y") {
		cur_pos_value = cur_pos.y;
	}
	else if (update_direction == "+z" || update_direction == "-z") {
		cur_pos_value = cur_pos.z;
	}
	else {
		return;
	}
	data_pos = cur_pos_value;
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
	data_direction = update_direction;
	prop_data_pos->SetFloat(data_pos);
	prop_data_vel->SetFloat(data_vel);
	prop_frame_of_data->SetInteger(frame_of_data);
	prop_data_direction->SetString(data_direction.c_str());
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
		saved_cur_pos = curpos;
		saved_cur_vel = curvel;
		saved_cur_frame = frame_cnt;

		auto result = cmp(frame_cnt, pos, vel);
		if (result == -1) {
			sprite_cur_data->SetTextColor(0xffffff00);
			char buf[128];
			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_cnt, update_direction.c_str(), curpos, curvel);
			sprite_cur_data->SetText(buf);
			m_bml->SendIngameMessage("DataUpdater: Frame or Direction mismatch, cannot update data. Use /datupd clear to clear stored data.");
		}
		else if (result == 1) {
			sprite_cur_data->SetTextColor(0xff00ff00);
			char buf[128];
			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_cnt, update_direction.c_str(), curpos, curvel);
			sprite_cur_data->SetText(buf);
			update_data(frame_cnt, pos, vel);
			AutoSaveFile();
		}
		else {
			sprite_cur_data->SetTextColor(0xffff0000);
			char buf[128];
			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_cnt, update_direction.c_str(), curpos, curvel);
			sprite_cur_data->SetText(buf);
		}
	}

}