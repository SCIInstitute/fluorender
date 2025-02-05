#ifndef _DL_TEST_H_
#define _DL_TEST_H_
#include <string>
#include <dlib/dnn.h>

using net_type = dlib::loss_metric<dlib::fc<2, dlib::input<dlib::matrix<double, 0, 1>>>>;
extern net_type net;
extern dlib::dnn_trainer<net_type> *trainer;

void DLTest();

#define gno_vp_output_size 26
#define gno_vp_input_size 15

//adjust net definition to change behaviors
using net_type_vp =
	dlib::loss_mean_squared_multioutput<
	dlib::fc<gno_vp_output_size,
	dlib::input<dlib::matrix<float>>>>;
	//dlib::relu<dlib::fc<gno_vp_input_size,
	//dlib::input<dlib::matrix<float>>
	//>>>>;

void DLTest2();
//class DTrainer
//{
//public :
//	DTrainer();
//	~DTrainer();
//
//	void add(float*, float*);
//	void train();
//	float* infer(float*);
//	double get_rate();
//	void set_model_file(const std::string& file);
//
//protected:
//	bool m_valid;
//	size_t m_trained_rec_num;
//	std::string m_model_file;
//
//	net_type_vp m_net;
//	dlib::dnn_trainer<net_type_vp> m_trainer;
//	dlib::matrix<float> m_result;
//
//	std::vector<dlib::matrix<float>> m_input;
//	std::vector<dlib::matrix<float>> m_output;
//};

#endif