// MemSim.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MemoryDevice.h"
#include <iostream>
using namespace std;

char *write_op[4] = { "0w0", "1w1", "1w0", "0w1" };
char *read_op[2] = { "0r0", "1r1" };
char *bit_state[2] = { "0", "1" };
enum FaultType { WDF, TF, RDF, DRDF, IRF, CFds, CFtr, CFwd, CFrd, CFdrd, CFir };
bool ver = false;

// Transform sensitizing fp strings into the code used here
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
/*
 *	return a random addr it there isn't in the list
 */
int random_no_repeat_addr(list <int>* my_list, int n)
{
	int value;
	bool find;

	do
	{
		find = false;
		value = rand() % n;
		for (list<int>::iterator it = (*my_list).begin(); it != (*my_list).end(); it++)
			if (*it == value)
			{
				find = true;
				break;
			}
	} while (find);

	
	return value;
}
/*
 *	Insert a fault of a determined type at a random address.
 */
int randomInsertFaults(bool repeatAddr, MemoryDevice *mem, list <int>* faults, FaultType type, int number)
{
	int i, j, k; // Control Variables
	int addr, addr_aggr, bit1, bit2; // address where goes fault
	
	//int addr_base; // address base to apply near fault
	//list <int> faults; // list with fault addresses
	int n; // number of (*mem)ory addresses

	// Cria memória
	n = (*mem).get_number_of_blocks();

	/* initialize random seed: */
	srand(time(NULL));

	switch (type)
	{
	case WDF:
		/*** Write Destructive Fault (WDF) ***/
		for (i = 0; i < number; i++)
		{
			

			// FP = <'0w0'/1/->

			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);

			(*mem).apply_single_cell_static_fault(stringToOpcode("0w0", addr, rand() % 8), 1, NULL);
			(*faults).push_back(addr);
			/* FP = <'1w1'/0/-> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("1w1", addr, rand() % 8), 0, NULL);
			(*faults).push_back(addr);
		}
		break;
	case TF:
		/*** Transition Fault (TF) ***/
		for (i = 0; i < number; i++)
		{
			/*  FP = <'0w1'/0/-> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("0w1", addr, rand() % 8), 0, NULL);
			(*faults).push_back(addr);
			/* FP = <'1w0'/1/-> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("1w0", addr, rand() % 8), 1, NULL);
			(*faults).push_back(addr);
		}
		break;
	case RDF:
		/*** Read Destructive Fault (RDF) ***/
		for (i = 0; i < number; i++)
		{
			/* FP = <'0r0'/1/1> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("0r0", addr, rand() % 8), 1, 1);
			(*faults).push_back(addr);
			/* FP = <'1r1'/0/0> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("1r1", addr, rand() % 8), 0, 0);
			(*faults).push_back(addr);
		}
		break;
	case DRDF:
		/* Deceptive Read Destructive Fault (DRDF)*/
		for (i = 0; i < number; i++)
		{
			/* FP = <'0r0'/1/0> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("0r0", addr, rand() % 8), 1, 0);
			(*faults).push_back(addr);
			/* FP = <'1r1'/0/1> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("1r1", addr, rand() % 8), 0, 1);
			(*faults).push_back(addr);
		}break;
	case IRF:
		/* Incorrect Read Fault (IRF) */
		for (i = 0; i < number; i++)
		{
			/* FP = <'0r0'/0/1> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("0r0", addr, rand() % 8), 0, 1);
			(*faults).push_back(addr);
			/* FP = <'1r1'/1/0> */
			addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
			(*mem).apply_single_cell_static_fault(stringToOpcode("1r1", addr, rand() % 8), 1, 0);
			(*faults).push_back(addr);
		}
		break;

		// applying two cell static (*faults)
		//cout << "Two cell static (*faults)..." << endl;

	case CFds:
		/*** Disturb coupling fault (CFds) ***/
		for (i = 0; i < number; i++)
		{
			/* <0w0;0/1/-> <1w1;0/1/-> <1w0;0/1/-> <0w1;0/1/->*/

			for (j = 0; j < 4; j++)
			{
				addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
				if (((rand() % 2) && addr < n-1) || addr == 0)
				{
					addr_aggr = addr + (rand() % 2);
				}
				else
				{
					addr_aggr = addr - (rand() % 2);
				}
				do{
					bit1 = rand() % 8;
					bit2 = rand() % 8;
				} while (bit1 == bit2 && addr == addr_aggr);
				if (ver) printf("%4d %4d %4d %d %d\n", (*faults).size(), addr, addr_aggr, bit1, bit2);

				// apply fault
				(*mem).apply_two_cell_static_fault(stringToOpcode(write_op[j], addr_aggr, bit1), stringToOpcode("0", addr, bit2), 1, NULL);
				(*faults).push_back(addr);
			}

			/* <0w0;1/0/-> <1w1;1/0/-> <1w0;1/0/-> <0w1;1/0/->*/

			for (j = 0; j < 4; j++)
			{
				addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
				if (((rand() % 2) && addr < n - 1) || addr == 0)
				{
					addr_aggr = addr + (rand() % 2);
				}
				else
				{
					addr_aggr = addr - (rand() % 2);
				}
				do{
					bit1 = rand() % 8;
					bit2 = rand() % 8;
				} while (bit1 == bit2 && addr == addr_aggr);
				if (ver) printf("%4d %4d %4d %d %d\n", (*faults).size(), addr, addr_aggr, bit1, bit2);

				// apply fault
				(*mem).apply_two_cell_static_fault(stringToOpcode(write_op[j], addr_aggr, bit1), stringToOpcode("1", addr, bit2), 0, NULL);
				(*faults).push_back(addr);
			}

			/* <0r0;0/1/-> <1r1;0/1/-> <0r0;1/0/-> <1r1;1/0/->*/

			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
					if (((rand() % 2) && addr < n - 1) || addr == 0)
						addr_aggr = addr + (rand() % 2);
					else
						addr_aggr = addr - (rand() % 2);
									
					do{
						bit1 = rand() % 8;
						bit2 = rand() % 8;
					} while (bit1 == bit2 && addr == addr_aggr);

					if (ver) printf("%4d %4d %4d %d %d\n", (*faults).size(), addr, addr_aggr, bit1, bit2);
					// apply fault
					(*mem).apply_two_cell_static_fault(stringToOpcode(read_op[j], addr_aggr, bit1), stringToOpcode(bit_state[k], addr, bit2), (k + 1) % 2, NULL);
					(*faults).push_back(addr);
				}
			}
		}
		break;
	case CFtr:
		/*** Transition coupling fault (CFtr) ***/
		for (i = 0; i < number; i++)
		{
			/* <0;1w0/1/-> <1;1w0/1/-> <0;0w1/0/-> <1;0w1/0/->*/

			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
					if (((rand() % 2) && addr < n - 1) || addr == 0)
					{
						addr_aggr = addr + (rand() % 2);
					}
					else
					{
						addr_aggr = addr - (rand() % 2);
					}
					do{
						bit1 = rand() % 8;
						bit2 = rand() % 8;
					} while (bit1 == bit2 && addr == addr_aggr);
					// apply fault
					(*mem).apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr_aggr, bit1), stringToOpcode(write_op[k + 2], addr, bit2), (k + 1) % 2, NULL);
					(*faults).push_back(addr);
				}
			}
		}
		break;
	case CFwd:
		/*** Write Destructive coupling fault (CFwd) ***/
		for (i = 0; i < number; i++)
		{
			/* <0;0w0/1/-> <1;0w0/1/-> <0;1w1/0/-> <1;1w1/0/->*/

			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
					if (((rand() % 2) && addr < n - 1) || addr == 0)
					{
						addr_aggr = addr + (rand() % 2);
					}
					else
					{
						addr_aggr = addr - (rand() % 2);
					}
					do{
						bit1 = rand() % 8;
						bit2 = rand() % 8;
					} while (bit1 == bit2 && addr == addr_aggr);
					// apply fault
					(*mem).apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr_aggr, bit1), stringToOpcode(write_op[k], addr, bit2), (k + 1) % 2, NULL);
					(*faults).push_back(addr);
				}
			}
		}
		break;
	case CFrd:
		/*** Read Destructive coupling fault (CFrd) ***/
		for (i = 0; i < number; i++)
		{
			/* <0;0r0/1/1> <1;0r0/1/1> <0;1r1/0/0> <1;1r1/0/0>*/

			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
					if ((rand() % 2 && addr < n - 1) || addr == 0)
					{
						addr_aggr = addr + (rand() % 2);
					}
					else
					{
						addr_aggr = addr - (rand() % 2);
					}
					do{
						bit1 = rand() % 8;
						bit2 = rand() % 8;
					} while (bit1 == bit2 && addr == addr_aggr);
					// apply fault
					(*mem).apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr_aggr, bit1), stringToOpcode(read_op[k], addr, bit2), (k + 1) % 2, (k + 1) % 2);
					(*faults).push_back(addr);
				}
			}
		}
		break;
	case CFdrd:
		/*** Deceptive Read Destructive coupling fault (CFdrd) ***/
		for (i = 0; i < number; i++)
		{
			/* <0;0r0/1/0> <1;0r0/1/0> <0;1r1/0/1> <1;1r1/0/1> */

			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
					if ((rand() % 2 && addr < n - 1) || addr == 0)
					{
						addr_aggr = addr + (rand() % 2);
					}
					else
					{
						addr_aggr = addr - (rand() % 2);
					}
					do{
						bit1 = rand() % 8;
						bit2 = rand() % 8;
					} while (bit1 == bit2 && addr == addr_aggr);
					// apply fault
					(*mem).apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr_aggr, bit1), stringToOpcode(read_op[k], addr, bit2), (k + 1) % 2, k);
					(*faults).push_back(addr);
				}
			}
		}
		break;
	case CFir:
		/*** Incorrect Read coupling fault (CFir) ***/
		for (i = 0; i < number; i++)
		{
			/* <0;0r0/0/1> <1;0r0/0/1> <0;1r1/1/0> <1;1r1/1/0> */

			for (k = 0; k < 2; k++)
			{
				for (j = 0; j < 2; j++)
				{
					addr = repeatAddr ? rand() % n : random_no_repeat_addr(faults, n);
					if (((rand() % 2) && addr < n - 1) || addr == 0)
					{
						addr_aggr = addr + (rand() % 2);
					}
					else
					{
						addr_aggr = addr - (rand() % 2);
					}
					do{
						bit1 = rand() % 8;
						bit2 = rand() % 8;
					} while (bit1 == bit2 && addr == addr_aggr);
					// apply fault
					(*mem).apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr_aggr, bit1), stringToOpcode(read_op[k], addr, bit2), k, (k + 1) % 2);
					(*faults).push_back(addr);
					if (ver) printf("%4d %4d %4d %d %d <%s;%s/%d/%d>\n", (*faults).size(), addr, addr_aggr, bit1, bit2, bit_state[j], read_op[k], k, (k + 1) % 2);
				}
			}
		}
		break;
	default:
		return 1;
		break;
	}

	// sort fault list
	(*faults).sort();
	// remove duplicates addresses 
	(*faults).unique();

	return 0;
}

