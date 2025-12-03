/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <imageJ_reader.h>
#include <Global.h>
#include <JVMInitializer.h>
#include <compatibility.h>

ImageJReader::ImageJReader():
	BaseVolReader()
{
	m_imageJ_cls = 0;

	m_resize_type = 0;
	m_resample_type = 0;
	m_alignment = 0;

	m_slice_seq = false;
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_eight_bit = true;

	m_valid_spc = false;
	m_spacing = fluo::Vector(1.0);
	m_excitation_wavelength = nullptr;

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;

	m_time_id = L"_T";
	
	//Java code to get the number of depth images.
	if (glbin_jvm_instance->IsValid())
	{
		m_imageJ_cls = glbin_jvm_instance->m_pEnv->FindClass("ImageJ_Reader");
		if (m_imageJ_cls == nullptr)
		{
			glbin_jvm_instance->m_pEnv->ExceptionDescribe();
			std::cerr << "ERROR: class not found !";
		}
	}
}

ImageJReader::~ImageJReader()
{
	//if (tiff_stream.is_open())
	//	tiff_stream.close();
}

//void ImageJReader::SetFile(const std::string &file)
//{
//	if (!file.empty())
//	{
//		if (!m_path_name.empty())
//			m_path_name.clear();
//		m_path_name.assign(file.length(), L' ');
//		copy(file.begin(), file.end(), m_path_name.begin());
//	}
//	m_id_string = m_path_name;
//}

void ImageJReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;	
}

int ImageJReader::Preprocess()
{
	int return_result = READER_OK;
	//separate path and name
	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;
	m_data_name = name;	

	// ImageJ code here..................
	if (m_imageJ_cls == nullptr) {
		std::cerr << "ERROR: class not found !";
	}
	else {
		// getting the image metadata.
		jmethodID method_handle = glbin_jvm_instance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getMetaData", "([Ljava/lang/String;)[I");
		if (method_handle == nullptr)
			std::cerr << "ERROR: method void getDepth() not found !" << std::endl;
		else {
			// This part goes in setFile.
			jobjectArray arr = glbin_jvm_instance->m_pEnv->NewObjectArray(2,      // constructs java array of 2
				glbin_jvm_instance->m_pEnv->FindClass("java/lang/String"),    // Strings
				glbin_jvm_instance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
			
			//char* cstr = new char[m_path_name.length() + 1];
			//sprintf(cstr, "%ws", m_path_name.c_str());
			std::string path_name = ws2s(m_path_name);

			glbin_jvm_instance->m_pEnv->SetObjectArrayElement(arr, 0, glbin_jvm_instance->m_pEnv->NewStringUTF(const_cast<char*>(path_name.c_str())));  // change an element
			//glbin_jvm_instance->m_pEnv->SetObjectArrayElement(arr, 1, glbin_jvm_instance->m_pEnv->NewStringUTF("4D_1ch.lsm"));  // change an element
			//jint depth = (jint)(glbin_jvm_instance->m_pEnv->CallStaticIntMethod(imageJ_cls, mid, arr));   // call the method with the arr as argument.
			//glbin_jvm_instance->m_pEnv->DeleteLocalRef(arr);     // release the object

			jintArray val = (jintArray)(glbin_jvm_instance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_handle, arr));   // call the method with the arr as argument.					
			if (!val)
				return READER_OPEN_FAIL;
			jsize len = glbin_jvm_instance->m_pEnv->GetArrayLength(val);
			jint* body = glbin_jvm_instance->m_pEnv->GetIntArrayElements(val, 0);

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
						m_size.x(test);
						break;
					case 2:
						m_size.y(test);
						break;
					case 3:
						m_size.z(test);
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
						m_spacing.x(test);
						break;
					case 8:
						m_spacing.x(m_spacing.x() + test * 0.0001);
						break;
					case 9:
						m_spacing.y(test);
						break;
					case 10:
						m_spacing.y(m_spacing.y() + test*0.0001);
						break;
					case 11:
						m_spacing.z(test);
						break;
					case 12:
						m_spacing.z(m_spacing.z() + test*0.0001);
						if (!m_spacing.any_le_zero())
						{
							m_valid_spc = true;
						}						
						break;
					default:
						if (i >= 13 && i <= (13 + m_chan_num - 1)) {
							if(test == -1)
								m_excitation_wavelength[i - 13] = 0;
							else	
								m_excitation_wavelength[i - 13] = test;
						}						
						break;
					}
				}
			}

			// release the object
			glbin_jvm_instance->m_pEnv->ReleaseIntArrayElements(val, body, JNI_ABORT);
			glbin_jvm_instance->m_pEnv->DeleteLocalRef(arr);
			glbin_jvm_instance->m_pEnv->DeleteLocalRef(val);
		}
	}
	m_cur_time = 0;	
	
	return return_result;
}

