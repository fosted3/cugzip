#ifndef aux_h_
#define aux_h_

#include <vector>
#include <cstdint>
#include <cstdlib>
#include <string>

uint32_t crc32(uint32_t, const std::vector<uint8_t>*, size_t, size_t);

struct lz77_data
{
	std::vector<uint8_t>  *uncompressed_data;	//Unmodified data
	std::vector<bool>     *is_match;			//Where true, data can be replaced with a length-distance pair
	std::vector<uint8_t>  *length;				//Densely stored length data
	std::vector<uint16_t> *distance;			//Densely stored distance data
	std::vector<size_t>   *index;				//Densely stored index data
};

struct huffman_table
{
	uint32_t codes[288];
	uint8_t length[288];
};

struct options
{
	std::string infile;
	std::string outfile;
	bool keep;
	bool verbose;
	bool gpu;
};

struct gzip_file
{
	uint8_t id1;
	uint8_t id2;
	uint8_t cm;
	uint8_t flg;
	uint32_t mtime;
	uint8_t xfl;
	uint8_t os;
	uint16_t xlen;
	uint8_t *extra;
	char *fname;
	char *fcomment;
	uint16_t crc16;
	uint8_t *blocks;
	uint32_t crc32;
	uint32_t isize;
};

struct blocks
{
	uint8_t *data;
	size_t osize;
	uint32_t crc32;
	uint32_t isize;
};

#define FTEXT		0x01
#define FHCRC		0x02
#define FEXTRA		0x04
#define FNAME		0x08
#define FCOMMENT	0x10

#endif
