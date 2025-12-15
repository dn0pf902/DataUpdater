#pragma once

#ifdef USING_BML_PLUS
# include <BMLPlus/BMLAll.h>
# ifndef m_bml
#  define m_bml m_BML
#  define m_sprite m_Sprite
#  define VT21_REF(x) &(x)
# endif
typedef const char* ICKSTRING;
#else
# include <BML/BMLAll.h>
# define VT21_REF(x) (x)
typedef CKSTRING ICKSTRING;
#endif

#include "physics_RT.h"

#include <memory>
#include <fstream>
#include <filesystem>

#define BML_TAS_PATH "..\\ModLoader\\TASRecords\\"

extern "C" {
	__declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

class DataUpdater : public IMod {
	class CommandDatupd : public ICommand {
	private:
		DataUpdater* m_mod;
	public:
		virtual std::string GetName() override { return "dataupdater"; };
		virtual std::string GetAlias() override { return "datupd"; };
		virtual std::string GetDescription() override { return "Commands of DataUpdater."; };
		virtual bool IsCheat() override { return false; };
		CommandDatupd(DataUpdater* mod) : m_mod(mod) {}

		virtual void Execute(IBML* bml, const std::vector<std::string>& args) override {
			if (args.size() <= 1 || args[1] == "help" || args[1] == "h") {
				bml->SendIngameMessage("/datupd back/b : recover data to preserved data");
				bml->SendIngameMessage("/datupd clear/c : clear data");
				return;
			}
			if (args[1] == "back" || args[1] == "b") {
				return m_mod->Execute(args);
			}
			if (args[1] == "clear" || args[1] == "c") {
				return m_mod->Execute(args);
			}
			if (args[1] == "save" || args[1] == "s") {
				return m_mod->Execute(args);
			}
			if (args[1] == "update" || args[1] == "u") {
				return m_mod->Execute(args);
			}
		}
		virtual const std::vector<std::string> GetTabCompletion(IBML* bml, const std::vector<std::string>& args) override {
			return args.size() == 2 ? std::vector<std::string>{"help", "back", "clear", "save", "update"} : std::vector<std::string>{};
		};
	};
	void Execute(const std::vector<std::string>& args) {
		if (args[1] == "back" || args[1] == "b") {
			std::string s = preserved_data;
			int frame = 0;
			std::string dir;
			float pos = 0.0f, vel = 0.0f;

			size_t p1 = s.find(':');
			size_t p_pos = s.find(",pos=");
			size_t p_vel = s.find(",vel=");
			if (p1 != std::string::npos && p_pos != std::string::npos && p_vel != std::string::npos) {
				frame = std::stoi(s.substr(1, p1 - 1));
				dir = s.substr(p1 + 1, p_pos - (p1 + 1));
				pos = std::stof(s.substr(p_pos + 5, p_vel - (p_pos + 5)));
				vel = std::stof(s.substr(p_vel + 5));
				data_direction = dir;
				data_pos = pos;
				data_vel = vel;
				frame_of_data = frame;
				prop_data_direction->SetString(data_direction.c_str());
				prop_data_pos->SetFloat(data_pos);
				prop_data_vel->SetFloat(data_vel);
				prop_frame_of_data->SetInteger(frame_of_data);

				char buf[128];
				std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_of_data, data_direction.c_str(), data_pos, data_vel);
				sprite_data->SetText(buf);
			}
			else {
				m_bml->SendIngameMessage("DataUpdater: Failed to parse preserved data.");
			}
			return;
		}

		if (args[1] == "clear" || args[1] == "c") {
			data_direction = "+x";
			data_pos = 0.0f;
			data_vel = 0.0f;
			frame_of_data = 0;
			prop_data_direction->SetString(data_direction.c_str());
			prop_data_pos->SetFloat(data_pos);
			prop_data_vel->SetFloat(data_vel);
			prop_frame_of_data->SetInteger(frame_of_data);

			char buf[128];
			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_of_data, data_direction.c_str(), data_pos, data_vel);
			sprite_data->SetText(buf);
			return;
		}

		if (args[1] == "save" || args[1] == "s") {
			SaveFile();
			return;
		}

		if (args[1] == "update" || args[1] == "u") {
			sprite_cur_data->SetTextColor(0xff00ff00);
			char buf[128];
			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", saved_cur_frame, update_direction.c_str(), saved_cur_pos, saved_cur_vel);
			sprite_cur_data->SetText(buf);

			std::snprintf(buf, sizeof(buf), "#%d:%s,pos=%.3f,vel=%.3f", frame_of_data, update_direction.c_str(), data_pos, data_vel);
			preserved_data = buf;
			prop_preserved_data->SetString(preserved_data.c_str());

			frame_of_data = saved_cur_frame;
			data_pos = saved_cur_pos;
			data_vel = saved_cur_vel;
			data_direction = update_direction;
			prop_frame_of_data->SetInteger(frame_of_data);
			prop_data_pos->SetFloat(data_pos);
			prop_data_vel->SetFloat(data_vel);
			prop_data_direction->SetString(data_direction.c_str());

			AutoSaveFile();
			return;
		}
	}
