#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <map>
//#include <unordered_set>
#include <cassert>
#include "aux.h"
#include "cu_header.h"

uint32_t get_three(const std::vector<uint8_t> *data, size_t offset)
{
	return (uint32_t) 0x00FFFFFF & ((data -> at(offset + 2) <<  16) | (data -> at(offset + 1) << 8) | (data -> at(offset)));
}

size_t expand_matches(std::vector<uint8_t> *data, std::vector<size_t> *matches)
{
	assert(matches -> size() > 1);
	size_t max_length = 258;
	for (size_t i = 0; i < matches -> size() - 1; i++)
	{
		if (max_length > (matches -> at(i + 1) - matches -> at(i))) { max_length = matches -> at(i + 1) - matches -> at(i); }
	}
	if (data -> size() - matches -> at(matches -> size() - 1) < max_length)
	{
		max_length = data -> size() - matches -> at(matches -> size() - 1);
	}
	if (max_length == 2) { max_length = 4; }
	if (max_length == 1) { return 1; }
	size_t return_val = 3;
	bool all_match = true;
	for (size_t length = 4; length <= max_length; length++)
	{
		uint8_t first_byte = data -> at(matches -> at(0) + length - 1);
		for (size_t i = 1; i < matches -> size(); i++)
		{
			if (!((matches -> at(i) + length - 1) < data -> size()))
			{
				all_match = false;
				break;
			}
			if (data -> at(matches -> at(i) + length - 1) != first_byte)
			{
				all_match = false;
				break;
			}
		}
		if (all_match) { return_val = length; }
		else { break; }
	}
	return return_val;
}

/*	The lz77_cpu function performs length-distance encoding of the input.
 *	
 */

