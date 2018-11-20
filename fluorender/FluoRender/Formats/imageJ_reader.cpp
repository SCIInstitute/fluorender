/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include "imageJ_reader.h"
#include "../compatibility.h"
#include <wx/stdpaths.h>

ImageJReader::ImageJReader()
{
	m_pJVMInstance = 0;
	m_imageJ_cls = 0;

	m_resize_type = 0;
	m_resample_type = 0;
	m_alignment = 0;

	m_slice_seq = false;
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;
	m_eight_bit = true;

	m_valid_spc = false;
	m_xspc = 1.0;
	m_yspc = 1.0;
	m_zspc = 1.0;
	m_excitation_wavelength = nullptr;

	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_batch = false;
	m_cur_batch = -1;

	m_time_id = L"_T";
	
	//Geting absolute path to class file.
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	string imageJPath = exePath + GETSLASH() + "Java_Code" + GETSLASH() + "ImageJ_Reader";

	//Java code to get the number of depth images.
	m_pJVMInstance = JVMInitializer::getInstance();
	if (m_pJVMInstance == nullptr)
		return;
	//printf("%s", imageJPath.c_str());
	//fflush(stdout);

	m_imageJ_cls = m_pJVMInstance->m_pEnv->FindClass("ImageJ_Reader");
	if (m_imageJ_cls == nullptr) {
        m_pJVMInstance->m_pEnv->ExceptionDescribe();
		cerr << "ERROR: class not found !";
	}
}

ImageJReader::~ImageJReader()
{
	//if (tiff_stream.is_open())
	//	tiff_stream.close();
}

void ImageJReader::SetFile(string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
	m_id_string = m_path_name;
}

void ImageJReader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;	
}

int ImageJReader::Preprocess()
{
	int return_result = READER_OK;
	//separate path and name
	wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;
	m_data_name = name;	

	// ImageJ code here..................
	if (m_imageJ_cls == nullptr) {
		cerr << "ERROR: class not found !";
	}
	else {
		// getting the image metadata.
		jmethodID method_handle = m_pJVMInstance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getMetaData", "([Ljava/lang/String;)[I");
		if (method_handle == nullptr)
			cerr << "ERROR: method void getDepth() not found !" << endl;
		else {
			// This part goes in setFile.
			jobjectArray arr = m_pJVMInstance->m_pEnv->NewObjectArray(2,      // constructs java array of 2
				m_pJVMInstance->m_pEnv->FindClass("java/lang/String"),    // Strings
				m_pJVMInstance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
			
			//char* cstr = new char[m_path_name.length() + 1];
			//sprintf(cstr, "%ws", m_path_name.c_str());
			string path_name = ws2s(m_path_name);

			m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 0, m_pJVMInstance->m_pEnv->NewStringUTF(const_cast<char*>(path_name.c_str())));  // change an element
			//m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 1, m_pJVMInstance->m_pEnv->NewStringUTF("4D_1ch.lsm"));  // change an element
			//jint depth = (jint)(m_pJVMInstance->m_pEnv->CallStaticIntMethod(imageJ_cls, mid, arr));   // call the method with the arr as argument.
			//m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);     // release the object

			jintArray val = (jintArray)(m_pJVMInstance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_handle, arr));   // call the method with the arr as argument.					
			jsize len = m_pJVMInstance->m_pEnv->GetArrayLength(val);
			jint* body = m_pJVMInstance->m_pEnv->GetIntArrayElements(val, 0);

			//Checking if the right format was loaded.
			if (len == 1) {
				int test = *body;
				if (test == 4)
					return_result = READER_FORMAT_ERROR;
				else if (test == 5)
					return_result = READER_OPEN_FAIL;
				else if (test == 7)
					return READER_JAVA_ARRAY_SIZE_ERROR;
				else
					return_result = READER_EMPTY_DATA; //This is for unkown exception on java side.
			}
			else
			{
				for (int i = 0; i < len; i++) {
					int test = *(body + i);
					switch (i)
					{
					case 0:
						break;
					case 1:
						m_x_size = test;
						break;
					case 2:
						m_y_size = test;
						break;
					case 3:
						m_slice_num = test;
						break;
					case 4:
						m_chan_num = test;
						m_excitation_wavelength = new int[m_chan_num];
						break;
					case 5:
						m_time_num = test;
						break;				
					case 6:
						if (test == 1)
							m_eight_bit = true;
						else
							m_eight_bit = false;
						break;
					case 7:						
						m_xspc = test;
						break;
					case 8:
						m_xspc += (test*0.0001);
						break;
					case 9:
						m_yspc = test;
						break;
					case 10:
						m_yspc += (test*0.0001);
						break;
					case 11:
						m_zspc = test;
						break;
					case 12:
						m_zspc += (test*0.0001);
						if (!double_equals(m_xspc, 0.0) && !double_equals(m_yspc, 0.0) && !double_equals(m_zspc, 0.0))
						{
							m_valid_spc = true;
						}						
						break;
					default:
						if (i >= 13 && i <= (13 + m_chan_num - 1)) {
							if(test == -1)
								m_excitation_wavelength[i - 13] = 0.0;
							else	
								m_excitation_wavelength[i - 13] = test;
						}						
						break;
					// TODO: What is m_max_value.
					}
				}
			}

			// release the object
			m_pJVMInstance->m_pEnv->ReleaseIntArrayElements(val, body, JNI_ABORT);
			m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);
			m_pJVMInstance->m_pEnv->DeleteLocalRef(val);
		}
	}
	m_cur_time = 0;	
	
	return return_result;
}

