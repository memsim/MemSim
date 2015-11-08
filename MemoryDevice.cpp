#include "stdafx.h"
#include "MemoryDevice.h"

using namespace std;

const unsigned char BIT_MASK[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
const int s_size = 39;
char *priv_string = (char *)malloc(sizeof(char) * (s_size + 1)); // to test

unsigned int opCodeOperation(unsigned int opCode)
{
	return opCode >> 29;
}
unsigned int opCodeIndex(unsigned int opCode)
{
	return (536870904 & opCode) >> 3;
}
unsigned int opCodeCell(unsigned int opCode)
{
	return opCode & 7;
}
OpCode opCodeExtract(unsigned int opCode)
{
	OpCode sensitizing;

	sensitizing.op = opCode >> 29;
	sensitizing.index = (536870904 & opCode) >> 3;
	sensitizing.cell = opCode & 7;
	return sensitizing;
}


MemoryDevice::MemoryDevice(int n_elements)
{
	int i;
	n_blocks = n_elements;

	// allocating memory array 
	block = (unsigned char *)malloc(sizeof(unsigned char)*n_elements);
	// allocating pointer to the fault primitives
	fp_list = (FP **)malloc(sizeof(FP *)*n_elements);

	//teste if all it was allocated

	// inicialize memory array with 0s e fr_list with NULLs
	for (i = 0; i < n_blocks; i++){
		*(block + i) = 0;
		*(fp_list + i) = NULL;
	}
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

void MemoryDevice::internal_write_bit(unsigned int index, unsigned int bit, bool value)
{
	unsigned char pattern;

	pattern = *(block + index);
	// performing write operation
	pattern = pattern & (~BIT_MASK[bit]); // clear bit to be set

	if (value){
		pattern = pattern ^ BIT_MASK[bit];
	}

	*(block + index) = pattern;
}

void MemoryDevice::write_bit(unsigned int index, unsigned int bit, bool value)
{
	unsigned char pattern;
	unsigned char bit_state;
	unsigned int op_bits, add_bits, cell_bits, op_code;
	FP fp;
	OpCode victim;
	bool victim_bit_state;

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
	if (*(fp_list + index) != NULL){
		fp = **(fp_list + index);
		if (op_code == fp.Sa){// compare operation code for the performed operation with fault
			if (fp.Sv == NULL){ // simple fault single cell -> type xwy and xrx
				// Applying fault
				value = fp.fault;
			}
			else{ // simple fault two cell -> Sa type xwy and xrx
				// Apply fault if Sv is a state sensitizing
				if (fp.Sv >= 3221225472){ // simple fault two cell -> Sv type x
					// extracting victim cell informations
					victim = opCodeExtract(fp.Sv);
					victim_bit_state = internal_read_bit(victim.index, victim.cell);
					if ((victim.op == 6 && !victim_bit_state) || (victim.op == 7 && victim_bit_state)){
						internal_write_bit(victim.index, victim.cell, fp.fault);
						// re-read pattern in case it has been modified
						pattern = *(block + index);
					}
				}
			}
		}
		else{
			if (fp.Sa >= 3221225472){
				if (fp.Sv == NULL){ // simple fault single cell -> type x 
					// extracting victim cell informations
					victim = opCodeExtract(fp.Sv);
					// Applying fault
					if ((victim.op == 6 && !value) || (victim.op == 7 && value)){
						value = fp.fault;
					}
				}
			}
		}
	}
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

bool MemoryDevice::internal_read_bit(unsigned int index, unsigned int bit)
{
	unsigned char pattern;
	unsigned char bit_state;

	pattern = *(block + index);

	// verify bit state 0 or 1
	bit_state = (pattern & BIT_MASK[bit]) >> bit;

	// performing read operation
	return bit_state ? true : false;
}

bool MemoryDevice::read_bit(unsigned int index, unsigned int bit)
{
	unsigned char pattern;
	unsigned char bit_state;
	unsigned int op_bits, add_bits, cell_bits, op_code;
	FP fp;
	OpCode victim;
	bool victim_state_bit;
	
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

	// looking for fault
	if (*(fp_list + index) != NULL){
		fp = **(fp_list + index);
		if (op_code == fp.Sa){// compare operation code for the performed operation with fault
			if (fp.Sv == NULL){
				// Applying fault
				pattern = pattern & (~BIT_MASK[bit]); // clear bit to be set

				if (fp.fault){
					pattern = pattern ^ BIT_MASK[bit];
				}

				*(block + index) = pattern;

			}
			else{
				// Apply fault if Sv is a state sensitizing
				if (fp.Sv >= 3221225472){
					// extracting victim cell informations
					victim = opCodeExtract(fp.Sv);
					victim_state_bit = internal_read_bit(victim.index, victim.cell);
					if ((victim.op == 6 && !victim_state_bit) || (victim.op == 7 && victim_state_bit))
						internal_write_bit(victim.index, victim.cell, fp.fault);
				}
			}
			// Applying read output
			bit_state = fp.read;
		}
	}
	// save last operation performed
	last_op_code = op_code;

	// performing read operation
	return bit_state ? true : false;
}

void MemoryDevice::apply_simple_fault_single_cell(unsigned int Sa, bool F, bool R)
{
	int index = opCodeIndex(Sa);
	FP *fp = (FP *)malloc(sizeof(FP));

	(*fp).Sa = Sa;
	(*fp).Sv = NULL;
	(*fp).fault = F;
	(*fp).read = R;
	
	*(fp_list + index) = fp;
}

void MemoryDevice::apply_simple_fault_two_cell(unsigned int Sa, unsigned int Sv, bool F, bool R)
{
	int index = opCodeIndex(Sa);
	FP *fp = (FP *)malloc(sizeof(FP));

	(*fp).Sa = Sa;
	(*fp).Sv = Sv;
	(*fp).fault = F;
	(*fp).read = R;

	*(fp_list + index) = fp;
}

MemoryDevice::~MemoryDevice()
{
	free(block);
	free(priv_string); // to test
	//free fault primitives
}
