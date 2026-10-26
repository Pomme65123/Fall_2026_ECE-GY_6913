/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size in bytes)  t=32-s-b
32 bit address (MSB -> LSB): TAG || SET || OFFSET

Tag Bits   : the tag field along with the valid bit is used to determine whether the block in the cache is valid or not.
Index Bits : the set index field is used to determine which set in the cache the block is stored in.
Offset Bits: the offset field is used to determine which byte in the block is being accessed.
*/

#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
// access state:
#define NA 0         // no action
#define RH 1         // read hit
#define RM 2         // read miss
#define WH 3         // Write hit
#define WM 4         // write miss
#define NOWRITEMEM 5 // no write to memory
#define WRITEMEM 6   // write to memory

struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
// You need to define your cache class here, or design your own data structure for L1 and L2 cache

/*
A single cache block:
    - valid bit (is the data in the block valid?)
    - dirty bit (has the data in the block been modified by means of a write?)
    - tag (the tag bits of the address)
    - data (the actual data stored in the block, in our case, we don't need to store the data)
*/
struct CacheBlock
{
    // we don't actually need to allocate space for data, because we only need to simulate the cache action
    // or else it would have looked something like this: vector<number of bytes> Data;

    bool valid{};
    bool dirty{};
    unsigned int tag{};
    unsigned int full_addr{};
};

/*
A CacheSet:
    - a vector of CacheBlocks
    - a counter to keep track of which block to evict next
*/
struct set
{
    // tips:
    // Associativity: eg. resize to 4-ways set associative cache
    vector<CacheBlock> blocks;
    int evict_counter{};
};

// You can design your own data structure for L1 and L2 cache; just an example here
class cache
{
    // some cache configuration parameters.
    // cache L1 or L2
public:
    int l1_num_set{};
    int l1_associativity{};
    int l1_index_bits{};
    int l1_offset_bits{};

    int l2_num_set{};
    int l2_associativity{};
    int l2_index_bits{};
    int l2_offset_bits{};

    vector<set> L1_sets;
    vector<set> L2_sets;
    cache(int l1_b_size, int l1_assoc, int l1_size_kib,
          int l2_b_size, int l2_assoc, int l2_size_kib)
    {
        // initialize the cache according to cache parameters
        int l1_total_bytes = l1_size_kib * 1024;

        if (l1_assoc == 0)
        {
            l1_num_set = 1;
            l1_associativity = l1_total_bytes / l1_b_size;
        }
        else
        {
            l1_associativity = l1_assoc;
            l1_num_set = l1_total_bytes / (l1_b_size * l1_associativity);
        }
        l1_offset_bits = (int)log2(l1_b_size);
        l1_index_bits = (int)log2(l1_num_set);

        L1_sets.resize(l1_num_set);

        for (int i{}; i < l1_num_set; i++)
        {
            L1_sets[i].blocks.resize(l1_associativity);
            for (int j{}; j < l1_associativity; j++)
            {
                L1_sets[i].blocks[j].valid = false;
                L1_sets[i].blocks[j].dirty = false;
                L1_sets[i].blocks[j].tag = 0;
                L1_sets[i].blocks[j].full_addr = 0;
            }
            L1_sets[i].evict_counter = 0;
        }
        int l2_total_bytes = l2_size_kib * 1024;
        if (l2_assoc == 0)
        {
            l2_num_set = 1;
            l2_associativity = l2_total_bytes / l2_b_size;
        }
        else
        {
            l2_associativity = l2_assoc;
            l2_num_set = l2_total_bytes / (l2_b_size * l2_associativity);
        }
        l2_offset_bits = (int)log2(l2_b_size);
        l2_index_bits = (int)log2(l2_num_set);
        L2_sets.resize(l2_num_set);
        for (int i{}; i < l2_num_set; i++)
        {
            L2_sets[i].blocks.resize(l2_associativity);
            for (int j{}; j < l2_associativity; j++)
            {
                L2_sets[i].blocks[j].valid = false;
                L2_sets[i].blocks[j].dirty = false;
                L2_sets[i].blocks[j].tag = 0;
                L2_sets[i].blocks[j].full_addr = 0;
            }
            L2_sets[i].evict_counter = 0;
        }
    }
    void get_tag_index_L1(unsigned int addr, unsigned int &tag, unsigned int &idx)
    {
        if (l1_num_set <= 1)
        {
            idx = 0;
        }
        else
        {
            idx = (addr >> l1_offset_bits) & ((1ULL << l1_index_bits) - 1);
        }
        int shift = l1_offset_bits + l1_index_bits;
        tag = (shift >= 32) ? 0 : (addr >> shift);
    }

