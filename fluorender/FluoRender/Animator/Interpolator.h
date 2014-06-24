#ifndef _INTERPOLATOR_H_
#define _INTERPOLATOR_H_

#include <vector>
#include <string>
#include "FlKey.h"
#include "FlKeyDouble.h"
#include "FlKeyQuaternion.h"
#include "FlKeyBoolean.h"
#include "../FLIVR/Quaternion.h"

using namespace std;
using namespace FLIVR;

typedef struct
{
	int id;		//identifier
	double t;	//time: 0-linear; 1-spline
	int type;	//interpolation method
	vector<FlKey*> keys; //keys
	string desc;//descriptions
} FlKeyGroup;

class Interpolator
{
public:
	Interpolator();
	~Interpolator();

	//create
	//return group id
	int Begin(double t);
	//return successfulness
	int AddKey(FlKey *key);
	//return completeness
	int End();

	//get info
	int GetKeyNum();
	double GetFirstT();
	double GetLastT();
	FlKeyGroup* GetKeyGroup(int index);
	int GetKeyIndex(int id);
	int GetKeyIndexFromTime(double t);
	int GetKeyID(int index);
	double GetKeyTime(int index);
	double GetKeyDuration(int index);
	int GetKeyType(int index);
	string GetKeyDesc(int index);
	int GetLastIndex()
	{ return (int)m_key_list.size() - 1;}
	vector<FlKeyGroup*>* GetKeyList()
	{ return &m_key_list; }

	//modify
	void Clear();
	void RemoveKey(int id);
	void ChangeTime(int index, double time);
	void ChangeDuration(int index, double duration);
	void MoveKeyBefore(int from_idx, int to_idx);
	void MoveKeyAfter(int from_idx, int to_idx);

	//get values
	bool GetDouble(KeyCode keycode, double t, double &dval);
	bool GetQuaternion(KeyCode keycode, double t, Quaternion &qval);
	bool GetBoolean(KeyCode keycode, double t, bool &bval);

	static int m_id;

private:
	//adding: 0-disabled; 1-enabled
	int m_adding;
	vector<FlKeyGroup*> m_key_list;

private:
	FlKey* SearchKey(KeyCode keycode, FlKeyGroup* g);
	bool StepDouble(KeyCode keycode, FlKeyGroup* g, double &dval);
	bool LinearDouble(KeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, double &dval);
	bool StepQuaternion(KeyCode keycode, FlKeyGroup* g, Quaternion &qval);
	bool LinearQuaternion(KeyCode keycode, FlKeyGroup* g1, FlKeyGroup* g2, double t, Quaternion &qval);
	double Smooth(double ft, bool s1, bool s2);

};

#endif//_INTERPOLATOR_H_
