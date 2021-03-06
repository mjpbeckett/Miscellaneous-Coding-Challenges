#include "encodings.h"
#include <iostream>
#include <string>
#include <bitset>
#include <stdexcept>
#include <vector>
#include <fstream>


// Single Character Conversions

char hex_to_char (char hexchar) {
    char rawchar = (hexchar & '@' ? hexchar + 9 : hexchar) & 0xF ;
    return rawchar ;
}

char char_to_hex (char rawchar, bool capital = false) {
    // converts rightmost four bits to a hex character
    const size_t a = capital ? 'A' : 'a' ;

    char hexchar = rawchar & 0xF ;
    if (hexchar < 10) {
        hexchar += '0' ;
    } else {
        hexchar += a - 10 ;
    }
    return hexchar ;
}

// Character values helpful for base64 encoding/decoding
const char b64_A = 0x00, b64_a = 0x1a, b64_0 = 0x34, b64_plus = 0x3e, b64_slash = 0x3f;
const char ascii_A = 0x41, ascii_a = 0x61, ascii_0 = 0x30, ascii_plus = 0x2b, ascii_slash = 0x2f;

char sixbit_to_b64 (char sixbit) {
    // converts rightmost six bits to b64 character
    sixbit &= 0x3f ;
    char outchar ;
    if (sixbit < b64_a) {
        outchar = sixbit + ascii_A ;
    } else if (sixbit < b64_0) {
        outchar = sixbit - b64_a + ascii_a ;
    } else if (sixbit < b64_plus) {
        outchar = sixbit - b64_0 + ascii_0 ;
    } else if (sixbit == b64_plus) {
        outchar = ascii_plus ;
    } else {
        outchar = ascii_slash ;
    }
    return outchar ;
}

char b64_to_sixbit (char b64char) {
    // converts b64 encoded character to sixbit representation, returned in rightmost six bits
    char sixbit ;
    if ((b64char >= ascii_A)&&(b64char < ascii_A + 26)) {
        sixbit = b64char - ascii_A + b64_A ;
    } else if ((b64char >= ascii_a)&&(b64char < ascii_a + 26)) {
        sixbit = b64char - ascii_a + b64_a ;
    } else if ((b64char >= ascii_0)&&(b64char < ascii_0 + 10)) {
        sixbit = b64char - ascii_0 + b64_0 ;
    } else if (b64char == ascii_plus) {
        sixbit = b64_plus ;
    } else if (b64char == ascii_slash) {
        sixbit = b64_slash ;
    } else {
        throw std::invalid_argument("Input is not encoded in base64.") ;
    }
    return sixbit ;
}


// String Conversions

void stream_to_b64 (std::string str, std::string& b64str, bool padding) {
    b64str.resize(((str.size() + 2) / 3) * 4) ; // b64str should be length of str / 3 (rounded up) multiplied by 4
    str.resize(str.size() + 1, 0x00) ; // pad str with 0s in case 6 does not divide number of bits in str

    size_t j = 0; // define it in this scope to be accessible subsequently for padding
    for (size_t byte_ind = 0, offset = 0; byte_ind < str.size() -1; byte_ind += (offset + 6)/8, offset = (offset + 6)%8)  {
        /* Iterate through blocks of six bits.
        byte_ind tracks index of current byte, offset number of additional bits
        to get to desired block of six */
        char sixbit = (str[byte_ind] << offset) | (str[byte_ind + 1] >> (8 - offset)); // find 8 bits starting at byte_ind*8 + offset
        sixbit >>= 2 ; // 6 bits of intrest are now rightmost
        b64str[j] = sixbit_to_b64(sixbit) ;
        j++ ;
    }

    if (padding) {
        while (j < b64str.size()) {
            b64str[j] = '=' ;
            j++ ;
        }
    } else {
        b64str.resize(j) ;
    }
}

