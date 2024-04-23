#include <util/espresso_core.h>
#include <util/system.h>
#include <string>
#include <vector>
#include <time.h>

int *getOptionalPointer(int height) {
    int ptr_1_decrement = 1, cp_nHeight = height;
    int *optionalPtrs = new int[2];
    optionalPtrs[0] = -1, optionalPtrs[1] = -1;
    while (cp_nHeight > 0) {
        if (cp_nHeight % 2 == 1) {
            optionalPtrs[0] = ptr_1_decrement > 1 ? height - ptr_1_decrement : -1;
            break;
        } else {
            cp_nHeight >>= 1, ptr_1_decrement <<= 1;
        }
    }
    int ptr_2_decrement = ptr_1_decrement * (ptr_1_decrement - 1); // 2^n * (2^n - 1)
    if ((height & ptr_2_decrement) == ptr_2_decrement && ptr_1_decrement > 1) optionalPtrs[1] = height - ptr_2_decrement;
    return optionalPtrs;
}

void pushHashValueToVector(std::string str, std::vector<unsigned char> *optionalPointers) {
    for (int i = 0; i < 5; i++) {
        long long hvContents = i != 4 ? strtoll(str.substr(14 * i, 14).c_str(), NULL, 16): strtoll(str.substr(56).c_str(), NULL, 16);
        int counter = 0, limit = i != 4 ? 7 : 4;
        std::vector<unsigned char> tmp;
        while (hvContents) tmp.push_back(hvContents & 0xff), hvContents >>= 8, counter++;
        while (counter < limit) tmp.push_back(0 & 0xff), counter++;
        while (!tmp.empty()) {
            optionalPointers->push_back(tmp.back());
            tmp.pop_back();
        }
    }
}

std::vector<unsigned char> *getOptionalPointersToVector(int height, CBlockIndex* pindexPrev) {
    std::vector<unsigned char> *optionalPointers;
    optionalPointers = new std::vector<unsigned char>[2];
    int *optionalPtrs = getOptionalPointer(height);
    if (optionalPtrs[0] != -1) {
        std::string ptr_1 = pindexPrev->GetAncestor(optionalPtrs[0])->GetBlockHash().ToString();
        pushHashValueToVector(ptr_1, &optionalPointers[0]);
    }
    if (optionalPtrs[1] != -1) {
        std::string ptr_2 = pindexPrev->GetAncestor(optionalPtrs[1])->GetBlockHash().ToString();
        pushHashValueToVector(ptr_2, &optionalPointers[1]);
    }
    LogPrintf("ok in get optional ptr\n");
    return optionalPointers;
}

std::string stringSplit(const std::string& strIn, std::string delim, int get) {
    int i = 0, length = delim.size();
    for (; i < strIn.size(); i++) if (strIn.substr(i, length) == delim) break;
    return get == 0 ? strIn.substr(0, i) : strIn.substr(i + length);
}

bool verifyCoinbase(int height, CBlockIndex* pindexPrev, CBlock* block) {
    if (height == 1) return true;
    int *optionalPtrs = getOptionalPointer(height);
    LogPrintf("%s\n", block->ToString());
    std::string ptr_1_hash = "", ptr_2_hash = "", coinbaseTx = stringSplit(stringSplit(block->ToString(), "coinbase ", 1), ")", 0);
    LogPrintf("%s\n", coinbaseTx);
    int sub_content_length = 0, pointer = 0;
    for (int i = 0; i < coinbaseTx.length();) {
        if (pointer == 0) sub_content_length = stoi(coinbaseTx.substr(i, 2), NULL, 16), i += (sub_content_length > 8) ? 4 : sub_content_length * 2 + 2, pointer = 1;
        else {
            sub_content_length = stoi(coinbaseTx.substr(i, 2), NULL, 16);
            if (sub_content_length == 0 || pointer > 2) break;
            if (pointer == 1) ptr_1_hash = coinbaseTx.substr(i + 2, sub_content_length * 2), i += sub_content_length * 2 + 2, pointer = 2;
            if (pointer == 2) ptr_2_hash = coinbaseTx.substr(i + 2, sub_content_length * 2), i += sub_content_length * 2 + 2, pointer = 3;
        }
    }
    bool ret = true;
    if (ptr_2_hash != "") {
        std::string hv_2_str = pindexPrev->GetAncestor(optionalPtrs[1])->GetBlockHash().ToString();
        ret = ret & (hv_2_str == ptr_2_hash);
    }
    if (ptr_1_hash != "") {
        std::string hv_1_str = pindexPrev->GetAncestor(optionalPtrs[0])->GetBlockHash().ToString();
        ret = ret & (hv_1_str == ptr_1_hash);
    }
    return ret;
}

