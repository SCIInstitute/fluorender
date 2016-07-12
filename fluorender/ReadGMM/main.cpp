#include <wx/wx.h>
//#include <vld.h>

using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Wrong arguments.\n");
		return 1;
	}

	wxString filename = argv[1];

	if (!GetSettings(filename))
		cout << "Settings not found. Use default settings.\n";

	printf("All done. Quit.\n");

	return 0;
}