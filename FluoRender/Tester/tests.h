#pragma once

void ObjectTest();
void ObjectTest2();
void ObjectTest3();
void ObjectTest4();

void GroupTest();
void GroupTest2();
void GroupTest3();
void GroupTest4();

void SpecialValueTest();

void MyTest();

void TableTest();

void PythonTest0();
#include <string>
#include <vector>
void PythonTest1(const std::string& config, const std::string& video);
void PythonTest2(const std::string& config, const std::string& video);

void OpenCVTest0();

void WalkCycleInit(const std::string& file, int l, int r, size_t ol);
void WalkCycleRefine(const std::string& datafile, const std::string& cyclefile, size_t ol);
void WalkCycleAvg(const std::string& f1, const std::string& f2, const std::string& fo);
void WalkCycleCompare(const std::string& datafile, const std::string& cyclefile, size_t ol);
void PhaseGraph(const std::string& infile, const std::string& cyclefile, size_t ol);
void ComputeVariance(const std::string& cycle_file, const std::vector<std::string>& sample_list, size_t ol);
void ComputeTime(const std::string& cycle_file, const std::vector<std::string>& sample_list, size_t ol, double d);