//    int pointer = 0, sub_content_length, status = 0;
//    for (int i = 0; i < coinbaseTx.length();) {
//        if (pointer == 0) sub_content_length = stoi(coinbaseTx.substr(i, 2), NULL, 16), i += (sub_content_length > 8) ? 4 : sub_content_length * 2 + 4, pointer = 1;
//        else {
//            std::string str = "", s_str;
//            sub_content_length = stoi(coinbaseTx.substr(i, 2), NULL, 16);   // get the length of sub content
//            if (sub_content_length > 8) str += ("000" + std::to_string(sub_content_length - 80)), sub_content_length = 0; // avoid the key word OP_1 to OP_16 (in this case, the subcontentlength is the content)
//            if (sub_content_length == 0) str += (status == 0 ? "0000000000000000" : "0000"), sub_content_length = 0; // avoid value 0
//            else {
//                std::string sub_coinbase_Tx = coinbaseTx.substr(i + 2, sub_content_length * 2);
//                if (sub_coinbase_Tx.substr(sub_coinbase_Tx.length() - 2, 2) == "00" && status > 0) sub_coinbase_Tx = sub_coinbase_Tx.substr(0, sub_coinbase_Tx.length()-2);
//                while (status == 0 && sub_coinbase_Tx.length() < 16) sub_coinbase_Tx = sub_coinbase_Tx + "00";
//                int length = sub_coinbase_Tx.length();
//                for (int j = 0; j < length; j += 2)
//                    str += (status == 0 && j == 0 && sub_coinbase_Tx.substr(length - j - 2, 1) == "0") ? sub_coinbase_Tx.substr(length - j - 1, 1) : sub_coinbase_Tx.substr(length - j - 2, 2);
//            }
//            if (pointer == 1) {
//                ptr_1_hash += str;
//                if (ptr_1_hash.length() == 64) pointer = 2;
//                i += sub_content_length * 2 + 4, status = stoi(coinbaseTx.substr(i - 2,2));
//            } else {
//                ptr_2_hash += str;
//                i += sub_content_length * 2 + 4, status = stoi(coinbaseTx.substr(i - 2,2));
//            }
//        }
//    }
//    bool ret = true;
//    if (ptr_2_hash != "") {
//        std::string hv_2_str = pindexPrev->GetAncestor(optionalPtrs[1])->GetBlockHash().ToString();
//        ret = ret & (hv_2_str == ptr_2_hash);
//    }
//    if (ptr_1_hash != "") {
//        std::string hv_1_str = pindexPrev->GetAncestor(optionalPtrs[0])->GetBlockHash().ToString();
//        ret = ret & (hv_1_str == ptr_1_hash);
//    }
//    return ret;

//long long **getOptionalPointersToInt_64Array(int height, CBlockIndex* pindexPrev) {
//    long long **optionalPointers;
//    optionalPointers = new long long* [2];
//    int *optionalPtrs = getOptionalPointer(height);
//    if (optionalPtrs[0] != -1) {
//        *optionalPointers = new long long [5];
//        std::string hv_1_str = pindexPrev->GetAncestor(optionalPtrs[0])->GetBlockHash().ToString();
//        for (int i = 0; i < 4; i++) optionalPointers[0][i] = strtoll(hv_1_str.substr(15 * i, 15).c_str(), NULL, 16);
//        optionalPointers[0][4] = strtoll(hv_1_str.substr(60).c_str(), NULL, 16);
//    } else *optionalPointers = nullptr;
//    if (optionalPtrs[1] != -1) {
//        *(optionalPointers + 1) = new long long [5];
//        std::string hv_2_str = pindexPrev->GetAncestor(optionalPtrs[1])->GetBlockHash().ToString();
//        for (int i = 0; i < 4; i++) optionalPointers[1][i] = strtoll(hv_2_str.substr(15 * i, 15).c_str(), NULL, 16);
//        optionalPointers[1][4] = strtoll(hv_2_str.substr(60).c_str(), NULL, 16);
//    } else *(optionalPointers + 1) = nullptr;
//    LogPrintf("ok in get optional ptr\n");
//    return optionalPointers;
//}

