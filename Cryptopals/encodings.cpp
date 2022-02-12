#include <iostream>
#include <string>


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
    std::string teststr = "Hello World!" ;
    std::string testhex ;
    stream_to_hex(teststr, testhex) ;
    std::cout << testhex << std::endl ;

    return 0 ;
}
