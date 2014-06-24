#ifndef _FLKEY_BOOLEAN_H_
#define _FLKEY_BOOLEAN_H_

#include "FlKey.h"

class FlKeyBoolean : public FlKey
{
public:
	FlKeyBoolean()
	{
		m_code.l0 = 0;
		m_code.l0_name = "";
		m_code.l1 = 0;
		m_code.l1_name = "";
		m_code.l2 = 0;
		m_code.l2_name = "";
		m_bval = false;
	}
	FlKeyBoolean(KeyCode keycode, bool bval)
	{
		m_code = keycode;
		m_bval = bval;
	}
	~FlKeyBoolean() {}

	int GetType() {return FLKEY_TYPE_BOOLEAN;}
	void SetValue(bool bval) {m_bval = bval;}
	bool GetValue() {return m_bval;}

private:
	bool m_bval;
};

#endif//_FLKEY_BOOLEAN_H_