#include <consensus/merkle.h>
#include <uint256.h>

std::vector<int> getSkipPointers(int height) {
    std::vector<int> pointers;
    int cp_height = height, counter = 4;
    while(cp_height >= 1) {
        if (cp_height % 4 == 0) {
            pointers.push_back(height - counter);
            cp_height = (int) cp_height / 4;
            counter *= 4;
        } else break;
    }
    return pointers;
}

std::string hashPointersStr(std::vector<std::string> hashStrs) {
    std::vector<uint256> hashes;
    std::vector<unsigned char> rootHashValue;
    for (std::string h : hashStrs) {
        hashes.push_back(uint256S(h));
    }
    uint256 roothash = ComputeMerkleRoot(hashes);
    return roothash.ToString();
}

std::vector<unsigned char> hashPointers(std::vector<std::string> hashStrs) {
    std::vector<unsigned char> rootHashValue;
    std::string roothashStr = hashPointersStr(hashStrs);
    pushHashValueToVector(roothashStr, &rootHashValue);
    return rootHashValue;
}

std::string getOptionalPointersProofStr(int height, CBlockIndex* pindexPrev) {
    std::string optionalPointers;
    std::vector<int> optionalPtrs = getSkipPointers(height);
    std::vector<std::string> hashStrs;
    if (!optionalPtrs.empty()) {
        for(int i : optionalPtrs) {
            hashStrs.push_back(pindexPrev->GetAncestor(i)->GetBlockHash().ToString());
        }
        optionalPointers = hashPointersStr(hashStrs);
    }
    LogPrintf("ok in get optional ptr\n");
    return optionalPointers;
}

std::vector<unsigned char> getOptionalPointersProof(int height, CBlockIndex* pindexPrev) {
    std::vector<unsigned char> optionalPointers;
    std::vector<int> optionalPtrs = getSkipPointers(height);
    std::vector<std::string> hashStrs;
    if (!optionalPtrs.empty()) {
        for(int i : optionalPtrs) {
            hashStrs.push_back(pindexPrev->GetAncestor(i)->GetBlockHash().ToString());
        }
        optionalPointers = hashPointers(hashStrs);
    }
    LogPrintf("ok in get optional ptr\n");
    return optionalPointers;
}

bool verifyCoinbaseSkipList(int height, CBlockIndex* pindexPrev, CBlock* block) {
    if (height == 1) return true;
    std::vector<int> optionalPtrs = getSkipPointers(height);
    LogPrintf("%s\n", block->ToString());
    std::vector<std::string> hashes;
    std::string roothash = "", coinbaseTx = stringSplit(stringSplit(block->ToString(), "coinbase ", 1), ")", 0);
    LogPrintf("%s\n", coinbaseTx);
    int sub_content_length = 0, pointer = 0;
    for (int i = 0; i < coinbaseTx.length();) {
        if (pointer == 0) sub_content_length = stoi(coinbaseTx.substr(i, 2), NULL, 16), i += (sub_content_length > 8) ? 4 : sub_content_length * 2 + 2, pointer = 1;
        else {
            sub_content_length = stoi(coinbaseTx.substr(i, 2), NULL, 16);
            if (sub_content_length == 0 || pointer > 1) break;
            if (pointer == 1) roothash = coinbaseTx.substr(i + 2, sub_content_length * 2), i += sub_content_length * 2 + 2, pointer = 2;
        }
    }
    if (roothash != "") {
        std::string vRoothashStr = getOptionalPointersProofStr(height, pindexPrev);
        LogPrintf("%s\n", roothash);
        if (vRoothashStr == roothash) return true;
        else return false;
    }
    return true;
}

void testForSearchBlock(CBlockIndex* pindexPrev, int times) {
    clock_t start_t, end_t;
    start_t = clock();
    for (int i=0; i<times; i++) {
        pindexPrev->GetAncestor(i);
    }
    end_t = clock();
    double duration = (double) (end_t - start_t) / CLOCKS_PER_SEC;
    LogPrintf("duration: %f of %d rounds", duration, times);
}