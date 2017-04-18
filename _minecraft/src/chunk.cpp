#include "chunk.h"

float NYChunk::_WorldVert[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6];
float NYChunk:: _WorldCols[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*4*4*6];
float  NYChunk::_WorldNorm[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6];
float  NYChunk::_WorldUV[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 2 * 4 * 6];

float NYChunk::_WorldVertT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float NYChunk::_WorldColsT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 4 * 4 * 6];
float  NYChunk::_WorldNormT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
float  NYChunk::_WorldUVT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 2 * 4 * 6];

	