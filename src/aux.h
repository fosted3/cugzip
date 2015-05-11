#ifndef aux_h_
#define aux_h_

#include <vector>
#include <cstdint>
#include <cstdlib>
#include <string>

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
	bool force_static;
	bool force_dynamic;
	bool thread;
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
	std::vector<uint8_t> *data;
	uint32_t crc32;
	uint32_t isize;
	bool final;
};

uint32_t crc32(uint32_t, const std::vector<uint8_t>*, size_t, size_t);
bool file_exists(std::string);
void setup_args(int, char**, options*);
uint32_t get_three(const std::vector<uint8_t>*, size_t);
void bit_pack(uint8_t*, uint32_t, uint8_t, size_t*, uint8_t*);

#define FTEXT		0x01
#define FHCRC		0x02
#define FEXTRA		0x04
#define FNAME		0x08
#define FCOMMENT	0x10

#endif
