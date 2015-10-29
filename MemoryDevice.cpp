#include "stdafx.h"
#include "MemoryDevice.h"


MemoryDevice::MemoryDevice(int n_elements)
{
	block = (unsigned char *)malloc(sizeof(unsigned char)*n_elements);
}

int MemoryDevice::get_number_of_blocks()
{
	return n_blocks;
}

void MemoryDevice::write(int index, unsigned char pattern)
{
	*(block + index) = pattern;
}

unsigned char MemoryDevice::read(int index, unsigned char pattern)
{
	return *(block + index);
}

MemoryDevice::~MemoryDevice()
{
	free(block);
}