void b64_to_stream (const std::string b64str, std::string& str) {

    str = "" ;

    char six_bits, bits_to_add, mask ;
    char remainder_bits = 0 ;using namespace std;
    size_t bit_offset = 0 ;

    for (size_t i = 0; (i < b64str.size())&&(b64str[i] != '='); i++) {
        six_bits = b64_to_sixbit( b64str[i] ) << 2 ;

        mask = 0xfc >> bit_offset ;
        // std::cout << "Remainder bits: " << std::bitset<8>(remainder_bits) ;
        // std::cout << ", Six bits: " << std::bitset<8>(six_bits) << "\n" ;
        bits_to_add = ((six_bits >> bit_offset)&mask)|remainder_bits ;
        if (bit_offset < 2) {
            remainder_bits = bits_to_add ;
        } else {
            // std::cout << "Added bits: " << std::bitset<8>(bits_to_add) << "\n" ;
            str.append(1, bits_to_add) ;
            remainder_bits = six_bits << ( 8-bit_offset ) ;
        }
        bit_offset = (bit_offset + 6)%8 ;
    }
}

void hex_to_stream (const std::string hexstr, std::string& str) {
    str.resize((hexstr.size() + 1) / 2) ;
    for (size_t i = 0, j = 0; i < str.size(); i++, j++) {
        str[i] = hex_to_char(hexstr[j]) << 4 ;
        j++ ;
        str[i] |= hex_to_char(hexstr[j]) ;
    }
}

void stream_to_hex (const std::string str, std::string& hexstr, bool capital) {
    hexstr.resize(str.size() * 2) ;

    for (size_t j = 0, i = 0; j < str.size()*2; j++, i++) {
        hexstr[j] = char_to_hex(str[i] >> 4) ;
        j++ ;
        hexstr[j] = char_to_hex(str[i]) ;
    }
}



// File Reading

void single_message_file_reader (const std::string filename, std::string& outstr,
                                 encoding in_encoding, encoding out_encoding) {
    /* Reads in file and sets outstr to contained message, concatenating all into single string.
    in_encoding and out_encoding set the encoding of input and output*/
    std::string inputstr = "" ;
    std::ifstream infile(filename) ;
    std::string line ;

    while (std::getline(infile, line)) {
        inputstr = inputstr.append(line) ;
    }
    infile.close() ;

    std::string interstr ;
    switch (in_encoding) {
        case hex:
            hex_to_stream(inputstr, interstr) ;
            break ;
        case b64:
            b64_to_stream(inputstr, interstr) ;
            break ;
        case raw:
            interstr = inputstr ;
    }
    switch (out_encoding) {
        case hex:
            stream_to_hex(interstr, outstr) ;
            break ;
        case b64:
            stream_to_b64(interstr, outstr) ;
            break ;
        case raw:
            outstr = interstr ;
    }
}


void multi_message_file_reader (const std::string filename, std::vector<std::string>& outvector,
                                encoding in_encoding, encoding out_encoding) {
    /* Reads in file and sets each line to an element of outvector.
    in_encoding and out_encoding set the encoding of input and output*/
    std::ifstream infile(filename) ;
    outvector.clear() ;

    std::string line, plainline, outline ;
    while (std::getline(infile, line)) {
        switch (in_encoding) {
            case hex:
                hex_to_stream(line, plainline) ;
                break ;
            case b64:
                b64_to_stream(line, plainline) ;
                break ;
            case raw:
                plainline = line ;
        }
        switch (out_encoding) {
            case hex:
                stream_to_hex(plainline, outline) ;
                break ;
            case b64:
                stream_to_b64(plainline, outline) ;
                break ;
            case raw:
                outline = plainline ;
        }
        outvector.push_back(outline) ;
    }
    infile.close() ;
}

void printsafe (const std::string str, std::string& safestr) {
    safestr = "" ;
    for (size_t i = 0; i < str.size(); i++) {
        safestr.append(1, (std::isprint(str[i])) ? str[i] : '_') ;
    }
}
