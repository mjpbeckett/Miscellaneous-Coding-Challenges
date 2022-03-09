#ifndef XOR_H
#define XOR_H
#include <array>
#include <string>
#include <vector>


#define TOP_LIST_SIZE 20

// XORING functions
void stream_xor (const std::string& in_str1, const std::string& in_str2, std::string& out_str) ;
void single_char_xor (const char cipher, const std::string& in_str, std::string& out_str) ;
void repeating_key_XOR (const std::string key, const std::string plaintext, std::string& transformed) ;

// Evaluation functions
struct xor_eval_summary {//Struct to hold evaluation of string
    char cipher = 0x00 ;
    float eval = 2 ; // actual evals will always be less than 2
    const std::string* message_ptr = NULL ; // Note points to untransformed message
} ;

void get_frequencies (const std::string& teststr, std::array<int, 27>& frequencies) ;
float eval_string (const std::string& str) ;
xor_eval_summary evaluate_xor (const char cipher, const std::string& message) ;
int hamming_distance (const std::string str1, const std::string str2) ;
void print_xor_eval (const xor_eval_summary eval) ;

// Detecting decryption
void single_byte_xor_tester (const std::string message, std::array<xor_eval_summary, TOP_LIST_SIZE>& top_list) ;
void detect_xor_decryption (const std::vector<std::string> messages, std::array<xor_eval_summary, TOP_LIST_SIZE>& top_list) ;
size_t guess_key_size (const std::string message_str) ;
void decrypt_repeating_key (const std::string encrypted, std::string& decrypted, std::string& key) ;

#endif
