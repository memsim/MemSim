#include "stdafx.h"
#include "MemoryDevice.h"
#include <iostream>
using namespace std;

const unsigned char BIT_MASK[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
const int s_size = 39;
char *priv_string = (char *)malloc(sizeof(char) * (s_size + 1)); // to test

MemoryDevice::MemoryDevice(int n_elements)
{
	n_blocks = n_elements;

	block = (unsigned char *)malloc(sizeof(unsigned char)*n_elements);
	//teste if it was allocated
}

int MemoryDevice::get_number_of_blocks()
{
	return n_blocks;
}

unsigned int MemoryDevice::get_last_op_code()
{
	return last_op_code;
}

char *MemoryDevice::get_last_op_code_string()
{
	char *string = priv_string;
	int i, j;
	unsigned int code = last_op_code;

	for (i = 0, j = 0; i < s_size; i++, j++){
		if (j != 0 && j % 4 == 0)
			*(string + s_size-1 - i++) = ' ';

		*(string + s_size-1 - i) = (1 & code ? '1' : '0');
		code >>= 1;
	}
	*(string + s_size) = '\0';
	return string;
}

void MemoryDevice::write_pattern(unsigned int index, unsigned char pattern)
{
	// looking for fault
	*(block + index) = pattern;
}

void MemoryDevice::write_bit(unsigned int index, unsigned int bit, bool value)
{
	unsigned char pattern;
	unsigned char bit_state;
	unsigned int op_bits, add_bits, cell_bits, op_code;
	
	pattern = *(block + index);

	// verify bit state 0 or 1
	bit_state = (pattern & BIT_MASK[bit]) >> bit;

	// define code for this operation
	
	if (bit_state) // define operation bits for a write operation
		if (value)
			op_bits = 3 << 29;
		else
			op_bits = 2 << 29;
	else
		if (value)
			op_bits = 1 << 29;
		else
			op_bits = 0;
	
	add_bits = index << 3; // define element address bits
	cell_bits = bit; // define cell operation bits

	op_code = op_bits ^ add_bits ^ cell_bits; // merge bits into op_code

	last_op_code = op_code;

	//cout << "op_code = " << op_code << endl;

	// looking for fault
	// compare operation code for the performed operation with some fault
	
	// performing write operation
	pattern = pattern & (~BIT_MASK[bit]); // clear bit to be set

	if (value){
		pattern = pattern ^ BIT_MASK[bit];
	}
	
	*(block + index) = pattern;
}

unsigned char MemoryDevice::read_pattern(unsigned int index)
{
	// looking for fault
	return *(block + index);
}

bool MemoryDevice::read_bit(unsigned int index, unsigned int bit)
{
	unsigned char pattern;
	unsigned char bit_state;
	unsigned int op_bits, add_bits, cell_bits, op_code;

	pattern = *(block + index);

	// verify bit state 0 or 1
	bit_state = (pattern & BIT_MASK[bit]) >> bit;

	// define code for this operation

	if (bit_state) // define operation bits for a read operation
		op_bits = 5 << 29;
	else
		op_bits = 4 << 29;;

	add_bits = index << 3; // define element address bits
	cell_bits = bit; // define cell operation bits

	op_code = op_bits ^ add_bits ^ cell_bits; // merge bits into op_code

	last_op_code = op_code;

	// looking for fault
	// compare operation code for the performed operation with some fault

	// performing read operation
	pattern = pattern & (~BIT_MASK[bit]); // clear bit to be set

	return bit_state ? true : false;
}


MemoryDevice::~MemoryDevice()
{
	free(block);
	free(priv_string); // to test
}
