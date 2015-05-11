#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
//#include <unordered_set>
//#include <queue>
#include <cassert>
#include <ctime>
#include <cstring>
#include <fstream>
#include "aux.h"
#include "cu_header.h"


uint32_t get_three(const std::vector<uint8_t> *data, size_t offset)
{
	return (uint32_t) 0x00FFFFFF & ((data -> at(offset + 2) <<  16) | (data -> at(offset + 1) << 8) | (data -> at(offset)));
}

/*	The lz77_cpu function performs length-distance encoding of the input.
 *	
 */

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
				//printf("Found new match, i: %lu, distance: %x, length: %x\n", i, i - (itr -> second) - 1, len);
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

void setup_args(int argc, char **argv, options *args)
{
	args -> infile = "";
	args -> outfile = "";
	args -> keep = false;
	args -> verbose = false;
	args -> gpu = false;
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-' && argv[i][1] != '-') //Beginning of single char options
		{
			size_t j = 1;
			while (argv[i][j] != 0) //Null terminated
			{
				if (argv[i][j] == 'k')
				{
					args -> keep = true;
				}
				else if (argv[i][j] == 'v')
				{
					args -> verbose = true;
				}
				else if (argv[i][j] == 'c')
				{
					args -> outfile = "STDOUT";
				}
				else if (argv[i][j] == 'g')
				{
					args -> gpu = true;
				}
			}
		}
		else if (argv[i][0] == '-' && argv[i][1] == '-') //Single word options
		{
			if (strcmp(argv[i], "--stdout") == 0 || strcmp(argv[i], "--to-stdout") == 0)
			{
				args -> outfile = "STDOUT";
			}
			else if (strcmp(argv[i], "--keep") == 0)
			{
				args -> keep = true;
			}
			else if (strcmp(argv[i], "--verbose") == 0)
			{
				args -> verbose = true;
			}
			else if (strcmp(argv[i], "--gpu") == 0)
			{
				args -> gpu = true;
			}
			else
			{
				std::cerr << "arg " << argv[i] << " unrecognized." << std::endl;
			}
		}
		else //Filename, this *should* be the last arg...
		{
			args -> infile = argv[i];
			if (args -> outfile.compare("") == 0)
			{
				args -> outfile = args -> infile;
				args -> outfile += ".gz";
			}
		}
	}
	if (args -> infile.compare("") == 0) { args -> infile = "STDIN"; }
	if (args -> infile.compare("") == 0 || args -> outfile.compare("") == 0)
	{
		std::cerr << "input file or output file missing. exiting." << std::endl;
		exit(1);
	}
}

bool deflate_cpu(options *args, blocks *compressed_data)
{
	return false;
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
	compressed_file.flg = FNAME | FCOMMENT;
	compressed_file.mtime = time(0);
	compressed_file.xfl = 0;
	compressed_file.os = 3;
	compressed_file.xlen = 0;
	compressed_file.extra = 0;
	compressed_file.fname = new char[args -> infile.size() + 1];
	args -> infile.copy(compressed_file.fname, args -> infile.size());
	compressed_file.fname[args -> infile.size()] = 0;
	char comment[] = "Made with cugzip.";
	compressed_file.fcomment = comment; //This remvoes the write-string warning about depriciated const char converison
	compressed_file.crc16 = 0;
	blocks compressed_data;
	bool success;
	if (args -> gpu) { success = deflate_gpu(args, &compressed_data); }
	else { success = deflate_cpu(args, &compressed_data); }
	compressed_file.blocks = compressed_data.data;
	if (compressed_file.blocks == NULL || !success) { return false; }
	compressed_file.crc32 = compressed_data.crc32;
	compressed_file.isize = compressed_data.isize;
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
	outfile.write((const char*) compressed_file.fcomment, 18);
	outfile.write((const char*) compressed_file.blocks, compressed_data.osize);
	outfile.write((const char*) &compressed_file.crc32, 4);
	outfile.write((const char*) &compressed_file.isize, 4);
	outfile.flush();
	if (!outfile.good()) { return false; }
	outfile.close();
	delete[] compressed_data.data;
	delete[] compressed_file.fname;
	return true;
}


int main(int argc, char **argv)
{
	//for (int i = 0; i < argc; i++) { std::cout << argv[i] << std::endl; }
	//std::vector<uint8_t> test_input(1024*1024*128); //128 MiB of input
	//std::vector<uint8_t> test_input = {'a', 'a', 'a', 'a', 'a', 'a', 'a', 'b', 'c', 'd', 'b', 'c', 'd', 'f', 'g', 'f', 'g', 'f', 'g', 'f', 'g', 'f', 'g', 'q', 'q', 'q', 'q', 'q', 'q'};
	//for (size_t i = 0; i < test_input.size(); i++) { test_input[i] = (uint8_t) (i & 0x000000FF); }
	//for (size_t i = 0; i < test_input.size(); i++) { test_input[i] = 0; }
	//struct lz77_data *data = lz77_cpu(&test_input);
	//lz77_cuda(&test_input);
	options args;
	setup_args(argc, argv, &args);
	if (deflate(&args)) { return 0; }
	std::cerr << "Error compressing data." << std::endl;
	//for (size_t i = 0; i < data -> index -> size(); i++) { std::cout << "D: " << data -> distance -> at(i) << ", L: " << (unsigned int) data -> length -> at(i) << ", I: " << data -> index -> at(i) << std::endl; }
	return 1;
}
