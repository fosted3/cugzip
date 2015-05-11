#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <ctime>
#include <cstring>
#include <fstream>
#include "aux.h"
#include "cu_header.h"

// The lz77_cpu function performs serial length-distance encoding of the input.

lz77_data* lz77_cpu(std::vector<uint8_t> *data)
{
	std::map<uint32_t, size_t> match_table;
	std::map<uint32_t, size_t>::iterator itr;
	uint32_t match_temp;
	struct lz77_data *lz77_return = new lz77_data;
	lz77_return -> uncompressed_data = data;
	lz77_return -> is_match = new std::vector<bool>(data -> size(), false);
	lz77_return -> length = new std::vector<uint8_t>;
	lz77_return -> distance = new std::vector<uint16_t>;
	lz77_return -> index = new std::vector<size_t>;
	for (size_t i = 0; i < data -> size() - 2; i++)
	{
		match_temp = get_three(data, i);
		itr = match_table.find(match_temp);
		if (itr == match_table.end())
		{
			match_table.insert(std::make_pair(match_temp, i));
		}
		else
		{
			if (i - (itr -> second) < 0x1000 && !(lz77_return -> is_match -> at(i)))
			{
				lz77_return -> distance -> push_back(i - (itr -> second) - 1); //
				lz77_return -> index -> push_back(i);
				size_t a = itr -> second + 2;
				size_t b = i + 2;
				uint8_t len = 0;
				for (size_t c = 0; c < 255; c++)
				{
					if (b + c < data -> size() && data -> at(a + c) == data -> at(b + c))
					{
						len++;
					}
					else
					{
						break;
					}
				}
				lz77_return -> length -> push_back(len);
				for (size_t j = 0; j < ((size_t) len) + 3; j++)
				{
					(*(lz77_return -> is_match))[i + j] = true;
				}
			}
			itr -> second = i;
		}
	}
	return lz77_return;
}

void populate_huffman_static(huffman_table *table)
{
	for (size_t i = 0; i < 288; i++)
	{
		if (i < 144)
		{
			table -> codes[i] = 0x30 + i;
			table -> length[i] = 8;
		}
		else if (i < 256)
		{
			table -> codes[i] = 0x190 + i - 144;
			table -> length[i] = 9;
		}
		else if (i < 280)
		{
			table -> codes[i] = i - 256;
			table -> length[i] = 7;
		}
		else
		{
			table -> codes[i] = 0xC0 + i - 280;
			table -> length[i] = 8;
		}
	}
}

void populate_huffman_dynamic(huffman_table *table, lz77_data *data)
{
	std::cerr << "Dynamic huffman tables not implemented yet. :(" << std::endl;
	populate_huffman_static(table);
}



