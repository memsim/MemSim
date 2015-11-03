// MemSim.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MemoryDevice.h"
#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	
	MemoryDevice devTest = MemoryDevice(2*1024);
	int i;
	int n = devTest.get_number_of_blocks();

	cout << sizeof(int) << endl;
	cout << n << endl;

	// test write_pattern

	for (i = 0; i < n; i++){
		devTest.write_pattern(i, 'A');
		cout << devTest.read_pattern(i);
	}
	
	// test read_pattern

	for (i = 0; i < n; i++){
		devTest.write_pattern(i, 'A');
		cout << devTest.read_pattern(i);
	}
	cout << endl;

	// test write_bit

	devTest.write_bit(0, 3, 1);
	cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
	cout << " = " << devTest.get_last_op_code() << endl;
	cout << devTest.read_pattern(0) << endl;

	// test method write_bit 
	bool w_bits[8] = { 1, 0, 1, 1, 1, 0, 1, 0 };
	
	for (i = 0; i < 8; i++){
		devTest.write_bit(1, i, w_bits[i]);
		cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
		cout << " = " << devTest.get_last_op_code() << endl;
	}
	cout << devTest.read_pattern(1) << endl;

	// test method read_bit
	char bits_read[9];

	for (i = 0; i < 8; i++){
		*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
		cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
		cout << " = " << devTest.get_last_op_code() << endl;
	}
	*(bits_read + 8) = '\0';
	cout << bits_read << endl;

	// test simple fault single cell
	devTest.apply_simple_fault_single_cell(1073741832, 1, NULL);
	devTest.write_bit(1, 0, 0);
	cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
	cout << " = " << devTest.get_last_op_code() << endl;
	cout << devTest.read_pattern(1) << endl;
	devTest.write_bit(1, 1, 1);
	cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
	cout << " = " << devTest.get_last_op_code() << endl;
	cout << devTest.read_pattern(1) << endl;

	//show bits
	for (i = 0; i < 8; i++){
		*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
	}
	*(bits_read + 8) = '\0';
	cout << bits_read << endl;

	return 0;
}