void ImageJReader::SetSliceSeq(bool ss)
{
	//enable searching for slices
	m_slice_seq = ss;
}

bool ImageJReader::GetSliceSeq()
{
	return m_slice_seq;
}

void ImageJReader::SetTimeId(wstring &id)
{
	m_time_id = id;
}

wstring ImageJReader::GetTimeId()
{
	return m_time_id;
}

wstring ImageJReader::GetCurDataName(int t, int c)
{
	wstring path, name;
	SEP_PATH_NAME(m_path_name, path, name);
	return name;
}

wstring ImageJReader::GetCurMaskName(int t, int c)
{
	wstring path, name;
	SEP_PATH_NAME(m_path_name, path, name);
	wstring mask_name = name.substr(0, name.find_last_of('.')) + L".msk";
	return mask_name;
}

wstring ImageJReader::GetCurLabelName(int t, int c)
{
	wstring path, name;
	SEP_PATH_NAME(m_path_name, path, name);
	wstring label_name = name.substr(0, name.find_last_of('.')) + L".lbl";
	return label_name;
}

void ImageJReader::SetBatch(bool batch)
{
	//if (batch)
	//{
	//	//separate path and name
	//	wstring search_path = GET_PATH(m_path_name);
	//	wstring suffix = GET_SUFFIX(m_path_name);
	//	FIND_FILES(search_path, suffix, m_batch_list, m_cur_batch);
	//	m_batch = true;
	//}
	//else
		m_batch = false;
}

int ImageJReader::LoadBatch(int index)
{
	int result = -1;
	//if (index >= 0 && index < (int)m_batch_list.size())
	//{
	//	m_path_name = m_batch_list[index];
	//	Preprocess();
	//	result = index;
	//	m_cur_batch = result;
	//}
	//else
	//	result = -1;

	return result;
}

Nrrd* ImageJReader::Convert(int t, int c, bool get_max)
{	
	if (t < 0 || t >= m_time_num || c < 0 || c >= m_chan_num)
		return 0;

	//if (isHyperstack_ && m_max_value)
	//	get_max = false;

	Nrrd* data = 0;
	//TODO: Fix the m_data_name.
	//TimeDataInfo chan_info = m_4d_seq[t];
	//if (!isHyperstack_ || isHsTimeSeq_)
		//m_data_name = GET_NAME(chan_info.slices[0].slice);
	
	data = ReadFromImageJ(t, c, get_max);
	m_cur_time = t;
	return data;
}

