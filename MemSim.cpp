// MemSim.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "MemoryDevice.h"
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	MemoryDevice devTest = MemoryDevice(2*1024);
	int i;
	int n = devTest.get_number_of_blocks();

	cout << n << endl;

	for (i = 0; i < n; i++){
		devTest.write(i, 'A');
		cout << devTest.read(i);
	}
	
	cout << endl;

	return 0;
}

