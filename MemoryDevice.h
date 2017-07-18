#pragma once

struct OpCode
{
	unsigned int op;
	unsigned int index;
	unsigned int cell;
};

struct FP
{
	unsigned int Sa;
	unsigned int Sv;
	bool fault;
	bool read;
};

struct FPList
{
	FP fp;
	FPList *next;
};

// MemoryDevice class definition.
class MemoryDevice
{
public:
	
	// Default Constructor
	MemoryDevice();

	// Constructor, create the memory array of n_elements with 1 byte each
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

	// Insert a Simple Fault Single Cell
	void apply_single_cell_static_fault(unsigned int Sa, bool F, bool R);

	// Insert a Simple Fault Two Cell
	void apply_two_cell_static_fault(unsigned int Sa, unsigned int Sv, bool F, bool R);

	// Destructor, deallocate the memory array
	~MemoryDevice();
private:

	// Pointer to the first element 
	unsigned char *block;

	// Memory device's number of blocks
	int n_blocks;

	// last operation, to be a list of operations
	unsigned int last_op_code;

	// fault primitive list for this device
	FPList **fp_list;

	// read without applying any fault and register operation
	bool MemoryDevice::internal_read_bit(unsigned int index, unsigned int bit);

	// write without applying any fault and register operation
	void MemoryDevice::internal_write_bit(unsigned int index, unsigned int bit, bool value);

	//FP looking_for_fault(bool *bit_ptr);
};