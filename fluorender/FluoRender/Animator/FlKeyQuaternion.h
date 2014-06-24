#ifndef _FLKEY_QUATERNION_H_
#define _FLKEY_QUATERNION_H_

#include "FlKey.h"
#include "../FLIVR/Quaternion.h"

using namespace FLIVR;

class FlKeyQuaternion : public FlKey
{
public:
	FlKeyQuaternion()
	{
		m_code.l0 = 0;
		m_code.l0_name = "";
		m_code.l1 = 0;
		m_code.l1_name = "";
		m_code.l2 = 0;
		m_code.l2_name = "";
	}
	FlKeyQuaternion(KeyCode keycode, Quaternion &qval)
	{
		m_code = keycode;
		m_qval = qval;
	}
	~FlKeyQuaternion() {}

	int GetType() {return FLKEY_TYPE_QUATER;}
	void SetValue(Quaternion &qval) {m_qval = qval;}
	Quaternion GetValue() {return m_qval;}

private:
	Quaternion m_qval;
};

#endif//_FLKEY_QUATERNION_H_