Nrrd* ImageJReader::ReadFromImageJ(int t, int c, bool get_max) {	
	// ImageJ code to read the data.	
	//char* path_cstr = new char[m_path_name.length() + 1];
	//sprintf(path_cstr, "%ws", m_path_name.c_str());
	string path_name = ws2s(m_path_name);

	jmethodID method_id = NULL;
	if (m_eight_bit == true){
		method_id = m_pJVMInstance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getByteData2D", "([Ljava/lang/String;II)[[B");
	}
	else {
		method_id = m_pJVMInstance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getIntData2D", "([Ljava/lang/String;II)[[S");
	}
	
	void* t_data = NULL;
	if (method_id == nullptr) {
		cerr << "ERROR: method void mymain() not found !" << endl;
		return NULL;
	}
	else if (m_eight_bit == true){
		jobjectArray arr = m_pJVMInstance->m_pEnv->NewObjectArray(2,      // constructs java array of 3
			m_pJVMInstance->m_pEnv->FindClass("java/lang/String"),    // Strings
			m_pJVMInstance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
		m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 0, m_pJVMInstance->m_pEnv->NewStringUTF(const_cast<char*>(path_name.c_str())));  // change an element		

		jobjectArray  val = (jobjectArray)(m_pJVMInstance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_id, arr, (jint)t, (jint)c));   // call the method with the arr as argument.
		//jboolean flag = m_pJVMInstance->m_pEnv->ExceptionCheck();
		//if (flag) {
		//	m_pJVMInstance->m_pEnv->ExceptionClear();
		//	//TODO: code to handle exception.
		//}

		jsize len = m_pJVMInstance->m_pEnv->GetArrayLength(val);
		if (len > 1)
		{
			int offset = 0;			
			for (int i = 0; i < len; i++) {				
				jbyteArray inner_data = static_cast<jbyteArray>(m_pJVMInstance->m_pEnv->GetObjectArrayElement(val, i));
				jsize len2 = m_pJVMInstance->m_pEnv->GetArrayLength(inner_data);				
				offset = i*len2;
				if (t_data == NULL)
					t_data = new unsigned char[len*len2];

				jbyte* body = (jbyte*)(m_pJVMInstance->m_pEnv->GetByteArrayElements(inner_data, 0));
				for (int j = 0; j < len2; ++j) {
					int test = *(body + j);					
					*((unsigned char*)t_data + offset + j) = test;
				}
				m_pJVMInstance->m_pEnv->ReleaseByteArrayElements(inner_data, body, JNI_ABORT);						
			}			
		}
		m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);
		m_pJVMInstance->m_pEnv->DeleteLocalRef(val);
	}
	else if (m_eight_bit == false)
	{
		//m_pJVMInstance->m_pEnv->PushLocalFrame(1000);
		jobjectArray arr = m_pJVMInstance->m_pEnv->NewObjectArray(2,      // constructs java array of 3
			m_pJVMInstance->m_pEnv->FindClass("java/lang/String"),    // Strings
			m_pJVMInstance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
		m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 0, m_pJVMInstance->m_pEnv->NewStringUTF(const_cast<char*>(path_name.c_str())));  // change an element		
		jobjectArray val = (jobjectArray)(m_pJVMInstance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_id, arr, (jint)t, (jint)c));   // call the method with the arr as argument.
		jsize len = m_pJVMInstance->m_pEnv->GetArrayLength(val);
		if (len > 1)
		{
			int offset = 0;
			for (int i = 0; i < len; i++) {
				jshortArray inner_data = static_cast<jshortArray>(m_pJVMInstance->m_pEnv->GetObjectArrayElement(val, 0));
				jsize len2 = m_pJVMInstance->m_pEnv->GetArrayLength(inner_data);
				offset = i*len2;
				if (t_data == NULL)
					t_data = t_data = new unsigned short int[len*len2];

				jshort* body = (jshort*)(m_pJVMInstance->m_pEnv->GetShortArrayElements(inner_data, 0));
				for (int j = 0; j < len2; ++j) {
					int test = *(body + j);
					*((unsigned short int*)t_data + offset + j) = test;
				}
				m_pJVMInstance->m_pEnv->ReleaseShortArrayElements(inner_data, body, JNI_ABORT);
			}
			/*
			jshort* body = m_pJVMInstance->m_pEnv->GetShortArrayElements(val, 0);
			unsigned short int* dummy = reinterpret_cast<unsigned short int*>(body);
			t_data = new unsigned short int[len];
			for (int i = 0; i < len; i++) {
				int test = *(body + i);
				*((unsigned short int*)t_data + i) = test;
			}
			m_pJVMInstance->m_pEnv->ReleaseShortArrayElements(val, body, JNI_ABORT);
			*/
		}
		m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);
		m_pJVMInstance->m_pEnv->DeleteLocalRef(val);
	}

	// Creating Nrrd out of the data.
	Nrrd *nrrdout = nrrdNew();	
	
	int numPages = m_slice_num;	
	unsigned long long total_size = (unsigned long long)m_x_size*(unsigned long long)m_y_size*(unsigned long long)numPages;	
	if (!t_data)
		throw std::runtime_error("No data received from imageJ.");
	
	if (m_eight_bit)
		nrrdWrap(nrrdout, (uint8_t*)t_data, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	else
		nrrdWrap(nrrdout, (uint16_t*)t_data, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);

	if (!m_eight_bit) {
		if (get_max) {
			double value;
			unsigned long long totali = (unsigned long long)m_slice_num*
				(unsigned long long)m_x_size*(unsigned long long)m_y_size;
			for (unsigned long long i = 0; i < totali; ++i)
			{
				value = ((unsigned short*)nrrdout->data)[i];
				m_max_value = value > m_max_value ? value : m_max_value;
			}			
		}
		if (m_max_value > 0.0) 
			m_scalar_scale = 65535.0 / m_max_value;
		else 
			m_scalar_scale = 1.0;		
	}
	else 
		m_max_value = 255.0;

	return nrrdout;
}
