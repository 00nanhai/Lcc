#include <iostream>
#include <string>

#include "my_md5.h"

int main(int argc, char **argv)
{
	if (argc < 2) return -1;
	string file = argv[1];
	string md5 = "";
	calc_md5(file, md5);
	cout << file << "\t" << md5 << endl;
	return 0;
}
