#ifndef ENCODINGS_H
#define ENCODINGS_H
#include <string>
#include <vector>

enum encoding {raw, hex, b64} ;

void stream_to_b64 (std::string str, std::string& b64str, bool padding=true) ;
void b64_to_stream (const std::string b64str, std::string& str) ;
void hex_to_stream (const std::string hexstr, std::string& str) ;
void stream_to_hex (const std::string str, std::string& hexstr, bool capital=false) ;

void single_message_file_reader (const std::string filename, std::string& outstr, encoding in_encoding=raw, encoding out_encoding=raw) ;
void multi_message_file_reader (const std::string filename, std::vector<std::string>& outvector, encoding in_encoding=raw, encoding out_encoding=raw) ;

void printsafe (const std::string str, std::string& safestr) ;

#endif