    void get_tag_index_L2(unsigned int addr, unsigned int &tag, unsigned int &idx)
    {
        if (l2_num_set <= 1)
        {
            idx = 0;
        }
        else
        {
            idx = (addr >> l2_offset_bits) & ((1ULL << l2_index_bits) - 1);
        }
        int shift = l2_offset_bits + l2_index_bits;
        tag = (shift >= 32) ? 0 : (addr >> shift);
    }

    bool insert_L2(unsigned int addr, bool is_dirty)
    {
        unsigned int tag2{};
        unsigned int idx2{};
        get_tag_index_L2(addr, tag2, idx2);
        for (int i{}; i < l2_associativity; i++)
        {
            if (!L2_sets[idx2].blocks[i].valid)
            {
                L2_sets[idx2].blocks[i].valid = true;
                L2_sets[idx2].blocks[i].dirty = is_dirty;
                L2_sets[idx2].blocks[i].tag = tag2;
                L2_sets[idx2].blocks[i].full_addr = addr;
                return false;
            }
        }

        int evict_way = L2_sets[idx2].evict_counter;
        bool mem_write = L2_sets[idx2].blocks[evict_way].dirty;

        L2_sets[idx2].blocks[evict_way].valid = true;
        L2_sets[idx2].blocks[evict_way].dirty = is_dirty;
        L2_sets[idx2].blocks[evict_way].tag = tag2;
        L2_sets[idx2].blocks[evict_way].full_addr = addr;
        L2_sets[idx2].evict_counter = (evict_way + 1) % l2_associativity;
        return mem_write;
    }

    bool insert_L1(unsigned int addr, bool is_dirty)
    {
        unsigned int tag1{};
        unsigned int idx1{};
        get_tag_index_L1(addr, tag1, idx1);

        for (int i{}; i < l1_associativity; i++)
        {
            if (!L1_sets[idx1].blocks[i].valid)
            {
                L1_sets[idx1].blocks[i].valid = true;
                L1_sets[idx1].blocks[i].dirty = is_dirty;
                L1_sets[idx1].blocks[i].tag = tag1;
                L1_sets[idx1].blocks[i].full_addr = addr;
                return false;
            }
        }

        int evict_way = L1_sets[idx1].evict_counter;
        unsigned int victim_addr = L1_sets[idx1].blocks[evict_way].full_addr;
        bool victim_dirty = L1_sets[idx1].blocks[evict_way].dirty;

        L1_sets[idx1].blocks[evict_way].valid = true;
        L1_sets[idx1].blocks[evict_way].dirty = is_dirty;
        L1_sets[idx1].blocks[evict_way].tag = tag1;
        L1_sets[idx1].blocks[evict_way].full_addr = addr;
        L1_sets[idx1].evict_counter = (evict_way + 1) % l1_associativity;

        return insert_L2(victim_addr, victim_dirty);
    }

    auto write(auto addr)
    {
        /*
        step 1: select the set in our L1 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> WriteHit!
                                                    -> Dirty Bit High
        step 3: Otherwise? -> WriteMiss!

        return WH or WM
        */
        unsigned int u_addr = addr;
        unsigned int tag{};
        unsigned int idx{};
        get_tag_index_L1(u_addr, tag, idx);

        for (int i{}; i < l1_associativity; i++)
        {
            if (L1_sets[idx].blocks[i].valid && L1_sets[idx].blocks[i].tag == tag)
            {
                L1_sets[idx].blocks[i].dirty = true;
                return WH;
            }
        }
        return WM;
    }

    auto writeL2(auto addr)
    {
        /*
        step 1: select the set in our L2 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> WriteHit!
                                                 -> Dirty Bit High
        step 3: Otherwise? -> WriteMiss!

        return {WM or WH, WRITEMEM or NOWRITEMEM}
        */
        unsigned int u_addr = addr;
        unsigned int tag{};
        unsigned int idx{};
        get_tag_index_L2(u_addr, tag, idx);

        for (int i{}; i < l2_associativity; i++)
        {
            if (L2_sets[idx].blocks[i].valid && L2_sets[idx].blocks[i].tag == tag)
            {
                L2_sets[idx].blocks[i].dirty = true;
                return std::make_pair(WH, NOWRITEMEM);
            }
        }
        return std::make_pair(WM, WRITEMEM);
    }

    auto readL1(auto addr)
    {
        /*
        step 1: select the set in our L1 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> ReadHit!
        step 3: Otherwise? -> ReadMiss!

        return RH or RM
        */
        unsigned int u_addr = addr;
        unsigned int tag{};
        unsigned int idx{};

        get_tag_index_L1(u_addr, tag, idx);

        for (int i{}; i < l1_associativity; i++)
        {
            if (L1_sets[idx].blocks[i].valid && L1_sets[idx].blocks[i].tag == tag)
            {
                return RH;
            }
        }
        return RM;
    }