lz77_data* lz77_cpu(std::vector<uint8_t> *data)
{
	std::map<uint32_t, std::vector<size_t> > match_table;
	std::map<uint32_t, std::vector<size_t> >::iterator itr;
	uint32_t match_temp;
	struct lz77_data *lz77_return = new lz77_data;
	lz77_return -> uncompressed_data = data;
	lz77_return -> is_match = new std::vector<bool>(data -> size(), false);
	lz77_return -> is_reference = new std::vector<bool>(data -> size(), false);
	lz77_return -> length = new std::vector<uint8_t>;
	lz77_return -> distance = new std::vector<uint16_t>;
	lz77_return -> index = new std::vector<size_t>;
	for (size_t i = 0; i < data -> size() - 2; i++)
	{
		match_temp = get_three(data, i);
		itr = match_table.find(match_temp);
		if (itr == match_table.end())
		{
			match_table.insert(std::make_pair(match_temp, std::vector<size_t>(1, i)));
			//std::cout << "New match " << match_temp << std::endl;
		}
		else
		{
			itr -> second.push_back(i);
			//std::cout << "Match found, vector size is now " << itr -> second.size() << std::endl;
		}
	}
	for (size_t i = 0; i < data -> size() - 2; i++)
	{
		match_temp = get_three(data, i);
		std::vector<size_t> *matches = &(match_table[match_temp]);
		if (matches -> size() > 1 && !(lz77_return -> is_match -> at(i)) && !(lz77_return -> is_reference -> at(i)))
		{
			size_t full_length = expand_matches(data, matches);
			std::cout << "Expanded match " << match_temp << " to " << full_length << std::endl;
			for (size_t j = 0; j < matches -> size(); j++)
			{
				//std::cout << j << std::endl;
				if (j && matches -> at(j) - matches -> at(j - 1) < 0x1000 && !(lz77_return -> is_reference -> at(matches -> at(j)))) //We have reference or match within range
				{
					if (matches -> at(j) - matches -> at(j - 1) > full_length) //not edge case
					{
						std::cout << "Not an edge case" << std::endl;
						lz77_return -> length -> push_back(full_length - 3);
						lz77_return -> distance -> push_back(matches -> at(j) - matches -> at(j-1) - 1);
						lz77_return -> index -> push_back(matches -> at(j));
						for (size_t k = 0; k < full_length; k++)
						{
							(*(lz77_return -> is_match))[matches -> at(j) + k] = true;
						}
					}
					else if (matches -> at(j) - matches -> at(j - 1) == full_length && full_length > 2) //Right after (e.g. abcabc), need to check if there's a repeated pattern
					{
						size_t actual_length = full_length;
						for (size_t k = j + 1; k < matches -> size(); k++)
						{
							if ((matches -> at(k) == matches -> at(k - 1) - actual_length) && (full_length + actual_length < 259))
							{
								actual_length += full_length;
							}
							else { break; }
						}
						lz77_return -> length -> push_back(actual_length - 3);
						lz77_return -> distance -> push_back(matches -> at(j) - matches -> at(j - 1) - 1);
						lz77_return -> index -> push_back(matches -> at(j));
						for (size_t k = 0; k < actual_length; k++)
						{
							(*(lz77_return -> is_match))[matches -> at(j) + k] = true;
						}
						std::cout << "Repeated patern case" << std::endl;
						j += (actual_length / full_length) - 1;
						//for (size_t k = 0; k < matches -> size(); k++) { std::cout << matches -> at(k) << " "; }
						//std::cout << std::endl;
					}
					else if (full_length == 4) //repeated two bytes, e.g. abababab, but needs to have more than 4 2-pairs
					{
						std::cout << "Repeated 2-byte case" << std::endl;
						size_t start = j;
						while(j < matches -> size() - 2 && matches -> at(j + 2) - matches -> at(j) == 2) { j += 2; }
						for (size_t q = 0; q < data -> size(); q++) { std::cout << (char) data -> at(q) << " "; }
						std::cout << std::endl;
						for (size_t q = 0; q < data -> size(); q++)
						{
							if (q == matches -> at(start) || q == matches -> at(j) + 4) { std::cout << "x "; }
							else { std::cout << "  "; }
						}
						std::cout << std::endl;
						
					}
					else //repeated byte e.g. aaaaaaaaa
					{
						std::cout << "Repeated byte case" << std::endl;
						//assert(j + 1 < matches -> size());
						size_t start = j;
						//std::cout << "j: " << j << std::endl;
						while ((j + 1 < matches -> size()) && matches -> at(j) + 1 == matches -> at(j + 1)) { j++; }
						//if (j - start < 2) { continue; }
						//std::cout << j - start << std::endl;
						lz77_return -> length -> push_back(j - start);
						lz77_return -> distance -> push_back(2);
						lz77_return -> index -> push_back(matches -> at(start));
						for (size_t l = matches -> at(start); l < matches -> at(j) + 3; l++)
						{
							(*(lz77_return -> is_match))[l] = true;
						}
						for (size_t q = 0; q < data -> size(); q++) { std::cout << (char) data -> at(q) << " "; }
						std::cout << std::endl;
						for (size_t q = 0; q < data -> size(); q++)
						{
							if (q == matches -> at(start) || q == matches -> at(j) + 2) { std::cout << "x "; }
							else { std::cout << "  "; }
						}
						std::cout << std::endl;
					}
				}
				else if (j < matches -> size() - 1 && matches -> at(j + 1) - matches -> at(j) < 0x1000 && !(lz77_return -> is_reference -> at(matches -> at(j)))) //We have matches after within range & not a current match
				{
					std::cout << "Setting up reference. Full length: " << full_length << std::endl;
					if (full_length == 1)
					{
						for (size_t k = 0; k < 3; k++)
						{
							(*(lz77_return -> is_reference))[matches -> at(j) + k] = true;
						}
					}
					else if (full_length == 2)
					{
						for (size_t k = 0; k < 4; k++)
						{
							(*(lz77_return -> is_reference))[matches -> at(j) + k] = true;
						}
					}
					else
					{
						for (size_t k = 0; k < full_length; k++)
						{
							(*(lz77_return -> is_reference))[matches -> at(j) + k] = true;
						}
					}
				}
			}
		}
	}
	return lz77_return;
}

int main(int argc, char **argv)
{
	for (int i = 0; i < argc; i++) { std::cout << argv[i] << std::endl; }
	std::vector<uint8_t> test_input(1024*1024); //One MiB of input
	//std::vector<uint8_t> test_input = {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b', 'c', 'd', 'b', 'c', 'd', 'f', 'g', 'f', 'g', 'f', 'g', 'f', 'g', 'f', 'g', 'q', 'q', 'q', 'q', 'q', 'q'};
	//for (size_t i = 0; i < test_input.size(); i++) { test_input[i] = (uint8_t) (i & 0x000000FF); }
	//for (size_t i = 0; i < test_input.size(); i++) { test_input[i] = 0; }
	//struct lz77_data *data = lz77_cpu(&test_input);
	lz77_cuda(&test_input);
	//for (size_t i = 0; i < data -> index -> size(); i++) { std::cout << "D: " << data -> distance -> at(i) << ", L: " << (unsigned int) data -> length -> at(i) << ", I: " << data -> index -> at(i) << std::endl; }
	return 0;
}