private:
	std::unique_ptr<BGui::Panel> bg;
	std::unique_ptr<BGui::Text> sprite_data, sprite_cur_data;
	bool enabled = false;
	bool update_enabled = false;
	bool hotkey_enabled = false;
	bool autosave_enabled = false;
	CKParameter* m_ActiveBall = nullptr;
	IProperty* prop_enabled = nullptr;
	IProperty* prop_update_enabled = nullptr;
	IProperty* prop_hotkey = nullptr;
	IProperty* prop_hotkey_enabled = nullptr;
	IProperty* prop_update_frame = nullptr;
	IProperty* prop_update_direction = nullptr;
	IProperty* prop_data_direction = nullptr;
	IProperty* prop_frame_of_data = nullptr;
	IProperty* prop_data_pos = nullptr, * prop_data_vel = nullptr;
	IProperty* prop_preserved_data = nullptr;
	IProperty* prop_enable_upd_lim = nullptr, * prop_upd_lim_pos = nullptr, * prop_upd_lim_vel = nullptr;
	IProperty* prop_upd_rule_list = nullptr;
	IProperty* prop_dlt_vel[6] = { nullptr }, * prop_dlt_pos[6] = { nullptr };
	IProperty* prop_UI_posx = nullptr, * prop_UI_posy = nullptr, * prop_UI_font = nullptr, * prop_UI_font_size = nullptr;
	IProperty* prop_UI_sizex = nullptr, * prop_UI_sizey = nullptr;
	IProperty* prop_autosave_enabled = nullptr, * prop_tas_name = nullptr, * prop_save_path = nullptr;
	CKKEYBOARD hotkey = {};
	InputHook* input_manager = nullptr;
	CKIpionManager* m_IpionManager = nullptr;

	const float ITEM_Y_SHIFT = 0.03f;
	float UI_posx = 0.2f, UI_posy = 0.9f;
	float UI_sizex = 0.6f, UI_sizey = 0.12f;
	std::string UI_font = "Consolas";
	int UI_font_size = 18;

	int frame_cnt = 0;
	int update_frame = 0;
	int frame_of_data = 0;
	std::string update_direction = "";
	std::string data_direction = "";
	std::string preserved_data = "";
	float data_pos = 0.0f, data_vel = 0.0f;
	float saved_cur_pos = 0.0f, saved_cur_vel = 0.0f;
	int saved_cur_frame = 0;

	// Comparison rules
	bool enable_upd_lim = true;
	float upd_lim_pos = 0.1f, upd_lim_vel = 1.0f;
	float dlt_pos[6] = { 0.0f }, dlt_vel[6] = { 0.0f };
	std::string upd_rule_list = "1";
	bool allowed_rules[6] = { false };

	std::string tas_filename = "1";
	std::string save_path = BML_TAS_PATH + std::string("SavedFile\\");
public:
	DataUpdater(IBML* bml) : IMod(bml) {}

	virtual ICKSTRING GetID() override { return "DataUpdater"; }
	virtual ICKSTRING GetVersion() override { return "0.1.2"; }
	virtual ICKSTRING GetName() override { return "DataUpdater"; }
	virtual ICKSTRING GetAuthor() override { return "dn0pf902"; }
	virtual ICKSTRING GetDescription() override { return "An auxiliary mod for TAS making."; }
	DECLARE_BML_VERSION;

	virtual void OnStartLevel() override;
	virtual void OnProcess() override;
	virtual void OnLoad() override;
	virtual void OnLoadScript(const char* filename, CKBehavior* script) override;
	virtual void OnPreLoadLevel() override;
	virtual void OnPreExitLevel() override;

	void GetUpdateRuleList();
	void ShowData();
	void HideData();
	int cmp(int frame, VxVector cur_pos, VxVector cur_vel) const;
	void update_data(int frame, VxVector cur_pos, VxVector cur_vel);
	void SaveFile();
	void AutoSaveFile();

	CK3dEntity* GetActiveBall() const {
		if (m_ActiveBall)
			return (CK3dEntity*)m_ActiveBall->GetValueObject();
		return nullptr;
	}

	VxVector get_ball_vel() {
		auto* ball = GetActiveBall();
		if (!ball) return VxVector(-1, -1, -1);
		VxVector vel{}, angular_vel{};
		auto* obj = m_IpionManager->GetPhysicsObject(ball);
		if (obj) {
			obj->GetVelocity(&vel, &angular_vel);
		}
		return vel;
	}

	VxVector get_ball_pos() {
		auto* ball = GetActiveBall();
		if (!ball) return VxVector(-1, -1, -1);
		VxVector pos{}, angles{};
		auto* obj = m_IpionManager->GetPhysicsObject(ball);
		if (obj) {
			obj->GetPosition(&pos, &angles);
		}
		return pos;
	}
};