bool deflate_cpu(options *args, blocks *compressed_data)
{
	std::vector<uint8_t> *file;
	if (args -> infile.compare("STDIN") == 0)
	{
		file = new std::vector<uint8_t>;
		char input;
		while (std::cin >> input)
		{
			file -> push_back((uint8_t) input);
		}
		compressed_data -> isize = file -> size();
	}
	else
	{
		if (!file_exists(args -> infile)) { return false; }
		std::fstream infile(args -> infile, std::ios::in | std::ios::binary);
		infile.seekg(0, infile.end);
		compressed_data -> isize = infile.tellg();
		infile.seekg(0, infile.beg);
		file = new std::vector<uint8_t>(compressed_data -> isize);
		uint8_t temp;
		for (size_t i = 0; i < compressed_data -> isize; i++)
		{
			infile.read((char*) &temp, 1);
			(*file)[i] = temp;
		}
	}
	compressed_data -> crc32 = crc32(0, file, 0, compressed_data -> isize);
	lz77_data *ld_coded = lz77_cpu(file);
	huffman_table huf_tab;
	bool static_huffman;
	if (args -> force_static) //force static
	{
		populate_huffman_static(&huf_tab);
		static_huffman = true;
	}
	else if (args -> force_dynamic) //force dynamic huffman tree generation
	{
		populate_huffman_dynamic(&huf_tab, ld_coded);
		static_huffman = false;
	}
	else //default static
	{
		populate_huffman_static(&huf_tab);
		static_huffman = true;
	}
	compressed_data -> data = new std::vector<uint8_t>;
	uint8_t *buf = new uint8_t[1024*1024*2];	//2 MiB of buffer. The buffer is written to file every time there is more than
	size_t cur_byte = 0;						//1 MiB of output & the output ends on a byte boundary.
	uint8_t cur_bit = 0;						//void bit_pack(uint8_t *buf, uint32_t data, uint8_t bits, size_t *cur_byte, uint8_t *cur_bit, bool lsb_first)
	size_t cur_ld_pair = 0;
	if (!static_huffman)						//Pack the huffman tree
	{
		std::cerr << "Dynamic huffman tables not implemented yet. :(" << std::endl;
	}
	else
	{
		if (compressed_data -> final)
		{
			bit_pack(buf, (uint32_t) 0x03, 3, &cur_byte, &cur_bit);
		}
		else
		{
			bit_pack(buf, (uint32_t) 0x01, 3, &cur_byte, &cur_bit);
		}
	}
	for (size_t i = 0; i < ld_coded -> uncompressed_data -> size(); i++)
	{
		if (cur_ld_pair < ld_coded -> index -> size() && i == ld_coded -> index -> at(cur_ld_pair)) //Pack in ld pair
		{
			uint32_t code;
			uint8_t code_length;
			uint16_t code_id;
			uint32_t extra_bits;
			uint8_t ld_length = ld_coded -> length -> at(cur_ld_pair);
			uint16_t ld_dist = ld_coded -> distance -> at(cur_ld_pair);
			if (ld_length < 8)		//this if - else if - else block could be written more efficiently
			{
				code = huf_tab.codes[((uint16_t) ld_length) + 257];
				code_length = huf_tab.length[((uint16_t) ld_length) + 257];
			}
			else if (ld_length < 16)
			{
				code_id = ((ld_length - 8) / 2) + 265;
				extra_bits = (ld_length - 8) % 2;
				code = huf_tab.codes[code_id] << 1 | extra_bits;
				code_length = huf_tab.length[code_id] + 1;
			}
			else if (ld_length < 32)
			{
				code_id = ((ld_length - 16) / 4) + 269;
				extra_bits = (ld_length - 16) % 4;
				code = huf_tab.codes[code_id] << 2 | extra_bits;
				code_length = huf_tab.length[code_id] + 2;
			}
			else if (ld_length < 64)
			{
				code_id = ((ld_length - 32) / 8) + 273;
				extra_bits = (ld_length - 32) % 8;
				code = huf_tab.codes[code_id] << 3 | extra_bits;
				code_length = huf_tab.length[code_id] + 3;
			}
			else if (ld_length < 128)
			{
				code_id = ((ld_length - 64) / 16) + 277;
				extra_bits = (ld_length - 64) % 16;
				code = huf_tab.codes[code_id] << 4 | extra_bits;
				code_length = huf_tab.length[code_id] + 4;
			}
			else if (ld_length < 254)
			{
				code_id = ((ld_length - 128) / 32) + 281;
				extra_bits = (ld_length - 128) % 32;
				code = huf_tab.codes[code_id] << 5 | extra_bits;
				code_length = huf_tab.length[code_id] + 5;
			}
			else
			{
				code = huf_tab.codes[285];
				code_length = huf_tab.length[285];
			}
			//printf("packing %x length %d byte %x bit %d\n", code, code_length, cur_byte, cur_bit);
			bit_pack(buf, code, code_length, &cur_byte, &cur_bit);
			if (ld_dist < 4)
			{
				code = ld_dist;
				code_length = 5;
			}
			else
			{
				uint32_t mod = 2;
				uint8_t shift = 1;
				uint32_t offset = 4;
				uint32_t greater_than = 8;
				while (ld_dist >= greater_than)
				{
					mod <<= 1;
					shift++;
					offset += 2;
					greater_than <<= 1;
				}
				code = ((ld_dist - (mod << 1)) / mod) + offset;
				code = (code << shift) | ((ld_dist - (mod << 1)) % mod);
				code_length = 5 + shift;
			}
			//printf("packing %x length %d byte %x bit %d\n", code, code_length, cur_byte, cur_bit);
			bit_pack(buf, code, code_length, &cur_byte, &cur_bit);
			cur_ld_pair++;
		}
		else if (!ld_coded -> is_match -> at(i)) //pack literal data
		{
			//printf("packing %x length %d byte %x bit %d\n", huf_tab.codes[ld_coded -> uncompressed_data -> at(i)], huf_tab.length[ld_coded -> uncompressed_data -> at(i)], cur_byte, cur_bit);
			bit_pack(buf, huf_tab.codes[ld_coded -> uncompressed_data -> at(i)], huf_tab.length[ld_coded -> uncompressed_data -> at(i)], &cur_byte, &cur_bit);
		}
		if (cur_byte > 1 && cur_bit == 0)
		{
			compressed_data -> data -> reserve(compressed_data -> data -> size() + cur_byte);
			for(size_t j = 0; j < cur_byte; j++)
			{
				//printf("pushing %x\n", buf[j]);
				compressed_data -> data -> push_back(buf[j]);
			}
			cur_byte = 0;
		}
	}
	bit_pack(buf, huf_tab.codes[256], huf_tab.length[256], &cur_byte, &cur_bit);
	if (compressed_data -> final)
	{
		bit_pack(buf, 0x00, 8 - cur_bit, &cur_byte, &cur_bit);
		assert(cur_bit == 0);
	}
	compressed_data -> data -> reserve(compressed_data -> data -> size() + cur_byte);
	for(size_t j = 0; j < cur_byte; j++)
	{
		compressed_data -> data -> push_back(buf[j]);
	}
	delete[] buf;
	delete file;
	delete ld_coded -> is_match;
	delete ld_coded -> length;
	delete ld_coded -> distance;
	delete ld_coded -> index;
	delete ld_coded;
	return true;
}

