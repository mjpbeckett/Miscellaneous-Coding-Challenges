#include "xor.h"
#include "encodings.h"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <map>
#include <string>
#include <vector>


std::map<char, float> letter_frequencies {
    {'A', 0.082},
    {'B', 0.015},
    {'C', 0.028},
    {'D', 0.043},
    {'E', 0.13},
    {'F', 0.022},
    {'G', 0.02},
    {'H', 0.061},
    {'I', 0.07},
    {'J', 0.002},
    {'K', 0.008},
    {'L', 0.04},
    {'M', 0.025},
    {'N', 0.067},
    {'O', 0.075},
    {'P', 0.019},
    {'Q', 0.001},
    {'R', 0.06},
    {'S', 0.063},
    {'T', 0.091},
    {'U', 0.028},
    {'V', 0.01},
    {'W', 0.024},
    {'X', 0.002},
    {'Y', 0.02},
    {'Z', 0.001},
} ;


void stream_xor (const std::string& in_str1, const std::string& in_str2, std::string& out_str) {
    out_str.resize( in_str1.size() ) ;
    for (size_t i = 0; i < in_str1.size(); i++) {
        out_str[i] = in_str1[i]^in_str2[i] ;
    }
}

void single_char_xor (const char cipher, const std::string& in_str, std::string& out_str) {
    out_str.resize(in_str.size()) ;
    for (size_t i = 0; i < in_str.size(); i++) {
        out_str[i] = in_str[i]^cipher ;
    }
}

void repeating_key_XOR (const std::string key, const std::string plaintext, std::string& transformed) {
    // XORs against key, repeating as necessary until entire message encrypted
    size_t message_length = plaintext.size() ;
    size_t key_length = key.size() ;
    transformed.resize( message_length ) ;
    for (size_t i = 0; i < message_length; i++) {
        transformed[i] = plaintext[i] ^ key[i%key_length] ;
    }
}

float squaresum (const std::array<float,27> in_array) {
    float total = 0 ;
    for (float i : in_array) {
        total += i*i ;
    }
    return total ;
}

void get_frequencies (const std::string& teststr, std::array<int, 27>& frequencies) {
    // records counts of each letter in teststr. frequencies should be array of size (at least) 27.
    // last element of frequencies is number of non-letter chars
    for (size_t i = 0; i < 27; i++) {
        frequencies[i] = 0 ; //Initialize array
    }
    for (size_t i = 0; i < teststr.length(); i++) {
        if ( (teststr[i] >= 'A') && (teststr[i] <= 'Z') ) {
            frequencies[(teststr[i]&0x1f) - 1] += 1 ;
        } else if ((teststr[i] >= 'a') && (teststr[i] <= 'z')) {
            frequencies[(teststr[i]&0x1f) - 1] += 1 ;
        } else {
            frequencies[26] += 1 ;
        }
    }
}

float eval_string (const std::string& str) {
    // returns square difference between letter frequencies in str and expected frequencies for English
    std::array<int, 27> frequencies ;
    std::array<float, 27> rel_freq_difference ;
    get_frequencies(str, frequencies) ;
    float strlength = str.size() ;
    for (size_t i = 0; i < 26; i++) {
        rel_freq_difference[i] = (frequencies[i] / strlength) - letter_frequencies['A' + i] ;
    }
    rel_freq_difference[26] = frequencies[26] / strlength ;
    float eval = squaresum(rel_freq_difference) ;
    return eval ;
}

xor_eval_summary evaluate_xor (const char cipher, const std::string& message) {
    // returns evaluation of decryption of message with cipher
    xor_eval_summary summary ;
    summary.cipher = cipher ;
    summary.message_ptr = &message ;

    std::string decryption ; // apply xor and evaluate
    single_char_xor(cipher, message, decryption) ;
    summary.eval = eval_string(decryption) ;

    return summary ;
}

bool better_than (xor_eval_summary eval1, xor_eval_summary eval2) {
    return eval1.eval < eval2.eval ;
}

void insert_eval (std::array<xor_eval_summary, TOP_LIST_SIZE>& top_list, xor_eval_summary new_entry) {
    // Inserts new entry into place in top_list, which is ordered list of best evaluations.
    xor_eval_summary* cur_ptr = top_list.end() - 1 ;
    if ( better_than(new_entry, *cur_ptr) ) {
        cur_ptr-- ;
        while ( better_than(new_entry, *cur_ptr) && (cur_ptr >= top_list.begin()) ) {
        // move all entries that are beaten by new_entry down one
            *(cur_ptr + 1) = *(cur_ptr) ;
            cur_ptr-- ;
        }
    }
    if (cur_ptr < top_list.end() - 1) {
        *(cur_ptr + 1) = new_entry ;
    }
}

int count_bits (const char ch) {
    int total = 0 ;
    int intch = int(ch) ;
    for (int i = 0; i < 8; i++) {
        total += intch&1 ;
        intch >>= 1 ;
    }
    return total ;
}

