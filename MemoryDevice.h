#pragma once
// MemoryDevice class definition.
class MemoryDevice
{
public:
	
	// Constructor, create the memory array with n_elements
	MemoryDevice(int n_elements);

	// Return the memory device's number of blocks 
	int get_number_of_blocks();

	// Write a pattern at a specific element
	void write(int index, unsigned char pattern);
	
	// Return a pattern of a specific element
	unsigned char read(int index, unsigned char pattern);
	
	// Destructor, deallocate the memory array
	~MemoryDevice();
private:

	// Pointer to the first element 
	unsigned char *block;

	// Memory device's number of blocks
	int n_blocks;
};