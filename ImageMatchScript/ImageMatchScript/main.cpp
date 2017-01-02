
#include "global.h"
#include "matchparser.h"
#include "matchscript.h"

#pragma comment(lib, "winmm.lib")

using namespace std;
using namespace cv;

void main()
{
	match::cMatchScript matchScript;
	if (!matchScript.Read("test.txt"))
	{
		cout << "Error Read match script" << endl;
		return;
	}

	cout << "Execute ------------------" << endl;
	matchScript.Exec();
}
