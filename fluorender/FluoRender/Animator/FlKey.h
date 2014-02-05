#ifndef _FLKEY_H_
#define _FLKEY_H_

#include <string>

using namespace std;

#define FLKEY_TYPE_DOUBLE	1
#define FLKEY_TYPE_QUATER	2
#define FLKEY_TYPE_BOOLEAN	3

class KeyCode
{
public:
	int l0;//view: 1
	string l0_name;//view name
	int l1;//volume: 2
	string l1_name;//volume name
	int l2;//volume property: 0
	string l2_name;//volume property name

	KeyCode& operator=(const KeyCode& keycode)
	{
		l0 = keycode.l0;
		l0_name = keycode.l0_name;
		l1 = keycode.l1;
		l1_name = keycode.l1_name;
		l2 = keycode.l2;
		l2_name = keycode.l2_name;
		return *this;
	}

	int operator==(const KeyCode& keycode) const
	{
		return l0==keycode.l0 &&
			l0_name==keycode.l0_name &&
			l1==keycode.l1 &&
			l1_name==keycode.l1_name &&
			l2==keycode.l2 &&
			l2_name==keycode.l2_name;
	}

	int operator!=(const KeyCode& keycode) const
	{
		return l0!=keycode.l0 ||
			l0_name!=keycode.l0_name ||
			l1!=keycode.l1 ||
			l1_name!=keycode.l1_name ||
			l2!=keycode.l2 ||
			l2_name!=keycode.l2_name;
	}
};

//virtual
class FlKey
{
public:
	//FlKey();
	virtual ~FlKey() {};

	virtual int GetType() = 0;
	KeyCode GetKeyCode()
	{return m_code;}

protected:
	KeyCode m_code;
};

#endif//_FLKEY_H_