int hamming_distance (const std::string str1, const std::string str2) {
    // Computes Hamming distance between two strings. Strings must be the same size
    int total = 0 ;
    for (size_t i = 0; i < str1.size(); i++) {
        total += count_bits( str1[i]^str2[i] ) ;
    }
    return total ;
}

void print_xor_eval (const xor_eval_summary eval) {
    // prints xor eval
    std::cout << "Cipher: " << std::bitset<8>(eval.cipher) << "\n" ;
    std::cout << "Evaluation: " << eval.eval << "\n" ;
    std::string decrypted, printstr ;
    single_char_xor(eval.cipher, *(eval.message_ptr), decrypted) ;
    printsafe (decrypted, printstr) ;
    std::cout << printstr << "\n" ;
}

void single_byte_xor_tester (const std::string message, std::array<xor_eval_summary, TOP_LIST_SIZE>& top_list) {
    // Inserts best evaluation summaries into positions in top_list
    char cipher = 0x00 ;
    xor_eval_summary eval ;
    do {
        eval = evaluate_xor (cipher, message) ;
        insert_eval (top_list, eval) ;
        cipher ++ ;
    } while (cipher != char(0xFF)) ;
}

void detect_xor_decryption (const std::vector<std::string> message_list, std::array<xor_eval_summary, TOP_LIST_SIZE>& top_list) {
    // populates list of most likely single-character XORs messages from message_list
    for (std::string message : message_list) {
        single_byte_xor_tester(message, top_list) ;
    }
}

size_t guess_key_size(const std::string message_str) {
    size_t cur_guess ;
    float best_dist = 48 ; // Note: we are looking for best distance over four blocks, normalized by blocklength, which must be at most 32
    float cur_dist ;
    std::vector<std::string> blocks(4) ;
    for ( size_t length = 2; length < 40; length++ ) {
        cur_dist = 0 ;
        for ( size_t i = 0; i < 4; i++ ) {
            blocks[i] = message_str.substr(length*i, length) ;
        }
        for ( size_t j = 0; j < 3; j++ ) {
            for ( size_t k = j+1; k < 4; k++) {
                cur_dist += hamming_distance(blocks[j],blocks[k]) ;
            }
        }
        cur_dist /= length ;

        if (cur_dist < best_dist) {
            cur_guess = length ;
            best_dist = cur_dist ;
        }
    }

    return cur_guess ;
}

void decrypt_repeating_key (const std::string encrypted,
                            std::string& decrypted,
                            std::string& key) {
    // Finds length of repeating key, then evaluates most likely key
    size_t keylength = guess_key_size (encrypted) ;
    //int num_rows = encrypted.size()/keylength ;
    //int last_row = encrypted.size()%keylength ;

    // Create array of evaluations, and initialize key_guess_array with most likely guess
    std::vector<std::array<xor_eval_summary, TOP_LIST_SIZE>> top_eval_array(keylength) ;
    // xor_eval_summary** key_guess_ptrs = new xor_eval_summary*[keylength] ;
    std::string key_guess ;
    std::vector<std::string> message_blocks(keylength) ;
    size_t columns_ind = 0 ;
    size_t block_length ;
    for (size_t block_ind = 0; block_ind < keylength; block_ind ++) {
        block_length = 0 ;
        for (size_t char_ind = block_ind; char_ind < encrypted.length(); char_ind += keylength) {
            message_blocks[block_ind].push_back(encrypted[char_ind]) ;
            block_length ++ ;
        }
    }

    columns_ind = 0 ;
    for (const auto& block : message_blocks) {
        single_byte_xor_tester(block, top_eval_array[columns_ind]) ;
        key_guess.push_back( top_eval_array[columns_ind][0].cipher ) ;
        columns_ind++ ;
    }

    repeating_key_XOR( key_guess, encrypted, decrypted ) ;
    std::string printstr ;
    printsafe(decrypted, printstr) ;
    std::cout << "Decrypted: " << printstr << "\n" ;
    std::cout << "Key: " << key_guess << "\n" ;
}


int main() {
    // std::string ciphertext, decryptedtext ;
    // std::array<xor_eval_summary, TOP_LIST_SIZE> top_list ;
    // hex_to_stream("1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736", ciphertext) ;
    // single_byte_xor_tester (ciphertext, top_list) ;
    // single_char_xor(top_list[0].cipher, ciphertext, decryptedtext) ;
    // std::cout << decryptedtext << "\n";

    // SINGLE BYTE WORKING NOW, WHAT'S WRONG WITH REPEATING KEY?
    std::string ciphertext, decryptedtext, key ;
    single_message_file_reader ("Set1Challenge6.txt", ciphertext, b64) ;
    decrypt_repeating_key (ciphertext, decryptedtext, key) ;

    return 0 ;
}
