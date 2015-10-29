#include "stdafx.h"
#include "MemoryDevice.h"


MemoryDevice::MemoryDevice(int n_elements)
{
	n_blocks = n_elements;

	block = (unsigned char *)malloc(sizeof(unsigned char)*n_elements);
	//teste if it's ok
}

int MemoryDevice::get_number_of_blocks()
{
	return n_blocks;
}

void MemoryDevice::write(int index, unsigned char pattern)
{
	*(block + index) = pattern;
}

unsigned char MemoryDevice::read(int index)
{
	return *(block + index);
}

MemoryDevice::~MemoryDevice()
{
	free(block);
}
