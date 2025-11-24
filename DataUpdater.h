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

extern "C" {
	__declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

class DataUpdater : public IMod {
private:
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