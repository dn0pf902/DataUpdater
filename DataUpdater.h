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

extern "C" {
	__declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

class DataUpdater : public IMod {
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
};