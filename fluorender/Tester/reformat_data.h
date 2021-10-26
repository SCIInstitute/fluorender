#include <iostream>
#include <string> 
#include <sstream>
#include <fstream> 

void reformat_data()
{
	std::ifstream infile("E:\\PROJECTS\\d3_test\\data.tsv");
	std::ofstream outfile("E:\\PROJECTS\\d3_test\\data.txt");
	std::string str;
	size_t pos;
	
	while (std::getline(infile, str))
	{
		int type = 0;
		pos = str.find("activity", 0);
		if (pos != std::string::npos)
			type = 1;
		pos = str.find("Running", 0);
		if (pos != std::string::npos)
		{
			type = 2;
			str.replace(pos, 7, "ID-10");
		}
		pos = str.find("Hiking", 0);
		if (pos != std::string::npos)
		{
			type = 3;
			str.replace(pos, 6, "ID-20");
		}
		pos = str.find("Biking", 0);
		if (pos != std::string::npos)
		{
			type = 3;
			str.replace(pos, 6, "ID-30");
		}


		if (type == 0)
			continue;
		//
		pos = str.find("\t", 0);
		while (pos != std::string::npos)
		{
			str.replace(pos, 1, ",");
			pos = str.find("\t", pos + 1);
		}
		str += "\\n\\";
		outfile << str << std::endl;
	}

	infile.close();
	outfile.close();
}