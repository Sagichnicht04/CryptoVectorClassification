#include <iostream>
#include <string>

// some global vars in vanitygen.hpp
static std::string foundAddress{};
static unsigned short fKeyId = 0;

static void inline CalculateW ()
{
    /*
    implementation of orignal
    */
    // from big endian to little endian ( swap )
    int dummy = 0;
}

static void inline TransformBlock ()
{
    /*
    implementation of orignal
    */
    // standard rounds and magic values
    // RNDr(S, W, 0, 0x428a2f98); RNDr(S, W, 1, 0x71374491);
}

void inline HashNextBlock ()
{
    /*
    implementation of orignal
    */
}

static bool check_prefix(const char * buf)
{
    return true;
}

static inline size_t ByteStreamToBase32 ()
{
    // last byte
    return 0;
}

static inline bool thread_find()
{
    /*
    Thanks to orignal ^-^
    For idea and example ^-^
    Orignal is sensei of crypto ;)
    */
    
    // precalculate first 5 blocks (320 bytes)
    // pre-calculate last W
    // our nonce is place in memory, where is b after 320 bytes (characters)
    // calculate hash of block with nonce
    // apply last block
    // get final hash
    // From there place we get a nonce, for some one a byte.
    // for another threads (?)
    return true;
}

void usage(void){
    // --threads -t (default count of system)
    // --multiplymode -m - multiple addresses search
}

void parsing(int argc, char ** args){
    // TODO: create libi2pd_tools
    // If file not exists we create a dump file. (a bug was found in issues)
    // TODO: for other types.
}

int main (int argc, char * argv[])
{
    // if argc size more than 2. nameprogram is 1. and 2 is prefix. if not there is will be flags like regex
    // TODO: ?
    
    // https://github.com/PurpleI2P/i2pd/blob/ae5239de435e1dcdff342961af9b506f60a494d4/libi2pd/Crypto.h#L310
    // init and terminate
    // By default false
    
    // if threads less than 0, then we get from system count of CPUs cores
    
    // Isntead proccess flipper?
    // if ( options . outputpath . empty () ); options . outputpath . assign ( DEF_OUT_FILE ) ;
    
    // there we gen key to buffer. That we mem allocate...
    // keys_len is will be constant. so calculate every time is a bad way
    
    // Start vanity generator in threads
    // there we start to change byte in our private key. we can change another bytes too 
    // but we just change 1 byte in all key. So. TODO: change all bytes not one?
    
    // our buf is our key, but in uint8 type, unsigned integ... another argument
    // is our prefix that we search in address
    // and j is magic number, is thread id. 
    // thoughtput is our magic number that we increment on 1000 everytime
    // so we just change a one a byte in key and convert private key to address
    // after we check it.
    
    // There will be proccessFlipper by accetone
    // if I correctly understand it's drop a payload things in a prefix/search data
    // or simmilar. We can just use regex. I would to use regex
    
    // before we write result we would to create private.dat a file. dump file. we can use for it keygen
    // IDK. what for acetone change this line to if (options.output...empty() ... assign
    // cplusplus.com/reference/string/string/assign yes we can. but I would don't change this
    
    // there we generate a key, like as in keygen.cpp
    // before a mining we would to create a dump file
    
    // void doSearch lamda
    
    // TODO: an another variable for file count and found keys as found keys by one runs
    
    return 0;
}