bool deflate_gpu(options *args, blocks *compressed_data)
{
	return false;
}

bool deflate(options *args)
{
	gzip_file compressed_file;
	compressed_file.id1 = 31;
	compressed_file.id2 = 139;
	compressed_file.cm = 8;
	compressed_file.flg = FNAME;
	compressed_file.mtime = time(0);
	compressed_file.xfl = 0;
	compressed_file.os = 3;
	compressed_file.xlen = 0;
	compressed_file.extra = 0;
	compressed_file.fname = new char[args -> infile.size() + 1];
	args -> infile.copy(compressed_file.fname, args -> infile.size());
	compressed_file.fname[args -> infile.size()] = 0;
	compressed_file.fcomment = 0;
	compressed_file.crc16 = 0;
	blocks compressed_data;
	if (args -> thread)
	{
		std::cerr << "Threading not implemented yet. :(" << std::endl;
		exit(1);
	}
	else
	{
		compressed_data.final = true;
		bool success;
		if (args -> gpu) { success = deflate_gpu(args, &compressed_data); }
		else { success = deflate_cpu(args, &compressed_data); }
		compressed_file.blocks = compressed_data.data -> data();
		if (compressed_file.blocks == NULL || !success) { return false; }
		compressed_file.crc32 = compressed_data.crc32;
		compressed_file.isize = compressed_data.isize;
	}
	if (args -> outfile.compare("STDOUT") == 0)
	{
		std::cout.write((const char*) &compressed_file.id1,	1);	//if uint8_t isn't one byte you have problems
		std::cout.write((const char*) &compressed_file.id2,	1);
		std::cout.write((const char*) &compressed_file.cm,	1);
		std::cout.write((const char*) &compressed_file.flg,	1);
		std::cout.write((const char*) &compressed_file.mtime,	4);
		std::cout.write((const char*) &compressed_file.xfl,	1);
		std::cout.write((const char*) &compressed_file.os,	1);
		std::cout.write((const char*) compressed_file.fname,	args -> infile.size() + 1);
		std::cout.write((const char*) compressed_file.blocks, compressed_data.data -> size());
		std::cout.write((const char*) &compressed_file.crc32, 4);
		std::cout.write((const char*) &compressed_file.isize, 4);
		std::cout.flush();
		if (!std::cout.good()) { return false; }
	}
	else
	{
		std::fstream outfile(args -> outfile, std::ios::out | std::ios::binary);
		outfile.seekp(0);
		outfile.write((const char*) &compressed_file.id1,	1);	//if uint8_t isn't one byte you have problems
		outfile.write((const char*) &compressed_file.id2,	1);
		outfile.write((const char*) &compressed_file.cm,	1);
		outfile.write((const char*) &compressed_file.flg,	1);
		outfile.write((const char*) &compressed_file.mtime,	4);
		outfile.write((const char*) &compressed_file.xfl,	1);
		outfile.write((const char*) &compressed_file.os,	1);
		outfile.write((const char*) compressed_file.fname,	args -> infile.size() + 1);
		outfile.write((const char*) compressed_file.blocks, compressed_data.data -> size());
		outfile.write((const char*) &compressed_file.crc32, 4);
		outfile.write((const char*) &compressed_file.isize, 4);
		outfile.flush();
		if (!outfile.good()) { return false; }
		outfile.close();
	}
	delete compressed_data.data;
	delete[] compressed_file.fname;
	return true;
}


int main(int argc, char **argv)
{
	options args;
	setup_args(argc, argv, &args);
	if (deflate(&args)) { return 0; }
	std::cerr << "Error compressing data." << std::endl;
	return 1;
}
