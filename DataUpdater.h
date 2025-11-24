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
	std::unique_ptr<BGui::Text> sprite;
	bool enabled = false;
	CKParameter* m_ActiveBall = nullptr;
	IProperty* prop_enabled = nullptr;
	IProperty* prop_key = nullptr;
	CKKEYBOARD hotkey = {};
	InputHook* input_manager = nullptr;
	CKIpionManager* m_IpionManager = nullptr;
	
	int frame_cnt = 0;
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
	virtual void OnBallNavActive() override;
	virtual void OnPreLoadLevel() override;
	
	void OnPrintData() {
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
			sprite->SetFont("Consolas", 20, 400, false, false);
		}

		char txt[128];
		std::snprintf(txt, sizeof(txt), "frame cnt = %d", frame_cnt);
		sprite->SetText(txt);
	}

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