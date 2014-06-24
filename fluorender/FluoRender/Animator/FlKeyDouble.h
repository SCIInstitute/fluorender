#ifndef _FLKEY_DOUBLE_H_
#define _FLKEY_DOUBLE_H_

#include "FlKey.h"

class FlKeyDouble : public FlKey
{
public:
	FlKeyDouble()
	{
		m_code.l0 = 0;
		m_code.l0_name = "";
		m_code.l1 = 0;
		m_code.l1_name = "";
		m_code.l2 = 0;
		m_code.l2_name = "";
		m_dval = 0.0;
	}
	FlKeyDouble(KeyCode keycode, double dval)
	{
		m_code = keycode;
		m_dval = dval;
	}
	~FlKeyDouble() {}

	int GetType() {return FLKEY_TYPE_DOUBLE;}
	void SetValue(double dval) {m_dval = dval;}
	double GetValue() {return m_dval;}

private:
	double m_dval;
};

#endif//_FLKEY_DOUBLE_H_
