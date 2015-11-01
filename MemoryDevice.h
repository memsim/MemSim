#pragma once
// MemoryDevice class definition.
class MemoryDevice
{
public:
	
	// Constructor, create the memory array with n_elements
	MemoryDevice(int n_elements);

	// Return the memory device's number of blocks 
	int get_number_of_blocks();
	
	// Return the last operations performed codes 
	unsigned int get_last_op_code();

	// Return the last operations performed codes binary string
	char *get_last_op_code_string();

	// Write a pattern at a specific element
	void write_pattern(unsigned int index, unsigned char pattern);

	// Write a specific bit to a specific element
	void write_bit(unsigned int index, unsigned int bit, bool value);
	
	// Return a pattern of a specific element
	unsigned char read_pattern(unsigned int index);

	// Return a specific bit of a specific element
	bool read_bit(unsigned int index, unsigned int bit);
	
	// Destructor, deallocate the memory array
	~MemoryDevice();
private:

	// Pointer to the first element 
	unsigned char *block;

	// Memory device's number of blocks
	int n_blocks;

	// last operation
	unsigned int last_op_code;

	//
};