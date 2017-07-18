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

MemoryDevice::MemoryDevice(void)
{}

MemoryDevice::MemoryDevice(int n_elements)
{
	int i;
	n_blocks = n_elements;

	// allocating memory array 
	block = (unsigned char *)malloc(sizeof(unsigned char)*n_elements);
	// allocating the fault primitives array
	fp_list = (FPList **)malloc(sizeof(FPList *)*n_elements);

	//teste if it was all allocated


	// inicialize memory array with 0s e fp_list array with NULLs
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
	FPList **nextFpList;
	OpCode aggressor;
	OpCode victim;
	bool aggressor_bit_state;
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
	
	// save last operation performed
	last_op_code = op_code;

	nextFpList = (fp_list + index);
	
	//cout << "op_code = " << op_code << endl;

	// has a fault 
	while (*nextFpList != NULL)
	{
		
		fp = (**nextFpList).fp;

		// static fault, single cell - the victim cell is the same aggressor cell
		if (fp.Sv == NULL)
		{
			// case sensitizing is xwy
			if (op_code == fp.Sa)
			{
				value = fp.fault;
			}
			else
			{
				// case sensitizing is x
				if (fp.Sa >= 3221225472){
					// extracting victim cell informations
					victim = opCodeExtract(fp.Sa);
					// Applying fault
					if ((victim.op == 6 && !value) || (victim.op == 7 && value)){
						value = fp.fault;
					}
				}
			}
		}
		// static fault, two cell - the victim cell is different from aggressor cell
		else
		{
			
			// operation in agressor cell
			if (op_code == fp.Sa)// case <xwy;z>
			{
				//verify victim cell state (z)
				if (fp.Sv >= 3221225472){
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
			// operation in victim cell
			else
			{
				if (op_code == fp.Sv)// case <x;ywz/*/->
				{
					//verify aggressor cell state (x)
					if (fp.Sa >= 3221225472)
					{// extracting victim cell informations
						victim = opCodeExtract(fp.Sv);
						// extracting aggressor cell informations
						aggressor = opCodeExtract(fp.Sa);
						aggressor_bit_state = internal_read_bit(aggressor.index, aggressor.cell);
						if ((aggressor.op == 6 && !aggressor_bit_state) || (aggressor.op == 7 && aggressor_bit_state)){
							
							//cout << "operation in victim cell " << fp.fault << endl;
							//internal_write_bit(victim.index, victim.cell, fp.fault);
							//cout << internal_read_bit(victim.index, victim.cell) << endl;
							// re-read pattern in case it has been modified
							//pattern = *(block + index);
							value = fp.fault;
						}
					}
				}
				/* rethink case <x;y>
				// case <x;y>
				if (fp.Sa >= 3221225472)
				{
					if (fp.Sv >= 3221225472)
					{
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
				*/
			}
		}
		// get next fp on the list
		nextFpList = &((**nextFpList).next);
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
	FPList **nextFpList;
	OpCode aggressor;
	OpCode victim;
	bool aggressor_bit_state;
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

	nextFpList = (fp_list + index);
	
	// has fault
	//cout << *nextFpList << endl;
	while (*nextFpList != NULL)
	{
		//cout << "fpList " << *nextFpList << " Next " << (**nextFpList).next << endl;
		//if (*(fp_list + index) != NULL){
		fp = (**nextFpList).fp;

		// static fault single cell - the victim cell is the same aggressor cell
		if (fp.Sv == NULL)
		{
			// case sensitizing is type xrx
			if (op_code == fp.Sa)
			{
				// Applying fault
				internal_write_bit(index, bit, fp.fault);

				// Applying read output
				bit_state = fp.read;
			}
			// case sensitizing is type x
			// it's only performed in write_bit method
		}
		// static fault two cell - the victim cell is different from aggressor cell
		else
		{
			// operation in aggressor cell
			// case sensitizing is type <xrx;y>
			if (op_code == fp.Sa)
			{
				// Apply fault if Sv is a state sensitizing
				if (fp.Sv >= 3221225472)
				{
					// extracting victim cell informations
					victim = opCodeExtract(fp.Sv);
					victim_state_bit = internal_read_bit(victim.index, victim.cell);

					if ((victim.op == 6 && !victim_state_bit) || (victim.op == 7 && victim_state_bit))
						internal_write_bit(victim.index, victim.cell, fp.fault);
				}
				// fp.read don't apply in this case
			}
			else
			{
				// operation in victim cell
				// case <x;yry/*/->
				if (op_code == fp.Sv)
				{
					//verify aggressor cell state (x)
					if (fp.Sa >= 3221225472)
					{// extracting victim cell informations
						victim = opCodeExtract(fp.Sv);
						// extracting aggressor cell informations
						aggressor = opCodeExtract(fp.Sa);
						aggressor_bit_state = internal_read_bit(aggressor.index, aggressor.cell);
						if ((aggressor.op == 6 && !aggressor_bit_state) || (aggressor.op == 7 && aggressor_bit_state)){
							// Applying fault
							internal_write_bit(index, bit, fp.fault);

							// Applying read output
							bit_state = fp.read;
						}
					}
				}
			}
		}
		// get next fp on the list
		nextFpList = &((**nextFpList).next);
	}


	// save last operation performed
	last_op_code = op_code;

	// performing read operation
	return bit_state ? true : false;
}

void MemoryDevice::apply_single_cell_static_fault(unsigned int Sa, bool F, bool R)
{
	int index = opCodeIndex(Sa);
	FPList *fpList = (FPList *)malloc(sizeof(FPList));
	FPList **nextFpList = (fp_list + index);


	//cout << "apply fault " << fpList << endl;
	
	(*fpList).fp.Sa = Sa;
	(*fpList).fp.Sv = NULL;
	(*fpList).fp.fault = F;
	(*fpList).fp.read = R;
	(*fpList).next = NULL;

	while (*nextFpList != NULL)
	{
		nextFpList = &((**nextFpList).next);
	}

	*(nextFpList) = fpList;
}

void MemoryDevice::apply_two_cell_static_fault(unsigned int Sa, unsigned int Sv, bool F, bool R)
{
	int indexA = opCodeIndex(Sa);
	int indexV = opCodeIndex(Sv);
	// Estorage FP and pointer to next FP on list
	FPList *fpList = NULL;
	FPList **nextFpList = NULL;


	// Record FP
	fpList = (FPList *)malloc(sizeof(FPList));
	
	(*fpList).fp.Sa = Sa;
	(*fpList).fp.Sv = Sv;
	(*fpList).fp.fault = F;
	(*fpList).fp.read = R;
	(*fpList).next = NULL;
 
	//cout << "apply fault " << fpList << " " << (*fpList).next << endl;

	// Record FP on the aggressor cell fault list
	nextFpList = (fp_list + indexA);

	//cout << "apply fault " << fpList << " " << (*fpList).next << endl;
	
	while (*nextFpList != NULL)
	{
		nextFpList = &((**nextFpList).next);

		//cout << "-apply fault " << fpList << " " << (*fpList).next << endl;
	}

	*(nextFpList) = fpList;

	if (indexA != indexV)
	{
		// Record FP on the victim cell fault list
		fpList = (FPList *)malloc(sizeof(FPList));

		(*fpList).fp.Sa = Sa;
		(*fpList).fp.Sv = Sv;
		(*fpList).fp.fault = F;
		(*fpList).fp.read = R;
		(*fpList).next = NULL;

		nextFpList = (fp_list + indexV);

		while (*nextFpList != NULL)
		{
			nextFpList = &((**nextFpList).next);
		}

		*(nextFpList) = fpList;
	}
	//cout << "apply fault " << fpList << " " << (*fpList).next << endl;
}

MemoryDevice::~MemoryDevice()
{
	/*free(block);
	free(priv_string); // to test
	*/
	//free fault primitives
}
