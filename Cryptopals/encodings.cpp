#include <iostream>
#include <string>


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
    sixbit &= 0x3f;
    char outchar;
    if (sixbit < b64_a) {
        outchar = sixbit + ascii_A;
    } else if (sixbit < b64_0) {
        outchar = sixbit - b64_a + ascii_a;
    } else if (sixbit < b64_plus) {
        outchar = sixbit - b64_0 + ascii_0;
    } else if (sixbit == b64_plus) {
        outchar = ascii_plus;
    } else {
        outchar = ascii_slash;
    }
    return outchar;
}


// String Conversions

void stream_to_b64 (std::string& str, std::string& b64str, bool padding = true) {
    b64str.resize(((str.size() + 2) / 3) * 4) ; // b64str should be length of str / 3 (rounded up) multiplied by 4
    str.resize(str.size() + 1, 0x00) ; // pad str with 0s in case 6 does not divide number of bits in str

    size_t j = 0; // define it in this scope to be accessible subsequently for padding
    for (size_t byte_ind = 0, offset = 0; byte_ind < str.size() -1; byte_ind += (offset + 6)/8, offset = (offset + 6)%8)  {
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

    str.resize(str.size() -1) ;
}

void hex_to_stream (const std::string hexstr, std::string& str) {
    str.resize((hexstr.size() + 1) / 2) ;
    for (size_t i = 0, j = 0; i < str.size(); i++, j++) {
        str[i] = hex_to_char(hexstr[j]) << 4 ;
        j++ ;
        str[i] |= hex_to_char(hexstr[j]) ;
    }
}

void stream_to_hex (const std::string str, std::string& hexstr, bool capital = false) {
    hexstr.resize(str.size() * 2) ;

    for (size_t j = 0, i = 0; j < str.size()*2; j++, i++) {
        hexstr[j] = char_to_hex(str[i] >> 4) ;
        j++ ;
        hexstr[j] = char_to_hex(str[i]) ;
    }
}


int main() {

    return 0 ;
}
