#include "DataManager.h"

#ifndef _VOLUMECALCULATOR_H_
#define _VOLUMECALCULATOR_H_

class VolumeCalculator
{
public:
	VolumeCalculator();
	~VolumeCalculator();

	void SetVolumeA(VolumeData *vd);
	void SetVolumeB(VolumeData *vd);

	VolumeData* GetVolumeA();
	VolumeData* GetVolumeB();
	VolumeData* GetResult();

	void Calculate(int type);

private:
	VolumeData *m_vd_r;	//result volume data

	VolumeData *m_vd_a;	//volume data A
	VolumeData *m_vd_b;	//volume data B

	int m_type;	//calculation type
				//1:substraction;
				//2:addition;
				//3:division;
				//4:intersection
				//5:apply mask (single volume multiplication)
				//6:apply mask inverted (multiplication with mask's complement in volume)
				//7:apply mask inverted, then replace volume a
				//8:intersection with masks if available
				//9:fill holes

private:
	void CreateVolumeResult1();//create the resulting volume from one input
	void CreateVolumeResult2();//create the resulting volume from two inputs

	//fill holes
	void FillHoles();
};

#endif//_VOLUMECALCULATOR_H_