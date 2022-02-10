#include <iostream>
#include <string>
#include <sstream>
using namespace std;

char single_byte_encoder (int in_byte) {
    int out_byte ;
    if (in_byte <= 0b011001) {
        out_byte = in_byte + 65 ;
    } else if (in_byte <= 0b110011) {
        out_byte = in_byte + 71 ;
    } else if (in_byte <= 0b111101) {
        out_byte = in_byte - 4 ;
    } else if (in_byte == 0b111110) {
        out_byte = 0x2b ;
    } else {
        out_byte = 0x21 ;
    }
    char out_char = char(out_byte) ;
    return out_char ;
}


int get_6bit_char ( unsigned int * int_array, int offset ) {
    // Returns the 6 bits starting at offset, assuming offset less than 32
    unsigned int mask = 0xfc000000 >> offset ;
    unsigned int out_int = (int_array[0] & mask) << offset ;
    if (offset >= 24) {
        mask = 0xfc000000 << (32 - offset) ;
        unsigned int extra_bits = (int_array[1] & mask) >> (32 - offset) ;
        out_int |= extra_bits ;
    }
    out_int >>= 26 ;
    cout << out_int << endl ;
    char out_char = single_byte_encoder(out_int) ;
    return out_char ;
}


void b64_encoder ( unsigned int int_array[], int arraysize, char* out_ptr, int encoded_size ) {
    // Returns pointer to an array of characters giving the base64 encoding of bytearray
    int i = 0 ;
    for ( int total_bits = 0; total_bits < 32*arraysize; total_bits += 6 ) {
        int offset = total_bits % 32 ;
        int byte_index = total_bits / 32 ;
        out_ptr[i] = get_6bit_char( int_array + byte_index, offset ) ;
        i++ ;
    }
    for ( int n = i+1; n < encoded_size; n++ ) {
        out_ptr[n] = '=';
    }
}

int main() {
    string instring ;
    cin >> instring ;
    int int_array_size = ((instring.size() - 1)/8 + 1) ;
    int array_size = ((instring.size() - 1)/2 + 1) ;
    int encoded_size = ((array_size - 1)/3 + 1)*4 ;
    unsigned int* int_array ;
    int_array = new unsigned int[int_array_size] ;
    for (int i = 0; i < int_array_size; i++) {
        string chunk = instring.substr(i*8, 8) ;
        stringstream(chunk) >> hex >> int_array[i] ;
    }

    char encoded_array [encoded_size] ;
    b64_encoder(int_array, int_array_size, encoded_array, encoded_size);
    cout << encoded_array ;

    return 0 ;
}
