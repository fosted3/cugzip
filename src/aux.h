#ifndef aux_h_
#define aux_h_

#include <vector>
#include <cstdint>
#include <cstdlib>

uint32_t crc32(uint32_t, const std::vector<uint8_t>*, size_t, size_t);

struct lz77_data
{
	std::vector<uint8_t>  *uncompressed_data;	//Unmodified data
	std::vector<bool>     *is_match;			//Where true, data can be replaced with a length-distance pair
	std::vector<bool>     *is_reference;		//Where true, data is reference to length-distance pair
	std::vector<uint8_t>  *length;				//Densely stored length data
	std::vector<uint16_t> *distance;			//Densely stored distance data
	std::vector<size_t>   *index;				//Densely stored index data
};

#endif
