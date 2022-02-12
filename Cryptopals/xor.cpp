#include <iostream>
#include <string>

void hex_to_stream (const std::string hexstr, std::string& str) ;

void stream_to_hex (const std::string str, std::string& hexstr, bool capital = false) ;

void stream_xor(const std::string& in_str1, const std::string& in_str2, std::string& out_str) {
    out_str.resize( in_str1.size() ) ;
    for (size_t i = 0; i < in_str1.size(); i++) {
        out_str[i] = in_str1[i]^in_str2[i] ;
    }
}

int main() {
    std::string hexstr1, hexstr2 ;
    do {
        std::cout << "Enter first hex-encoded string: " ;
        std::cin >> hexstr1 ;
        std::cout << "Enter second hex-encoded string: " ;
        std::cin >> hexstr2 ;
        if (hexstr1.size() != hexstr2.size()) {
            std::cout << "Strings must be same length./n" ;
        }
    } while (hexstr1.size() != hexstr2.size()) ;
    std::string rawstr1, rawstr2 ;
    hex_to_stream(hexstr1, rawstr1) ;
    hex_to_stream(hexstr2, rawstr2) ;

    std::string xorraw, xorhex ;
    stream_xor(rawstr1, rawstr2, xorraw) ;
    stream_to_hex(xorraw, xorhex) ;
    std::cout << "XOR'd string: " << xorhex ;

    return 0 ;
}