    auto readL2(auto addr)
    {
        /*
        step 1: select the set in our L2 cache using set index bits
        step 2: iterate through each way in the current set
            - If Matching tag and Valid Bit High -> ReadHit!
                                                 -> copy dirty bit
        step 3: otherwise? -> ReadMiss! -> need to pull data from Main Memory
        step 4: find a place in L1 for our requested data
            - case 1: empty way in L1 -> place requested data
            - case 2: no empty way in L1 -> evict from L1 to L2
                    - case 2.1: empty way in L2 -> place evicted L1 data there
                    - case 2.2: no empty way in L2 -> evict from L2 to memory

        return {RM or RH, WRITEMEM or NOWRITEMEM}
        */
        unsigned int u_addr = addr;
        unsigned int tag2{};
        unsigned int idx2{};
        get_tag_index_L2(u_addr, tag2, idx2);

        int l2_state{RM};
        bool is_dirty{};

        for (int i{}; i < l2_associativity; i++)
        {
            if (L2_sets[idx2].blocks[i].valid && L2_sets[idx2].blocks[i].tag == tag2)
            {
                l2_state = RH;
                is_dirty = L2_sets[idx2].blocks[i].dirty;
                L2_sets[idx2].blocks[i].valid = false;
                break;
            }
        }

        bool mem_write = insert_L1(u_addr, is_dirty);
        return std::make_pair(l2_state, mem_write ? WRITEMEM : NOWRITEMEM);
    }
};

// // Tips: another example:
// class CacheSystem
// {
//     Cache L1;
//     Cache L2;

// public:
//     int L1AcceState, L2AcceState, MemAcceState;
//     auto read(auto addr) {};
//     auto write(auto addr) {};
// };
// class Cache
// {
//     set *CacheSet;

// public:
//     auto read_access(auto addr) {};
//     auto write_access(auto addr) {};
//     auto check_exist(auto addr) {};
//     auto evict(auto addr) {};
// };
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

int main(int argc, char *argv[])
{

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof()) // read config file
    {
        cache_params >> dummyLine;               // L1:
        cache_params >> cacheconfig.L1blocksize; // L1 Block size
        cache_params >> cacheconfig.L1setsize;   // L1 Associativity
        cache_params >> cacheconfig.L1size;      // L1 Cache Size
        cache_params >> dummyLine;               // L2:
        cache_params >> cacheconfig.L2blocksize; // L2 Block size
        cache_params >> cacheconfig.L2setsize;   // L2 Associativity
        cache_params >> cacheconfig.L2size;      // L2 Cache Size
    }
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;

    /*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    if (cacheconfig.L1blocksize != cacheconfig.L2blocksize)
    {
        printf("please test with the same block size\n");
        return 1;
    }
    cache c1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size,
             cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);

    int L1AcceState = NA;          // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = NA;          // L2 access state variable, can be one of NA, RH, RM, WH, WM;
    int MemAcceState = NOWRITEMEM; // Main Memory access state variable, can be either NA or WH;

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr))
            {
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32>(addr);

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0) // a Read request
            {
                // Implement by you:
                //   read access to the L1 Cache,
                //   and then L2 (if required),
                //   update the access state variable;
                //   return: L1AcceState L2AcceState MemAcceState

                // For example:
                // L1AcceState = cache.readL1(addr); // read L1
                // if(L1AcceState == RM){
                //     L2AcceState, MemAcceState = cache.readL2(addr); // if L1 read miss, read L2
                // }
                // else{ ... }
                L1AcceState = c1.readL1(addr);
                if (L1AcceState == RM)
                {
                    auto result = c1.readL2(addr);
                    L2AcceState = result.first;
                    MemAcceState = result.second;
                }
                else
                {
                    L2AcceState = NA;
                    MemAcceState = NOWRITEMEM;
                }
            }
            else
            { // a Write request
              // Implement by you:
              //   write access to the L1 Cache, or L2 / main MEM,
              //   update the access state variable;
              //   return: L1AcceState L2AcceState

                // For example:
                // L1AcceState = cache.writeL1(addr);
                // if (L1AcceState == WM){
                //     L2AcceState, MemAcceState = cache.writeL2(addr);
                // }
                // else if(){...}
                L1AcceState = c1.write(addr);
                if (L1AcceState == WM)
                {
                    auto result = c1.writeL2(addr);
                    L2AcceState = result.first;
                    MemAcceState = result.second;
                }
                else
                {
                    L2AcceState = NA;
                    MemAcceState = NOWRITEMEM;
                }
            }
            /*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

            // Grading: don't change the code below.
            // We will print your access state of each cycle to see if your simulator gives the same result as ours.
            tracesout << L1AcceState << " " << L2AcceState << " " << MemAcceState << endl; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