std::wstring ImageJReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring ImageJReader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring ImageJReader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
	return label_name;
}

void ImageJReader::SetBatch(bool batch)
{
	//if (batch)
	//{
	//	//separate path and name
	//	wstring search_path = GET_PATH(m_path_name);
	//	wstring suffix = GET_SUFFIX(m_path_name);
	//	FIND_FILES_BATCH(search_path, ESCAPE_REGEX(suffix), m_batch_list, m_cur_batch);
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
		//m_data_name = GET_STEM(chan_info.slices[0].slice);
	
	data = ReadFromImageJ(t, c, get_max);
	m_cur_time = t;
	return data;
}

Nrrd* ImageJReader::ReadFromImageJ(int t, int c, bool get_max) {	
	// ImageJ code to read the data.
	std::string path_name = ws2s(m_path_name);

	jmethodID method_id = NULL;
	if (m_eight_bit == true){
		method_id = glbin_jvm_instance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getByteData2D", "([Ljava/lang/String;II)[[B");
	}
	else {
		method_id = glbin_jvm_instance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getIntData2D", "([Ljava/lang/String;II)[[S");
	}
	
	void* t_data = NULL;
	if (method_id == nullptr) {
		std::cerr << "ERROR: method void mymain() not found !" << std::endl;
		return NULL;
	}
	else if (m_eight_bit == true){
		jobjectArray arr = glbin_jvm_instance->m_pEnv->NewObjectArray(2,      // constructs java array of 3
			glbin_jvm_instance->m_pEnv->FindClass("java/lang/String"),    // Strings
			glbin_jvm_instance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
		glbin_jvm_instance->m_pEnv->SetObjectArrayElement(arr, 0, glbin_jvm_instance->m_pEnv->NewStringUTF(const_cast<char*>(path_name.c_str())));  // change an element		

		jobjectArray  val = (jobjectArray)(glbin_jvm_instance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_id, arr, (jint)t, (jint)c));   // call the method with the arr as argument.
		//jboolean flag = glbin_jvm_instance->m_pEnv->ExceptionCheck();
		//if (flag) {
		//	glbin_jvm_instance->m_pEnv->ExceptionClear();
		//	//TODO: code to handle exception.
		//}

		jsize len = glbin_jvm_instance->m_pEnv->GetArrayLength(val);
		if (len >= 1)
		{
			unsigned long long offset = 0;			
			for (unsigned long long i = 0; i < len; i++) {
				jbyteArray inner_data = static_cast<jbyteArray>(glbin_jvm_instance->m_pEnv->GetObjectArrayElement(val, (jsize)i));
				jsize len2 = glbin_jvm_instance->m_pEnv->GetArrayLength(inner_data);				
				offset = i*len2;
				if (t_data == NULL)
					t_data = new unsigned char[(unsigned long long)len*len2];

				jbyte* body = (jbyte*)(glbin_jvm_instance->m_pEnv->GetByteArrayElements(inner_data, 0));
				for (unsigned long long j = 0; j < len2; ++j) {
					int test = *(body + j);					
					((unsigned char*)t_data)[offset + j] = test;
				}
				glbin_jvm_instance->m_pEnv->ReleaseByteArrayElements(inner_data, body, JNI_ABORT);						
			}			
		}
		else {
			jshortArray inner_data = static_cast<jshortArray>(glbin_jvm_instance->m_pEnv->GetObjectArrayElement(val, 0));
			jshort* body = (jshort*)(glbin_jvm_instance->m_pEnv->GetShortArrayElements(inner_data, 0));
			int test = *(body);
			std::cout << "Error";
		}
		glbin_jvm_instance->m_pEnv->DeleteLocalRef(arr);
		glbin_jvm_instance->m_pEnv->DeleteLocalRef(val);
	}
	else if (m_eight_bit == false)
	{
		//glbin_jvm_instance->m_pEnv->PushLocalFrame(1000);
		jobjectArray arr = glbin_jvm_instance->m_pEnv->NewObjectArray(2,      // constructs java array of 3
			glbin_jvm_instance->m_pEnv->FindClass("java/lang/String"),    // Strings
			glbin_jvm_instance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
		glbin_jvm_instance->m_pEnv->SetObjectArrayElement(arr, 0, glbin_jvm_instance->m_pEnv->NewStringUTF(const_cast<char*>(path_name.c_str())));  // change an element		
		jobjectArray val = (jobjectArray)(glbin_jvm_instance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_id, arr, (jint)t, (jint)c));   // call the method with the arr as argument.
		jsize len = glbin_jvm_instance->m_pEnv->GetArrayLength(val);

		if (len >= 1)
		{
			unsigned long long offset = 0;
			for (unsigned long long i = 0; i < len; i++) {
				jshortArray inner_data = static_cast<jshortArray>(glbin_jvm_instance->m_pEnv->GetObjectArrayElement(val, (jsize)i));
				jsize len2 = glbin_jvm_instance->m_pEnv->GetArrayLength(inner_data);
				offset = i*len2;
				if (t_data == NULL)
					t_data = t_data = new unsigned short int[(unsigned long long)len*len2];

				jshort* body = (jshort*)(glbin_jvm_instance->m_pEnv->GetShortArrayElements(inner_data, 0));
				for (unsigned long long j = 0; j < len2; ++j) {
					int test = *(body + j);
					*((unsigned short int*)t_data + offset + j) = test;
				}
				glbin_jvm_instance->m_pEnv->ReleaseShortArrayElements(inner_data, body, JNI_ABORT);
			}
			/*
			jshort* body = glbin_jvm_instance->m_pEnv->GetShortArrayElements(val, 0);
			unsigned short int* dummy = reinterpret_cast<unsigned short int*>(body);
			t_data = new unsigned short int[len];
			for (int i = 0; i < len; i++) {
				int test = *(body + i);
				*((unsigned short int*)t_data + i) = test;
			}
			glbin_jvm_instance->m_pEnv->ReleaseShortArrayElements(val, body, JNI_ABORT);
			*/
		}
		else {
			jshortArray inner_data = static_cast<jshortArray>(glbin_jvm_instance->m_pEnv->GetObjectArrayElement(val, 0));
			jshort* body = (jshort*)(glbin_jvm_instance->m_pEnv->GetShortArrayElements(inner_data, 0));
			int test = *(body);
			std::cout << "Error";
		}
		unsigned short int test = ((unsigned short int *)(t_data))[10*488 + 10];
		glbin_jvm_instance->m_pEnv->DeleteLocalRef(arr);
		glbin_jvm_instance->m_pEnv->DeleteLocalRef(val);
	}

	// Creating Nrrd out of the data.
	Nrrd *nrrdout = nrrdNew();
	
	unsigned long long total_size = m_size.get_size_xyz();
	if (!t_data)
		return NULL;
	
	if (m_eight_bit)
		nrrdWrap_va(nrrdout, (uint8_t*)t_data, nrrdTypeUChar, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
	else
		nrrdWrap_va(nrrdout, (uint16_t*)t_data, nrrdTypeUShort, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSpacing, m_spacing.x(), m_spacing.y(), m_spacing.z());
	auto max_size = m_size * m_spacing;
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());

	if (!m_eight_bit) {
		if (get_max) {
			double value;
			unsigned long long totali = m_size.get_size_xyz();
			for (unsigned long long i = 0; i < totali; ++i)
			{
				value = ((unsigned short*)nrrdout->data)[i];
				m_min_value = i == 0 ? value : (value < m_min_value ? value : m_min_value);
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
