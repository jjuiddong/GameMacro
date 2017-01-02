
#include "global.h"
#include "dictionary.h"

using namespace std;


void main()
{
	tess::cDictionary dict;
	if (!dict.Init("dictionary.txt"))
	{
		cout << "Error read dictionary file " << endl;
		return;
	}

	ifstream ifs("testset.txt");
	if (!ifs.is_open())
	{
		cout << "Error read testset file" << endl;
		return;
	}

	string line;
	while (getline(ifs, line))
	{
		trim(line);
		if (line.empty())
			continue;

		vector<string> tmp;
		const string result = dict.Search(line, tmp);
		cout << line << "  --------->  " << result << endl;
	}
}
