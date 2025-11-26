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

extern "C" {
	__declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

class DataUpdater : public IMod {
private:
	std::unique_ptr<BGui::Panel> bg;
	std::unique_ptr<BGui::Text> sprite_data, sprite_cur_data;
	bool enabled = false;
	bool hotkey_enabled = false;
	CKParameter* m_ActiveBall = nullptr;
	IProperty* prop_enabled = nullptr;
	IProperty* prop_hotkey = nullptr;
	IProperty* prop_hotkey_enabled = nullptr;
	IProperty* prop_update_frame = nullptr;
	IProperty* prop_update_direction = nullptr;
	IProperty* prop_frame_of_data = nullptr;
	IProperty* prop_data_pos = nullptr, * prop_data_vel = nullptr;
	IProperty* prop_preserved_data = nullptr;
	IProperty* prop_dlt_vel = nullptr, * prop_dlt_pos = nullptr;
	IProperty* prop_UI_posx = nullptr, * prop_UI_posy = nullptr, * prop_UI_font = nullptr, * prop_UI_font_size = nullptr;
	IProperty* prop_UI_sizex = nullptr, * prop_UI_sizey = nullptr;
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
	std::string preserved_data = "";
	float data_pos = 0.0f, data_vel = 0.0f;
	float dlt_pos = 0.1f, dlt_vel = 0.1f;
public:
	DataUpdater(IBML* bml) : IMod(bml) {}

	virtual ICKSTRING GetID() override { return "DataUpdater"; }
	virtual ICKSTRING GetVersion() override { return "0.1.0"; }
	virtual ICKSTRING GetName() override { return "DataUpdater"; }
	virtual ICKSTRING GetAuthor() override { return "dn0pf902"; }
	virtual ICKSTRING GetDescription() override { return "1"; }
	DECLARE_BML_VERSION;

	virtual void OnStartLevel() override;
	virtual void OnProcess() override;
	virtual void OnLoad() override;
	virtual void OnPostLoadLevel() override;
	virtual void OnLoadScript(const char* filename, CKBehavior* script) override;
	virtual void OnPreLoadLevel() override;
	virtual void OnPreExitLevel() override;
	
	void ShowData();
	void HideData();
	bool cmp(VxVector cur_pos, VxVector cur_vel) const;
	void update_data(int frame, VxVector cur_pos, VxVector cur_vel);

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