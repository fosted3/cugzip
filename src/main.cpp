#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>

//The CRC32 function, and its corresponding table, are available on
//http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/libkern/crc32.c
//as open source.

static uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

uint32_t crc32(uint32_t crc, const std::vector<uint8_t> *data, size_t offset, size_t length)
{
	crc = crc ^ ~0U;
	for (size_t i = offset; i < offset + length; i++)
	{
		crc = crc32_tab[(crc ^ (*data)[i]) & 0xFF] ^ (crc >> 8);
	}
	return crc ^ ~0U;
}

uint32_t get_three(const std::vector<uint8_t> *data, size_t offset)
{
	return (uint32_t) 0x00FFFFFF & ((data -> at(offset + 2) <<  16) | (data -> at(offset + 1) << 8) | (data -> at(offset)));
}

struct lz77_data
{
	std::vector<uint8_t>  *uncompressed_data;	//Unmodified data
	std::vector<bool>     *is_match;			//Where true, data can be replaced with a length-distance pair
	std::vector<bool>     *is_reference;		//Where true, data is reference to length-distance pair
	std::vector<uint8_t>  *length;				//Length data when has_match is true
	std::vector<uint16_t> *distance;			//Distance data when has_match is true
};

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
	size_t return_val = 3;
	bool all_match = true;
	for (size_t length = 4; length <= max_length; length++)
	{
		uint8_t first_byte = data -> at(matches -> at(0) + length - 1);
		for (size_t i = 1; i < matches -> size(); i++)
		{
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
	//std::vector<uint32_t> crc_data;
	//crc_data.reserve(data -> size() - 2);
	//std::vector<size_t> index_data;
	//index_data.reserve(data -> size() - 2);
	std::map<uint32_t, std::vector<size_t> > match_table;
	std::map<uint32_t, std::vector<size_t> >::iterator itr;
	uint32_t match_temp;
	struct lz77_data *lz77_return = new lz77_data;
	lz77_return -> uncompressed_data = data;
	lz77_return -> is_match = new std::vector<bool>(data -> size(), false);
	lz77_return -> is_reference = new std::vector<bool>(data -> size(), false);
	lz77_return -> length = new std::vector<uint8_t>(data -> size());
	lz77_return -> distance = new std::vector<uint16_t>(data -> size());
	/*for (size_t str_len = 258; str_len > 255; str_len--)
	{
		std::cout << str_len << std::endl;
		//crc_data.resize(data -> size() + 1 - str_len);
		//index_data.resize(data -> size() + 1 - str_len);
		hash_table.clear();
		for (size_t i = 0; i < data -> size() + 1 - str_len; i++)
		{
			if (lz77_return -> has_match[i]) { continue; }
			hash_temp = crc32(0, data, i, str_len);
			itr = hash_table.find(hash_temp);
			if (itr == hash_table.end()) //not found - insert into hash table
			{
				hash_table.insert(std::make_pair(hash_temp, std::vector<size_t>(0)));
				hash_table[hash_temp].push_back(i);
			}
			else
			{
				hash_table[hash_temp].push_back(i);
			}
			//crc_data[i] = crc32(0, data, i, str_len);
			//index_data[i] = i;
		}
		
		//now that the hash table has been generated - we can start doing replacements
		//one thing to check is that there are no repitions of substrings within the found substrings.
		//e.g. abcabcabcabc would be better compressed with abc[x,y] than abcabc[x,y]
		for (uint8_t substr_div = 2; str_len / substr_dv > 3; substr_div++)
		{
			if (str_len % substr_div) { continue; }
			else
			{
			}
		}
	}*/
	for (size_t i = 0; i < data -> size() - 2; i++)
	{
		match_temp = get_three(data, i);
		itr = match_table.find(match_temp);
		if (itr == match_table.end())
		{
			match_table.insert(std::make_pair(match_temp, std::vector<size_t>(1, i)));
			std::cout << "New match " << match_temp << std::endl;
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
				if (j && matches -> at(j) - matches -> at(j - 1) < 0x1000) //We have reference within range
				{
					
					for (size_t k = 0; k < full_length; k++)
					{
						(*(lz77_return -> is_match))[matches -> at(j) + k] = true;
					}
				}
				else if (j < matches -> size() - 1 && matches -> at(j + 1) - matches -> at(j) < 0x1000) //We have matches after within range & not a current match
				{
					if (matches -> at(j + 1) - matches -> at(j) > full_length
					for (size_t k = 0; k < full_length; k++)
					{
						(*(lz77_return -> is_reference))[matches -> at(j) + k] = true;
					}
				}
				/*for (size_t k = 0; k < full_length; k++)
				{
					if (j)
					{
						(*(lz77_return -> is_match))[matches -> at(j) + k] = true;
					}
					else
					{
						(*(lz77_return -> is_reference))[matches -> at(j) + k] = true;
					}
				}*/
			}
		}
	}
//(*(lz77_return -> length))[matches -> at(j)] = 
//(*(lz77_return -> distance))[matches -> at(j)] = matches
	return lz77_return;
}

int main(int argc, char **argv)
{
	std::vector<uint8_t> test_input(1024*1024); //One MiB of input
	for (size_t i = 0; i < test_input.size(); i++) { test_input[i] = (uint8_t) (i & 0x000000FF); }
	lz77_cpu(&test_input);
	return 0;
}
