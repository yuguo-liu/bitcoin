#ifndef BITCOIN_UTIL_FEATHERCHAIN_CORE_H
#define BITCOIN_UTIL_FEATHERCHAIN_CORE_H

#include <chain.h>
#include <utility>
#include <string>
#include <vector>
#include <primitives/block.h>

long long **getOptionalPointersToInt_64Array(int height, CBlockIndex* pindexPrev);
/**
 * func: getOptionalPointersToInt_64Array
 * param:
 *      height - int: the height of current block
 *      pindexPrev - CBlock*: the CBlock of the previous block of current block
 * return:
 *      long long**: the 2 form of optional ptrs in int_64 array
 * */

bool verifyCoinbase(int height, CBlockIndex* pindexPrev, CBlock* block);
/**
 * func: verifyCoinbase
 * param:
 *      height - int: the height of current block
 *      pindexPrev - CBlock*: the CBlock of the previous block of current block
 *      coinbaseTx - CMutableTransaction: the coinbase content of current block
 * return:
 *      bool: the correction of coinbase (true/false)
 * */

std::vector<unsigned char> *getOptionalPointersToVector(int height, CBlockIndex* pindexPrev);

std::vector<unsigned char> getOptionalPointersProof(int height, CBlockIndex* pindexPrev);
bool verifyCoinbaseSkipList(int height, CBlockIndex* pindexPrev, CBlock* block);
void testForSearchBlock(CBlockIndex* pindexPrev, int times);
#endif //BITCOIN_UTIL_FEATHERCHAIN_CORE_H