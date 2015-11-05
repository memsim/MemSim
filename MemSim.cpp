// MemSim.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MemoryDevice.h"
#include <iostream>
using namespace std;

unsigned int stringToOpcode(char *string, int index, int cell)
{
	unsigned int op_bits, add_bits, cell_bits, op_code;

	if (*(string + 1) == 'w'){
		if (*(string + 0) == '0')
			if (*(string + 2) == '0')
				op_bits = 0;
			else
				op_bits = 1 << 29;
		else
			if (*(string + 2) == '0')
				op_bits = 2 << 29;
			else
				op_bits = 3 << 29;
	}
	else{
		if (*(string + 1) == 'r'){
			if (*(string + 0) == '0')
				op_bits = 4 << 29;
			else
				op_bits = 5 << 29;
		}
		else{
			if (*string == '0')
				op_bits = 6 << 29;
			else
				op_bits = 7 << 29;
		}
	}
	add_bits = index << 3; // define element address bits
	cell_bits = cell; // define cell operation bits

	op_code = op_bits ^ add_bits ^ cell_bits; // merge bits into op_code

	return op_code;
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	MemoryDevice devTest = MemoryDevice(2*1024);
	int i;
	char bits_read[9];
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

	// test write_bit 'I'

	devTest.write_bit(0, 3, 1);
	cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
	cout << " = " << devTest.get_last_op_code() << endl;
	cout << devTest.read_pattern(0) << " - ";

	// test method read_bit
	for (i = 0; i < 8; i++){
		*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
	}
	*(bits_read + 8) = '\0';
	cout << bits_read << endl;

	// Apply simple fault two cell
	cout << "simple fault two cell" << endl;
	cout << stringToOpcode("0w0", 1, 1) << endl;
	cout << stringToOpcode("1", 1, 0) << endl;
	
	devTest.apply_simple_fault_two_cell(stringToOpcode("0w0", 1, 1), stringToOpcode("1", 1, 0), 0, NULL);

	// test method write_bit ']'
	bool w_bits[8] = { 1, 0, 1, 1, 1, 0, 1, 0 };
	
	for (i = 0; i < 8; i++){
		devTest.write_bit(1, i, w_bits[i]);
		cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
		cout << " = " << devTest.get_last_op_code() << endl;
	}
	cout << devTest.read_pattern(1) << " - ";

	// test method read_bit
	for (i = 0; i < 8; i++){
		*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
	}
	*(bits_read + 8) = '\0';
	cout << bits_read << endl;

	// test method read_bit
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
	cout << devTest.read_pattern(1) << " - ";

	//show bits
	for (i = 0; i < 8; i++){
		*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
	}
	*(bits_read + 8) = '\0';
	cout << bits_read << endl;
	/*
	cout << stringToOpcode("0w0", 0, 0) << endl;
	cout << stringToOpcode("0w1", 0, 0) << endl;
	cout << stringToOpcode("1w0", 0, 0) << endl;
	cout << stringToOpcode("1w1", 0, 0) << endl;
	cout << stringToOpcode("0r0", 0, 0) << endl;
	cout << stringToOpcode("1r1", 0, 0) << endl;
	cout << stringToOpcode("0", 0, 0) << endl;
	cout << stringToOpcode("1", 0, 0) << endl;
	*/

	return 0;
}

