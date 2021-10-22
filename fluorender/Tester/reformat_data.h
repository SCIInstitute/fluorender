#include <iostream>
#include <string> 
#include <sstream>
#include <fstream> 

void reformat_data()
{
	std::ifstream infile("E:\\PROJECTS\\fluorender_yexp\\Templates\\data.tsv");
	std::ofstream outfile("E:\\PROJECTS\\fluorender_yexp\\Templates\\data.txt");
	std::string str;
	while (std::getline(infile, str))
	{
		size_t pos = str.find("\t", 0);
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