int randomInsertRandomFaults(bool repeatAddr, MemoryDevice *mem, list <int>* faults, int max)
{
	/* initialize random seed: */
	srand(time(NULL));

	// FaultTypes : WDF, TF, RDF, DRDF, IRF, CFds, CFtr, CFwd, CFrd, CFdr, CFir
	randomInsertFaults(repeatAddr, mem, faults, WDF, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, TF, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, RDF, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, DRDF, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, IRF, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, CFds, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, CFtr, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, CFwd, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, CFrd, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, CFdrd, rand() % max + 1);
	randomInsertFaults(repeatAddr, mem, faults, CFir, rand() % max + 1);

	/*
	// sort fault list
	(*faults).sort();
	// remove duplicates addresses 
	(*faults).unique();
	*/
	return 0;
}

int runMarchCTest(MemoryDevice *mem, SYSTEMTIME *time, list <int>* failed)
{
	int i; // Control Variables
	int addr; // (*mem)ory address
	SYSTEMTIME start, end, duration; // Report variables
	//list <int> (*failed); // list with found fault addresses
	
	int n = (*mem).get_number_of_blocks();

	

	// clear list detected fails
	if (!(*failed).empty())
		(*failed).clear();

	// get start test time
	GetSystemTime(&start);

	/// *** MARCH C TEST CODE ***
	
	// M0: w0
	for (addr = 0; addr < n; addr++)
	{

		//cout << addr << " ";
		for (i = 0; i < 8; i++)
		{
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M1 : ^ r0 w1
	for (addr = 0; addr < n; addr++)
	{
		//cout << addr << " ";
		for (i = 0; i < 8; i++)
		{
			if ((*mem).read_bit(addr, i))
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1);
		}
	}
	// M2 : ^ r1 w0
	for (addr = 0; addr < n; addr++)
	{
		for (i = 0; i < 8; i++)
		{
			if (!(*mem).read_bit(addr, i))
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M3 : ~ r0 w1
	for (addr = (n - 1); addr >= 0; addr--)
	{
		//cout << addr << " ";
		for (i = 7; i >= 0; i--)
		{
			if ((*mem).read_bit(addr, i))
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1);
		}
	}
	// M4 : ~ r1 w0
	for (addr = n - 1; addr >= 0; addr--){
		for (i = 7; i >= 0; i--){
			if (!(*mem).read_bit(addr, i)){
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M5 : r0
	for (addr = 0; addr < n; addr++){
		for (i = 0; i < 8; i++){
			if ((*mem).read_bit(addr, i)){
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
		}
	}

	/// ***         END       ***
	/// *** MARCH C TEST CODE ***/

	// get end test time
	GetSystemTime(&end);

	// get test duration
	duration.wHour = end.wHour - start.wHour;
	duration.wMinute = end.wMinute - start.wMinute;
	duration.wSecond = end.wSecond - start.wSecond;
	duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

	*time = duration;

	//cout << end << endl;
	
	// reduce list
	(*failed).sort();
	(*failed).unique();
	


	return 0;
}

int runMarchBTest(MemoryDevice *mem, SYSTEMTIME *time, list <int>* failed)
{
	int i; // Control Variables
	int addr; // (*mem)ory address
	SYSTEMTIME start, end, duration; // Report variables
	//list <int> (*(*failed)); // list with found fault addresses

	int n = (*mem).get_number_of_blocks();

	

	// clear list detected fails
	if (!(*failed).empty())
		(*failed).clear();

	// get start test time
	GetSystemTime(&start);

	/// *** MARCH B TEST CODE ***
	
	// M0: w0
	for (addr = 0; addr < n; addr++)
	{
		for (i = 0; i < 8; i++)
		{
			if (ver) cout << "m0" << addr << i << " ";
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M1 : ^ r0 w1 r1 w0 r0 w1
	for (addr = 0; addr < n; addr++)
	{
		if (ver) cout << "m1" << addr << " ";
		for (i = 0; i < 8; i++)
		{
			if ((*mem).read_bit(addr, i))
			{
				if (ver) cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1);
			if (!(*mem).read_bit(addr, i))
			{
				if (ver) cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0);
			if ((*mem).read_bit(addr, i))
			{
				if (ver) cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1);
		}
	}
	// M2 : ^ r1 w0 w1
	for (addr = 0; addr < n; addr++)
	{
		if (ver) cout << "m2" << addr << " ";
		for (i = 0; i < 8; i++)
		{
			if (!(*mem).read_bit(addr, i))
			{
				if (ver) cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0);
			(*mem).write_bit(addr, i, 1);
		}
	}
	// M3 : ~ r1 w0 w1 w0
	for (addr = (n - 1); addr >= 0; addr--)
	{
		if (ver) cout << "m3" << addr << " ";
		for (i = 7; i >= 0; i--)
		{
			if (!(*mem).read_bit(addr, i))
			{
				if (ver) cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0);
			(*mem).write_bit(addr, i, 1);
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M4 : ~ r0 w1 w0
	for (addr = n - 1; addr >= 0; addr--){
		if (ver) cout << "m4" << addr << " ";
		for (i = 7; i >= 0; i--){
			if ((*mem).read_bit(addr, i)){
				if (ver) cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1);
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M5 : r0
	for (addr = 0; addr < n; addr++){
		if (ver) cout << "m5" << addr << " ";
		for (i = 0; i < 8; i++){
			if ((*mem).read_bit(addr, i)){
				if (ver) cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
		}
	}

	/// ***         END       ***
	/// *** MARCH B TEST CODE ***/


	// get end test time
	GetSystemTime(&end);

	// get test duration
	duration.wHour = end.wHour - start.wHour;
	duration.wMinute = end.wMinute - start.wMinute;
	duration.wSecond = end.wSecond - start.wSecond;
	duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

	*time = duration;

	//cout << end << endl;

	// reduce list
	(*failed).sort();
	(*failed).unique();



	return 0;
}

int runMarchSSTest(MemoryDevice *mem, SYSTEMTIME *time, list <int>* failed)
{
	int i; // Control Variables
	int addr; // (*mem)ory address
	SYSTEMTIME start, end, duration; // Report variables
	//list <int> (*(*failed)); // list with found fault addresses

	int n = (*mem).get_number_of_blocks();



	// clear list detected fails
	if (!(*failed).empty())
		(*failed).clear();

	// get start test time
	GetSystemTime(&start);

	/// *** MARCH SS TEST CODE ***

	// M0: w0
	for (addr = 0; addr < n; addr++)
	{

		//cout << addr << " ";
		for (i = 0; i < 8; i++)
		{
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M1 : ^ r0 r0 w0 r0 w1
	for (addr = 0; addr < n; addr++)
	{
		//cout << addr << " ";
		for (i = 0; i < 8; i++)
		{
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0); // w0
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1); // w1
		}
	}
	// M2 : ^ r1 r1 w1 r1 w0
	for (addr = 0; addr < n; addr++)
	{
		for (i = 0; i < 8; i++)
		{
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1); // w1
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0); // w0
		}
	}
	// M3 : ~ r0 r0 w0 r0 w1
	for (addr = (n - 1); addr >= 0; addr--)
	{
		//cout << addr << " ";
		for (i = 7; i >= 0; i--)
		{
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0); // w0
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1); // w1
		}
	}
	// M4 : ~ r1 r1 w1 r1 w0
	for (addr = n - 1; addr >= 0; addr--){
		for (i = 7; i >= 0; i--){
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1); // w1
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0); // w0
		}
	}
	// M5 : r0
	for (addr = 0; addr < n; addr++){
		for (i = 0; i < 8; i++){
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
		}
	}

	/// ***         END       ***
	/// *** MARCH SS TEST CODE ***/


	// get end test time
	GetSystemTime(&end);

	// get test duration
	duration.wHour = end.wHour - start.wHour;
	duration.wMinute = end.wMinute - start.wMinute;
	duration.wSecond = end.wSecond - start.wSecond;
	duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

	*time = duration;

	//cout << end << endl;

	// reduce list
	(*failed).sort();
	(*failed).unique();



	return 0;
}

int runPMOVITest(MemoryDevice *mem, SYSTEMTIME *time, list <int>* failed)
{
	int i; // Control Variables
	int addr; // (*mem)ory address
	SYSTEMTIME start, end, duration; // Report variables
	//list <int> (*(*failed)); // list with found fault addresses

	int n = (*mem).get_number_of_blocks();



	// clear list detected fails
	if (!(*failed).empty())
		(*failed).clear();

	// get start test time
	GetSystemTime(&start);

	/// *** PMOVI TEST CODE ***

	// M0: ~ w0
	for (addr = (n - 1); addr >= 0; addr--)
	{

		//cout << addr << " ";
		for (i = 7; i >= 0; i--)
		{
			(*mem).write_bit(addr, i, 0);
		}
	}
	// M1 : ^ r0 w1 r1
	for (addr = 0; addr < n; addr++)
	{
		//cout << addr << " ";
		for (i = 0; i < 8; i++)
		{
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1); // w1
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
		}
	}
	// M2 : ^ r1 w0 r0
	for (addr = 0; addr < n; addr++)
	{
		for (i = 0; i < 8; i++)
		{
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0); // w0
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
		}
	}
	// M3 : ~ r0 w1 r1
	for (addr = (n - 1); addr >= 0; addr--)
	{
		//cout << addr << " ";
		for (i = 7; i >= 0; i--)
		{
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 1); // w1
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
		}
	}
	// M4 : ~ r1 w0 r0
	for (addr = n - 1; addr >= 0; addr--){
		for (i = 7; i >= 0; i--){
			if (!(*mem).read_bit(addr, i)) // r1
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
			(*mem).write_bit(addr, i, 0); // w0
			if ((*mem).read_bit(addr, i)) // r0
			{
				//cout << "erro : bit " << i << " address " << addr << endl;
				(*failed).push_back(addr);
			}
		}
	}


	/// ***         END       ***
	/// *** PMOVI TEST CODE ***/


	// get end test time
	GetSystemTime(&end);

	// get test duration
	duration.wHour = end.wHour - start.wHour;
	duration.wMinute = end.wMinute - start.wMinute;
	duration.wSecond = end.wSecond - start.wSecond;
	duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

	*time = duration;

	//cout << end << endl;

	// reduce list
	(*failed).sort();
	(*failed).unique();

	return 0;
}

int variousTests(int teste)
{
	// Cria Memória de 2kB
	MemoryDevice devTest = MemoryDevice(2 * 1024);
	int i, j, k;
	char bits_read[9];
	bool w_bits[8] = { 1, 0, 1, 1, 1, 0, 1, 0 }; // ']'
	int n = devTest.get_number_of_blocks();
	// to test
	int addr = 0; //address to test memory fault
	int addr_base = 0; //address base for test groups
	int v1;
	// Report variables
	SYSTEMTIME start, end, duration;
	// create list for testing diag
	list < int > faults;
	list < int > failed;
	list<int>::iterator it2;
	float n_fault;
	float n_fail;

	/* initialize random seed: */
	srand(time(NULL));

	cout << sizeof(unsigned char) << endl;
	cout << n << " bytes" << endl << endl;

	switch (teste){
	case 1:
		// test write_pattern
		cout << "Testing method write_pattern()\n";

		for (i = 0; i < n; i++){
			devTest.write_pattern(i, 'A');
			//cout << devTest.read_pattern(i);
		}

		// test read_pattern
		cout << "Testing method read_pattern()\n";

		for (i = 0; i < n; i++){
			//devTest.write_pattern(i, 'A');
			cout << devTest.read_pattern(i);
		}
		cout << endl;

		// test write_bit 'I'
		cout << "Testing method write_bit()\n";

		devTest.write_bit(0, 3, 1);
		cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
		cout << " = " << devTest.get_last_op_code() << endl;
		cout << devTest.read_pattern(0) << endl;

		// test method read_bit
		cout << "Testing method read_bit()\n";

		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// Apply two cell static fault
		cout << "two cell static fault" << endl;
		cout << stringToOpcode("0w0", 1, 1) << endl;
		cout << stringToOpcode("1", 1, 0) << endl;

		devTest.apply_two_cell_static_fault(stringToOpcode("0w0", 1, 1), stringToOpcode("1", 1, 0), 0, NULL);

		// test method write_bit ']'

		for (i = 0; i < 8; i++){
			devTest.write_bit(1, i, w_bits[i]);
			cout << "op_code = " << devTest.get_last_op_code_string();// << endl;
			cout << " = " << devTest.get_last_op_code() << endl;
		}
		cout << devTest.read_pattern(1) << endl;

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

		// test single cell static fault
		cout << "test single cell satic fault" << endl;
		devTest.apply_single_cell_static_fault(1073741832, 1, NULL);
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

		/* Exemplos de OpCodes /
		cout << "Exemplos de OpCodes:\n";
		cout << "0w0 :" << stringToOpcode("0w0", 0, 0) << endl;
		cout << "0w1 :" << stringToOpcode("0w1", 0, 0) << endl;
		cout << "1w0 :" << stringToOpcode("1w0", 0, 0) << endl;
		cout << "1w1 :" << stringToOpcode("1w1", 0, 0) << endl;
		cout << "0r0 :" << stringToOpcode("0r0", 0, 0) << endl;
		cout << "1r1 :" << stringToOpcode("1r1", 0, 0) << endl;
		cout << "0   :" << stringToOpcode("0", 0, 0) << endl;
		cout << "1   :" << stringToOpcode("1", 0, 0) << endl;
		/**/
		break;
	case 2:
		/*************************************/
		/* Testing single cell static faults */
		/*************************************/
		cout << " ***  Testing single cell static faults  *** " << endl;

		/* Write Destructive Fault (WDF)*/
		cout << "** Write Destructive Fault (WDF) ** " << endl;
		/* S "0w0" */
		cout << "* FP = <'0w0'/1/->" << endl;

		//show bits for address 0
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(0, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("0w0", 0, 0), 1, NULL);

		// sensitizing fault
		devTest.write_bit(0, 0, 0);

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(0, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* FP = <'1w1'/0/-> */
		cout << "* FP = <'1w1'/0/->" << endl;

		//show bits for address 3
		for (i = 0; i < 8; i++){
			devTest.write_bit(3, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(3, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("1w1", 3, 0), 0, NULL);

		// sensitizing fault
		devTest.write_bit(3, 0, 1);

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(3, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* Transition Fault (TF) */
		cout << "** Transition Fault (TF) **" << endl;
		/* S "0w1" */
		cout << "* FP = <'0w1'/0/->" << endl;

		//show bits for address 1
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("0w1", 1, 0), 0, NULL);

		// sensitizing fault
		devTest.write_bit(1, 0, 1);

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(1, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* S "1w0" */
		cout << "FP = <'1w0'/1/->" << endl;

		//show bits for address 2
		for (i = 0; i < 8; i++){
			devTest.write_bit(2, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(2, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("1w0", 2, 0), 1, NULL);

		// sensitizing fault
		devTest.write_bit(2, 0, 0);

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(2, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* Read Destructive Fault (RDF) */
		cout << "** Read Destructive Fault (RDF) **" << endl;
		/* FP = <'0r0'/1/1> */
		cout << "* FP = <'0r0'/1/1>" << endl;

		//show bits for address 4
		for (i = 0; i < 8; i++){
			//devTest.write_bit(3, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(4, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("0r0", 4, 0), 1, 1);

		// sensitizing fault
		cout << devTest.read_bit(4, 0) << endl;

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(4, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* FP = <'1r1'/0/0> */
		cout << "* FP = <'1r1'/0/0>" << endl;

		//show bits for address 5
		for (i = 0; i < 8; i++){
			devTest.write_bit(5, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(5, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("1r1", 5, 0), 0, 0);

		// sensitizing fault
		cout << devTest.read_bit(5, 0) << endl;

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(5, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* Deceptive Read Destructive Fault (DRDF)*/
		cout << "** Deceptive Read Destructive Fault (DRDF) **" << endl;
		/* FP = <'0r0'/1/0> */
		cout << "* FP = <'0r0'/1/0>" << endl;

		//show bits for address 6
		for (i = 0; i < 8; i++){
			//devTest.write_bit(3, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(6, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("0r0", 6, 0), 1, 0);

		// sensitizing fault
		cout << devTest.read_bit(6, 0) << endl;

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(6, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* FP = <'1r1'/0/1> */
		cout << "* FP = <'1r1'/0/1>" << endl;

		//show bits for address 7
		for (i = 0; i < 8; i++){
			devTest.write_bit(7, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(7, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("1r1", 7, 0), 0, 1);

		// sensitizing fault
		cout << devTest.read_bit(7, 0) << endl;

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(7, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* Incorrect Read Fault (IRF) */
		cout << "** Incorrect Read Fault (IRF) **" << endl;
		/* FP = <'0r0'/0/1> */
		cout << "* FP = <'0r0'/0/1>" << endl;

		//show bits for address 8
		for (i = 0; i < 8; i++){
			//devTest.write_bit(3, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(8, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("0r0", 8, 0), 0, 1);

		// sensitizing fault
		cout << devTest.read_bit(8, 0) << endl;

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(8, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		/* FP = <'1r1'/1/0> */
		cout << "* FP = <'1r1'/1/0>" << endl;

		//show bits for address 9
		for (i = 0; i < 8; i++){
			devTest.write_bit(9, i, 1);
			*(bits_read + 7 - i) = devTest.read_bit(9, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_single_cell_static_fault(stringToOpcode("1r1", 9, 0), 1, 0);

		// sensitizing fault
		cout << devTest.read_bit(9, 0) << endl;

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(9, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		break;
	case 3:
		/**********************************/
		/* Testing two cell static faults */
		/**********************************/
		cout << " ***  Testing two cell static faults  *** " << endl;

		/*** Disturb coupling fault (CFds) ***/
		/* <0w0;0/1/-> <1w1;0/1/-> <1w0;0/1/-> <0w1;0/1/->*/
		// address base for test is 0
		addr_base = 0;

		devTest.write_pattern(addr_base + 1, 'A');
		devTest.write_pattern(addr_base + 2, 'A');

		cout << "** Disturb coupling fault (CFds) ** " << endl;
		for (j = 0; j < 4; j++)
		{
			addr = addr_base + j;

			cout << "* FP = <" << write_op[j] << ";0/1/->" << endl;

			//show bits
			for (i = 0; i < 8; i++){
				*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
			}
			*(bits_read + 8) = '\0';
			cout << bits_read << endl;

			// apply fault
			devTest.apply_two_cell_static_fault(stringToOpcode(write_op[j], addr, 0), stringToOpcode("0", addr, 1), 1, NULL);

			// sensitizing fault
			devTest.write_bit(addr, 0, j % 2);

			// verify fault
			for (i = 0; i < 8; i++){
				*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
			}
			*(bits_read + 8) = '\0';
			cout << bits_read << endl;

		}

		/* <0w0;1/0/-> <1w1;1/0/-> <1w0;1/0/-> <0w1;1/0/->*/
		// address base for test is 4
		addr_base = 4;

		devTest.write_pattern(addr_base + 0, 'B');
		devTest.write_pattern(addr_base + 1, 'C');
		devTest.write_pattern(addr_base + 2, 'C');
		devTest.write_pattern(addr_base + 3, 'B');

		for (j = 0; j < 4; j++)
		{
			addr = addr_base + j;
			cout << "* FP = <" << write_op[j] << ";1/0/->" << endl;

			//show bits
			for (i = 0; i < 8; i++){
				*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
			}
			*(bits_read + 8) = '\0';
			cout << bits_read << endl;

			// apply fault
			devTest.apply_two_cell_static_fault(stringToOpcode(write_op[j], addr, 0), stringToOpcode("1", addr, 1), 0, NULL);

			// sensitizing fault
			devTest.write_bit(addr, 0, j % 2);

			// verify fault
			for (i = 0; i < 8; i++){
				*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
			}
			*(bits_read + 8) = '\0';
			cout << bits_read << endl;

		}
		/* <0r0;0/1/-> <1r1;0/1/-> <0r0;1/0/-> <1r1;1/0/->*/
		// address base for test is 8
		addr_base = 8;

		//devTest.write_pattern(addr_base + 0, 'B');
		devTest.write_pattern(addr_base + 1, 'A');
		devTest.write_pattern(addr_base + 2, 'B');
		devTest.write_pattern(addr_base + 3, 'C');

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				cout << "* FP = <" << read_op[j] << ";" << k << "/" << (k + 1) % 2 << "/->" << endl;

				//show bits
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;

				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(read_op[j], addr, 0), stringToOpcode(bit_state[k], addr, 1), (k + 1) % 2, NULL);

				// sensitizing fault
				cout << devTest.read_bit(addr, 0) << endl;

				// verify fault
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;

			}
		}
		/*** Transition coupling fault (CFtr) ***/

		cout << "** Transition coupling fault (CFtr) ** " << endl;
		/* <0;1w0/1/-> <1;1w0/1/-> <0;0w1/0/-> <1;0w1/0/->*/
		addr_base = 12;

		devTest.write_pattern(addr_base + 0, 'A');
		devTest.write_pattern(addr_base + 1, 'C');
		//devTest.write_pattern(addr_base + 2, 'B');
		devTest.write_pattern(addr_base + 3, 'B');

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				cout << "* FP = <" << j << ";" << write_op[k + 2] << "/" << (k + 1) % 2 << "/->" << endl;

				//show bits for addr
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;

				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(write_op[k + 2], addr, 0), (k + 1) % 2, NULL);

				// sensitizing fault
				devTest.write_bit(addr, 0, k);

				// verify fault
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;
			}
		}

		/*** Write Destructive coupling fault (CFwd) ***/

		cout << "** Write Destructive coupling fault (CFwd) ** " << endl;
		/* <0;0w0/1/-> <1;0w0/1/-> <0;1w1/0/-> <1;1w1/0/->*/
		addr_base = 16;

		//devTest.write_pattern(addr_base + 0, 'A');
		devTest.write_pattern(addr_base + 1, 'B');
		devTest.write_pattern(addr_base + 2, 'A');
		devTest.write_pattern(addr_base + 3, 'C');

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				cout << "* FP = <" << j << ";" << write_op[k] << "/" << (k + 1) % 2 << "/->" << endl;

				//show bits for addr
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;

				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(write_op[k], addr, 0), (k + 1) % 2, NULL);

				// sensitizing fault
				devTest.write_bit(addr, 0, k);

				// verify fault
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;
			}
		}

		/*** Read Destructive coupling fault (CFrd) ***/

		cout << "** Read Destructive coupling fault (CFrd) ** " << endl;
		/* <0;0r0/1/1> <1;0r0/1/1> <0;1r1/0/0> <1;1r1/0/0>*/
		addr_base = 20;

		//devTest.write_pattern(addr_base + 0, 'A');
		devTest.write_pattern(addr_base + 1, 'B');
		devTest.write_pattern(addr_base + 2, 'A');
		devTest.write_pattern(addr_base + 3, 'C');

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				cout << "* FP = <" << j << ";" << read_op[k] << "/" << (k + 1) % 2 << "/" << (k + 1) % 2 << ">" << endl;

				//show bits for addr
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;

				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(read_op[k], addr, 0), (k + 1) % 2, (k + 1) % 2);

				// sensitizing fault
				cout << devTest.read_bit(addr, 0) << endl;

				// verify fault
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;
			}
		}

		/*** Deceptive Read Destructive coupling fault (CFdr) ***/

		cout << "** Deceptive Read Destructive coupling fault (CFdr) ** " << endl;
		/* <0;0r0/1/0> <1;0r0/1/0> <0;1r1/0/1> <1;1r1/0/1> */
		addr_base = 24;

		//devTest.write_pattern(addr_base + 0, 'A');
		devTest.write_pattern(addr_base + 1, 'B');
		devTest.write_pattern(addr_base + 2, 'A');
		devTest.write_pattern(addr_base + 3, 'C');

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				cout << "* FP = <" << j << ";" << read_op[k] << "/" << (k + 1) % 2 << "/" << k << ">" << endl;

				//show bits for addr
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;

				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(read_op[k], addr, 0), (k + 1) % 2, k);

				// sensitizing fault
				cout << devTest.read_bit(addr, 0) << endl;

				// verify fault
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;
			}
		}

		/*** Incorrect Read coupling fault (CFir) ***/

		cout << "** Incorrect Read coupling fault (CFir) ** " << endl;
		/* <0;0r0/0/1> <1;0r0/0/1> <0;1r1/1/0> <1;1r1/1/0> */
		addr_base = 28;

		devTest.write_pattern(addr_base + 0, '@');
		devTest.write_pattern(addr_base + 1, 'B');
		devTest.write_pattern(addr_base + 2, 'A');
		devTest.write_pattern(addr_base + 3, 'C');

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				cout << "* FP = <" << j << ";" << read_op[k] << "/" << k << "/" << (k + 1) % 2 << ">" << endl;

				//show bits for addr
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << endl;

				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(read_op[k], addr, 0), k, (k + 1) % 2);

				// sensitizing fault
				cout << devTest.read_bit(addr, 0) << endl;

				// verify fault
				for (i = 0; i < 8; i++){
					*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
				}
				*(bits_read + 8) = '\0';
				cout << bits_read << " != " << devTest.read_pattern(addr) << endl;
			}
		}


		break;

	case 4:
		/*********************************************/
		/* Testing apply two faults on the same cell */
		/*********************************************/
		cout << " ***  Testing apply two faults on the same address  *** " << endl;

		// "** Two read operations two-cell faults ** "
		// <1r1;1/1/-> <0;1r1/0/->
		cout << "** Two read operations two-cell faults ** " << endl;
		// address base for test is 0
		addr = 0;

		devTest.write_pattern(addr, 'G'); // 01000111

		//show bits for address 0
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_two_cell_static_fault(stringToOpcode("1r1", addr, 0), stringToOpcode("1", addr, 1), 0, NULL);
		devTest.apply_two_cell_static_fault(stringToOpcode("0", addr, 3), stringToOpcode("1r1", addr, 2), 0, 1);

		// sensitizing fault
		devTest.read_bit(addr, 0);
		devTest.read_bit(addr, 2);

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;


		// ** Two write operations two-cell faults ** 
		// <1w0;1/1/-> <0;1w0/0/->
		cout << "** Two write operations two-cell faults ** " << endl;
		// address base for test is 0
		addr = 1;

		devTest.write_pattern(addr, 'G'); // 01000111

		//show bits for address 0
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		// apply fault
		devTest.apply_two_cell_static_fault(stringToOpcode("1w0", addr, 0), stringToOpcode("1", addr, 1), 0, NULL);
		devTest.apply_two_cell_static_fault(stringToOpcode("0", addr, 3), stringToOpcode("1w0", addr, 2), 1, NULL);
		devTest.apply_single_cell_static_fault(stringToOpcode("0r0", addr, 7), 1, 1);
		devTest.apply_single_cell_static_fault(stringToOpcode("1w0", addr, 6), 1, NULL);

		// sensitizing fault
		devTest.write_bit(addr, 0, 0);
		devTest.write_bit(addr, 2, 0);
		devTest.write_bit(addr, 6, 0);
		devTest.read_bit(addr, 7);

		// verify fault
		for (i = 0; i < 8; i++){
			*(bits_read + 7 - i) = devTest.read_bit(addr, i) ? '1' : '0';
		}
		*(bits_read + 8) = '\0';
		cout << bits_read << endl;

		break;

	case 5:
		/*********************************************/
		/*         Testing Fault Detection           */
		/*********************************************/
		cout << " >>> Testing Fault Detection <<<" << endl << endl;
		cout << "Injecting Faults... " << endl;

		// try rand number
		v1 = rand() % n;         // v1 in the range 0 to n
		cout << v1 << endl;

		// applying single cell static faults
		cout << "Single Cell Static Faults..." << endl;

		/*** Write Destructive Fault (WDF) ***/

		// FP = <'0w0'/1/->
		addr = 1;
		devTest.apply_single_cell_static_fault(stringToOpcode("0w0", addr, 2), 1, NULL);
		faults.push_back(addr);
		/* FP = <'1w1'/0/-> */
		addr = 3;
		devTest.apply_single_cell_static_fault(stringToOpcode("1w1", addr, 1), 0, NULL);
		faults.push_back(addr);

		/*** Transition Fault (TF) ***/

		/*  FP = <'0w1'/0/-> */
		addr = 15;
		devTest.apply_single_cell_static_fault(stringToOpcode("0w1", addr, 4), 0, NULL);
		faults.push_back(addr);
		/* FP = <'1w0'/1/-> */
		addr = 19;
		devTest.apply_single_cell_static_fault(stringToOpcode("1w0", addr, 0), 1, NULL);
		faults.push_back(addr);

		/*** Read Destructive Fault (RDF) ***/

		/* FP = <'0r0'/1/1> */
		addr = 24;
		devTest.apply_single_cell_static_fault(stringToOpcode("0r0", addr, 6), 1, 1);
		faults.push_back(addr);
		/* FP = <'1r1'/0/0> */
		addr = 25;
		devTest.apply_single_cell_static_fault(stringToOpcode("1r1", addr, 2), 0, 0);
		faults.push_back(addr);

		/* Deceptive Read Destructive Fault (DRDF)*/

		/* FP = <'0r0'/1/0> */
		addr = 36;
		devTest.apply_single_cell_static_fault(stringToOpcode("0r0", addr, 7), 1, 0);
		faults.push_back(addr);
		/* FP = <'1r1'/0/1> */
		addr = 39;
		devTest.apply_single_cell_static_fault(stringToOpcode("1r1", addr, 6), 0, 1);
		faults.push_back(addr);

		/* Incorrect Read Fault (IRF) */

		/* FP = <'0r0'/0/1> */
		addr = 43;
		devTest.apply_single_cell_static_fault(stringToOpcode("0r0", addr, 3), 0, 1);
		faults.push_back(addr);
		/* FP = <'1r1'/1/0> */
		addr = 49;
		devTest.apply_single_cell_static_fault(stringToOpcode("1r1", addr, 0), 1, 0);
		faults.push_back(addr);

		// applying two cell static faults
		cout << "Two cell static faults..." << endl;

		/*** Disturb coupling fault (CFds) ***/

		/* <0w0;0/1/-> <1w1;0/1/-> <1w0;0/1/-> <0w1;0/1/->*/
		// address base for test
		addr_base = 50;

		for (j = 0; j < 4; j++)
		{
			addr = addr_base + j;
			// apply fault
			devTest.apply_two_cell_static_fault(stringToOpcode(write_op[j], addr, 0), stringToOpcode("0", addr, 1), 1, NULL);
			faults.push_back(addr);
		}

		/* <0w0;1/0/-> <1w1;1/0/-> <1w0;1/0/-> <0w1;1/0/->*/
		// address base for test
		addr_base = 60;

		for (j = 0; j < 4; j++)
		{
			addr = addr_base + j;
			// apply fault
			devTest.apply_two_cell_static_fault(stringToOpcode(write_op[j], addr, 0), stringToOpcode("1", addr, 1), 0, NULL);
			faults.push_back(addr);
		}

		/* <0r0;0/1/-> <1r1;0/1/-> <0r0;1/0/-> <1r1;1/0/->*/
		// address base for test
		addr_base = 70;

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(read_op[j], addr, 0), stringToOpcode(bit_state[k], addr, 1), (k + 1) % 2, NULL);
				faults.push_back(addr);
			}
		}

		/*** Transition coupling fault (CFtr) ***/

		/* <0;1w0/1/-> <1;1w0/1/-> <0;0w1/0/-> <1;0w1/0/->*/
		addr_base = 82;

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(write_op[k + 2], addr, 0), (k + 1) % 2, NULL);
				faults.push_back(addr);
			}
		}

		/*** Write Destructive coupling fault (CFwd) ***/

		/* <0;0w0/1/-> <1;0w0/1/-> <0;1w1/0/-> <1;1w1/0/->*/
		addr_base = 90;

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(write_op[k], addr, 0), (k + 1) % 2, NULL);
				faults.push_back(addr);
			}
		}

		/*** Read Destructive coupling fault (CFrd) ***/

		/* <0;0r0/1/1> <1;0r0/1/1> <0;1r1/0/0> <1;1r1/0/0>*/
		addr_base = 102;

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(read_op[k], addr, 0), (k + 1) % 2, (k + 1) % 2);
				faults.push_back(addr);
			}
		}

		/*** Deceptive Read Destructive coupling fault (CFdr) ***/

		/* <0;0r0/1/0> <1;0r0/1/0> <0;1r1/0/1> <1;1r1/0/1> */
		addr_base = 114;

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(read_op[k], addr, 0), (k + 1) % 2, k);
				faults.push_back(addr);
			}
		}

		/*** Incorrect Read coupling fault (CFir) ***/

		/* <0;0r0/0/1> <1;0r0/0/1> <0;1r1/1/0> <1;1r1/1/0> */
		addr_base = 120;

		for (k = 0; k < 2; k++)
		{
			for (j = 0; j < 2; j++)
			{
				addr = addr_base++;
				// apply fault
				devTest.apply_two_cell_static_fault(stringToOpcode(bit_state[j], addr, 1), stringToOpcode(read_op[k], addr, 0), k, (k + 1) % 2);
				faults.push_back(addr);
			}
		}


		/// *** MARCH C TEST CODE ***
		cout << "Running March C... ";

		// clear list detected fails
		if (!failed.empty())
			failed.clear();

		// get start test time
		GetSystemTime(&start);

		// M0: w0
		for (addr = 0; addr < n; addr++)
		{

			//cout << addr << " ";
			for (i = 0; i < 8; i++)
			{
				devTest.write_bit(addr, i, 0);
			}
		}
		// M1 : ^ r0 w1
		for (addr = 0; addr < n; addr++)
		{
			//cout << addr << " ";
			for (i = 0; i < 8; i++)
			{
				if (devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1);
			}
		}
		// M2 : ^ r1 w0
		for (addr = 0; addr < n; addr++)
		{
			for (i = 0; i < 8; i++)
			{
				if (!devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0);
			}
		}
		// M3 : ~ r0 w1
		for (addr = (n - 1); addr >= 0; addr--)
		{
			//cout << addr << " ";
			for (i = 7; i >= 0; i--)
			{
				if (devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1);
			}
		}
		// M4 : ~ r1 w0
		for (addr = n - 1; addr >= 0; addr--){
			for (i = 7; i >= 0; i--){
				if (!devTest.read_bit(addr, i)){
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0);
			}
		}
		// M5 : r0
		for (addr = 0; addr < n; addr++){
			for (i = 0; i < 8; i++){
				if (devTest.read_bit(addr, i)){
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
			}
		}

		// get end test time
		GetSystemTime(&end);
		cout << "FINISHED" << endl << endl;

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;




		/// *** TEST REPORT ***
		cout << "\n >>>     MARCH C  Test Report     <<<" << endl << endl;

		failed.sort();
		failed.unique();

		n_fault = faults.size();
		n_fail = failed.size();

		cout << "Number of Fault Injected : " << n_fault << endl;
		cout << "Number of Fault Detected : " << n_fail << endl;
		printf("Detectation Rate: %.2f%%\n", (n_fail / n_fault) * 100);
		printf("The test duration was %dm%d.%03ds to %d bytes\n\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);

		// detailed fails
		printf("+--------------------+\n");
		printf("|   Detailed Faults  |\n");
		printf("+---------+----------+\n");
		printf("| Address | Detected |\n");
		printf("+---------+----------+\n");
		it2 = failed.begin();
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		{
			printf("|%7d  |    %s   |\n", *it, *it == *it2 ? "yes" : "no ");
			if (*it == *it2)
				it2++;
		}

		printf("+---------+----------+\n");

		/*// show lists of fault injected and fault found
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		cout << *it << ' ';
		cout << endl;

		for (list<int>::iterator it = failed.begin(); it != failed.end(); it++)
		cout << *it << ' ';
		cout << endl;
		//*/

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;
		printf("The test duration was %dm%d.%03ds to %d bytes\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);


		/// ***         END       ***
		/// *** MARCH C TEST CODE ***

		// ****************************************************************************************************8

		/// *** MARCH B TEST CODE ***
		cout << "Running March B... ";

		// clear list detected fails
		if (!failed.empty())
			failed.clear();

		// get start test time
		GetSystemTime(&start);

		// M0: w0
		for (addr = 0; addr < n; addr++)
		{

			//cout << addr << " ";
			for (i = 0; i < 8; i++)
			{
				devTest.write_bit(addr, i, 0);
			}
		}
		// M1 : ^ r0 w1 r1 w0 r0 w1
		for (addr = 0; addr < n; addr++)
		{
			//cout << addr << " ";
			for (i = 0; i < 8; i++)
			{
				if (devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1);
				if (!devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0);
				if (devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1);
			}
		}
		// M2 : ^ r1 w0 w1
		for (addr = 0; addr < n; addr++)
		{
			for (i = 0; i < 8; i++)
			{
				if (!devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0);
				devTest.write_bit(addr, i, 1);
			}
		}
		// M3 : ~ r1 w0 w1 w0
		for (addr = (n - 1); addr >= 0; addr--)
		{
			//cout << addr << " ";
			for (i = 7; i >= 0; i--)
			{
				if (!devTest.read_bit(addr, i))
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0);
				devTest.write_bit(addr, i, 1);
				devTest.write_bit(addr, i, 0);
			}
		}
		// M4 : ~ r0 w1 w0
		for (addr = n - 1; addr >= 0; addr--){
			for (i = 7; i >= 0; i--){
				if (devTest.read_bit(addr, i)){
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1);
				devTest.write_bit(addr, i, 0);
			}
		}
		// M5 : r0
		for (addr = 0; addr < n; addr++){
			for (i = 0; i < 8; i++){
				if (devTest.read_bit(addr, i)){
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
			}
		}

		// get end test time
		GetSystemTime(&end);
		cout << "FINISHED" << endl << endl;

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;




		/// *** TEST REPORT ***
		cout << "\n >>>     MARCH B  Test Report     <<<" << endl << endl;

		failed.sort();
		failed.unique();

		n_fault = faults.size();
		n_fail = failed.size();

		cout << "Number of Fault Injected : " << n_fault << endl;
		cout << "Number of Fault Detected : " << n_fail << endl;
		printf("Detectation Rate: %.2f%%\n", (n_fail / n_fault) * 100);
		printf("The test duration was %dm%d.%03ds to %d bytes\n\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);

		// detailed fails
		printf("+--------------------+\n");
		printf("|   Detailed Faults  |\n");
		printf("+---------+----------+\n");
		printf("| Address | Detected |\n");
		printf("+---------+----------+\n");
		it2 = failed.begin();
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		{
			printf("|%7d  |    %s   |\n", *it, *it == *it2 ? "yes" : "no ");
			if (*it == *it2)
				it2++;
		}

		printf("+---------+----------+\n");

		/*// show lists of fault injected and fault found
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		cout << *it << ' ';
		cout << endl;

		for (list<int>::iterator it = failed.begin(); it != failed.end(); it++)
		cout << *it << ' ';
		cout << endl;
		//*/

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;
		printf("The test duration was %dm%d.%03ds to %d bytes\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);


		/// ***         END       ***
		/// *** MARCH B TEST CODE ***

		/********************************************************************************************************/

		/// *** MARCH SS TEST CODE ***
		cout << "Running March SS... ";


		// clear list detected fails
		if (!failed.empty())
			failed.clear();


		// get start test time
		GetSystemTime(&start);

		// M0: w0
		for (addr = 0; addr < n; addr++)
		{

			//cout << addr << " ";
			for (i = 0; i < 8; i++)
			{
				devTest.write_bit(addr, i, 0);
			}
		}
		// M1 : ^ r0 r0 w0 r0 w1
		for (addr = 0; addr < n; addr++)
		{
			//cout << addr << " ";
			for (i = 0; i < 8; i++)
			{
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0); // w0
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1); // w1
			}
		}
		// M2 : ^ r1 r1 w1 r1 w0
		for (addr = 0; addr < n; addr++)
		{
			for (i = 0; i < 8; i++)
			{
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1); // w1
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0); // w0
			}
		}
		// M3 : ~ r0 r0 w0 r0 w1
		for (addr = (n - 1); addr >= 0; addr--)
		{
			//cout << addr << " ";
			for (i = 7; i >= 0; i--)
			{
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0); // w0
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1); // w1
			}
		}
		// M4 : ~ r1 r1 w1 r1 w0
		for (addr = n - 1; addr >= 0; addr--){
			for (i = 7; i >= 0; i--){
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1); // w1
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0); // w0
			}
		}
		// M5 : r0
		for (addr = 0; addr < n; addr++){
			for (i = 0; i < 8; i++){
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
			}
		}

		// get end test time
		GetSystemTime(&end);
		cout << "FINISHED" << endl << endl;

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;




		/// *** TEST REPORT ***
		cout << "\n >>>     MARCH SS  Test Report     <<<" << endl << endl;

		failed.sort();
		failed.unique();

		n_fault = faults.size();
		n_fail = failed.size();

		cout << "Number of Fault Injected : " << n_fault << endl;
		cout << "Number of Fault Detected : " << n_fail << endl;
		printf("Detectation Rate: %.2f%%\n", (n_fail / n_fault) * 100);
		printf("The test duration was %dm%d.%03ds to %d bytes\n\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);

		// detailed fails
		printf("+--------------------+\n");
		printf("|   Detailed Faults  |\n");
		printf("+---------+----------+\n");
		printf("| Address | Detected |\n");
		printf("+---------+----------+\n");
		it2 = failed.begin();
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		{
			printf("|%7d  |    %s   |\n", *it, *it == *it2 ? "yes" : "no ");
			if (*it == *it2)
				it2++;
		}

		printf("+---------+----------+\n");

		/*// show lists of fault injected and fault found
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		cout << *it << ' ';
		cout << endl;

		for (list<int>::iterator it = failed.begin(); it != failed.end(); it++)
		cout << *it << ' ';
		cout << endl;
		//*/

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;
		printf("The test duration was %dm%d.%03ds to %d bytes\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);


		/// ***         END       ***
		/// *** MARCH SS TEST CODE ***

		/********************************************************************************************************/

		/// *** PMOVI TEST CODE ***
		cout << "Running March SS... ";

		// clear list detected fails
		if (!failed.empty())
			failed.clear();

		// get start test time
		GetSystemTime(&start);

		// M0: ~ w0
		for (addr = (n - 1); addr >= 0; addr--)
		{

			//cout << addr << " ";
			for (i = 7; i >= 0; i--)
			{
				devTest.write_bit(addr, i, 0);
			}
		}
		// M1 : ^ r0 w1 r1
		for (addr = 0; addr < n; addr++)
		{
			//cout << addr << " ";
			for (i = 0; i < 8; i++)
			{
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1); // w1
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
			}
		}
		// M2 : ^ r1 w0 r0
		for (addr = 0; addr < n; addr++)
		{
			for (i = 0; i < 8; i++)
			{
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0); // w0
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
			}
		}
		// M3 : ~ r0 w1 r1
		for (addr = (n - 1); addr >= 0; addr--)
		{
			//cout << addr << " ";
			for (i = 7; i >= 0; i--)
			{
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 1); // w1
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
			}
		}
		// M4 : ~ r1 w0 r0
		for (addr = n - 1; addr >= 0; addr--){
			for (i = 7; i >= 0; i--){
				if (!devTest.read_bit(addr, i)) // r1
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
				devTest.write_bit(addr, i, 0); // w0
				if (devTest.read_bit(addr, i)) // r0
				{
					//cout << "erro : bit " << i << " address " << addr << endl;
					failed.push_back(addr);
				}
			}
		}

		// get end test time
		GetSystemTime(&end);
		cout << "FINISHED" << endl << endl;

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;




		/// *** TEST REPORT ***
		cout << "\n >>>     PMOVI  Test Report     <<<" << endl << endl;

		failed.sort();
		failed.unique();

		n_fault = faults.size();
		n_fail = failed.size();

		cout << "Number of Fault Injected : " << n_fault << endl;
		cout << "Number of Fault Detected : " << n_fail << endl;
		printf("Detectation Rate: %.2f%%\n", (n_fail / n_fault) * 100);
		printf("The test duration was %dm%d.%03ds to %d bytes\n\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);

		// detailed fails
		printf("+--------------------+\n");
		printf("|   Detailed Faults  |\n");
		printf("+---------+----------+\n");
		printf("| Address | Detected |\n");
		printf("+---------+----------+\n");
		it2 = failed.begin();
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		{
			printf("|%7d  |    %s   |\n", *it, *it == *it2 ? "yes" : "no ");
			if (*it == *it2)
				it2++;
		}

		printf("+---------+----------+\n");

		/*// show lists of fault injected and fault found
		for (list<int>::iterator it = faults.begin(); it != faults.end(); it++)
		cout << *it << ' ';
		cout << endl;

		for (list<int>::iterator it = failed.begin(); it != failed.end(); it++)
		cout << *it << ' ';
		cout << endl;
		//*/

		// get test duration
		duration.wHour = end.wHour - start.wHour;
		duration.wMinute = end.wMinute - start.wMinute;
		duration.wSecond = end.wSecond - start.wSecond;
		duration.wMilliseconds = end.wMilliseconds - start.wMilliseconds;

		//cout << end << endl;
		printf("The test duration was %dm%d.%03ds to %d bytes\n", duration.wMinute, duration.wSecond, duration.wMilliseconds, n);


		/// ***         END       ***
		/// *** PMOVI TEST CODE ***

		/*************************************************************************************************************/

		break;

	default:
		cout << "Choose a value to TESTE" << endl;
		break;
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	int i; // Control Variables
	int d = 100; // number of devices
	int f = 1; // number of each faults
	int size = 2*1024; // memory size 2KB

	// Report variables
	SYSTEMTIME duration;
	// create list for testing diag
	list < int > faults;
	list < int > failed;
	//list<int>::iterator it, it2;
	int n_fault;
	int n_fail[4];

	//printf("%d %s", argc, argv[0]);
	// header
	printf("Device Faults MARCH_B MARCH_C MARCH_SS PMOVI\n");

	// Quantidade de memorias
	for (i = 0; i < d; i++)
	{
		//printf("\n >>>          MEMORY %d           <<<\n\n", i + 1);
		MemoryDevice devTest1 = MemoryDevice(size);
		int n = (devTest1).get_number_of_blocks();

		//cout << "Injecting fault into the memory... ";
		
		// clear fault list
		if (!(faults).empty())
			(faults).clear();

		//randomInsertRandomFaults(false, &devTest1, &faults, f);

		// FaultTypes : WDF, TF, RDF, DRDF, IRF, CFds, CFtr, CFwd, CFrd, CFdr, CFir
		randomInsertFaults(false, &devTest1, &faults, CFds, f);
		n_fault = faults.size();
		//printf("%d\n", n_fault);

		//cout << "FINISHED. " << endl;

		// *** March B *** //
		runMarchBTest(&devTest1, &duration, &failed);
		//cout << "FINISHED. " << endl;
		n_fail[0] = failed.size();
		
		// *** March C *** //
		//printf("Running Test March C on memory %2d... ", i + 1);
		runMarchCTest(&devTest1, &duration, &failed);
		//cout << "FINISHED. " << endl;
		n_fail[1] = failed.size();

		// *** March SS *** //
		runMarchSSTest(&devTest1, &duration, &failed);
		//cout << "FINISHED. " << endl;
		n_fail[2] = failed.size();
		
		// *** PMOVI *** //
		runPMOVITest(&devTest1, &duration, &failed);
		//cout << "FINISHED. " << endl;
		n_fail[3] = failed.size();
		
		// print results
		printf("%06d %6d %7d %7d %8d %5d\n", i + 1, n_fault, n_fail[0], n_fail[1], n_fail[2], n_fail[3]);
	}
}

