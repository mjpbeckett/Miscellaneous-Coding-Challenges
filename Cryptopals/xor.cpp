#include <iostream>
#include <string>
#include <map>
#include <bitset>
#include <algorithm>
#include <fstream>
#include <vector>
#include <ctype.h>

void hex_to_stream (const std::string hexstr, std::string& str) ; // defined in encodings.cpp

void stream_to_hex (const std::string str, std::string& hexstr, bool capital = false) ; // defined in encodings.cpp

void b64_to_stream (const std::string b64str, std::string& str) ; // defined in encodings.cpp

void stream_xor(const std::string& in_str1, const std::string& in_str2, std::string& out_str) {
    out_str.resize( in_str1.size() ) ;
    for (size_t i = 0; i < in_str1.size(); i++) {
        out_str[i] = in_str1[i]^in_str2[i] ;
    }
}

void single_char_xor(const char cipher, const std::string& in_str, std::string& out_str) {
    out_str.resize(in_str.size()) ;
    for (size_t i = 0; i < in_str.size(); i++) {
        out_str[i] = in_str[i]^cipher ;
    }
}

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

float squaresum(float* in_array, size_t array_size) {
    float total = 0 ;
    for (size_t i = 0; i < array_size; i ++) {
        total += in_array[i]*in_array[i] ;
    }
    return total ;
}

void get_frequencies(const std::string& teststr, int* frequencies) {
    // records counts of each letter in teststr. frequencies should be array of size (at least) 27.
    // last element of frequencies is number of non-letter chars
    int non_letter = 0 ;
    for (size_t i = 0; i < 26; i++) {
        frequencies[i] = 0 ; //Initialize array
    }
    for (size_t i = 0; i < teststr.size(); i++) {
        if ( (teststr[i] >= 'A') && (teststr[i] <= 'Z') ) {
            frequencies[(teststr[i]&0x1f) - 1] += 1 ;
        } else if ((teststr[i] >= 'a') && (teststr[i] <= 'z')) {
            frequencies[(teststr[i]&0x1f) - 1] += 1 ;
        } else {
            frequencies[26] += 1 ;
        }
    }
}

float eval_string(const std::string& str) {
    // returns square difference between letter frequencies in str and expected frequencies for English
    int frequencies[27] ;
    float rel_freq_difference[27] ;
    get_frequencies(str, frequencies) ;
    float strlength = str.size() ;
    for (size_t i = 0; i < 26; i++) {
        rel_freq_difference[i] = (frequencies[i] / strlength) - letter_frequencies['A' + i] ;
    }
    rel_freq_difference[26] = frequencies[26]/strlength ;
    float eval = squaresum(rel_freq_difference, 27);
    return eval ;
}

struct xor_eval_summary {
    char cipher = 0x00 ;
    float eval = 2 ; // actual evals will always be less than 2
    const std::string* message_ptr = NULL ; // Note points to untransformed message
} ;

xor_eval_summary evaluate_xor (const char cipher, const std::string* message_ptr) {
    // returns evaluation of decryption of message with cipher
    xor_eval_summary summary ;
    summary.cipher = cipher ;
    summary.message_ptr = message_ptr ;

    std::string decryption ; // apply xor and evaluate
    single_char_xor(cipher, *message_ptr, decryption) ;
    summary.eval = eval_string(decryption) ;

    return summary ;
}

bool better_than (xor_eval_summary eval1, xor_eval_summary eval2) {
    return eval1.eval < eval2.eval ;
}

void insert_eval (xor_eval_summary* top_list, xor_eval_summary* bottom_ptr, xor_eval_summary new_entry) {
    // Inserts new entry into place in top_list, which is ordered list of best evaluations.
    // bottom_ptr should be last actual element of list, although the array should have
    // one extra space allocated for simplicity of moving elements down
    xor_eval_summary* cur_ptr = bottom_ptr ;
    xor_eval_summary* lower_ptr = bottom_ptr + 1 ;
    while ( better_than(new_entry, *cur_ptr) && (cur_ptr >= top_list) ) {
        // move all entries that are beaten by new_entry down one
        *lower_ptr = *cur_ptr ;
        cur_ptr-- ;
        lower_ptr-- ;
    }
    *lower_ptr = new_entry ;
}

void single_byte_xor_tester (const std::string* message_ptr, xor_eval_summary* top_list, xor_eval_summary* bottom_ptr) {
    // Inserts best evaluation summaries into positions in top_list. Array at top_list should have
    // one space beyond bottom_ptr allocated
    char cipher = 0x00 ;
    xor_eval_summary eval ;
    do {
        eval = evaluate_xor (cipher, message_ptr) ;
        insert_eval (top_list, bottom_ptr, eval) ;
        cipher ++ ;
    } while (cipher != char(0xFF)) ;
}


size_t read_in_messages(std::istream& message_file, std::vector<std::string>& message_list) {
    // reads in lines from message_file, populates message_list, and returns number of messages
    size_t num_messages = 0 ;
    std::string hex_line ;
    std::string raw_line ;
    while (std::getline(message_file, hex_line)) {
        hex_to_stream(hex_line, raw_line) ;
        message_list.push_back(raw_line) ;
        num_messages++ ;
    }
    return num_messages ;
}

void detect_xor_decryption(std::istream& message_file, std::vector<std::string>& message_list, xor_eval_summary* top_list, xor_eval_summary* bottom_ptr) {
    // populates list of most likely single-character XORs of lines from message_file
    size_t num_messages = read_in_messages(message_file, message_list) ;
    for (size_t i = 0; i < num_messages; i++) {
        single_byte_xor_tester(&(message_list[i]), top_list, bottom_ptr) ;
    }
}


void user_check_decryptions (xor_eval_summary* top_list, xor_eval_summary* bottom_ptr) {
    std::string decryption ;
    bool notfound = true ;
    char response ;
    std::string hexstr ;
    xor_eval_summary* cur_ptr = top_list ;
    xor_eval_summary cur_eval = *cur_ptr ;
    while (notfound) {
        std::cout << "Char: " ;
        std::cout << cur_eval.cipher << std::endl ;
        std::cout << "Evaluation: " ;
        std::cout << cur_eval.eval << std::endl ;
        std::cout << "\n" ;
        stream_to_hex(*(cur_eval.message_ptr), hexstr) ;
        std::cout << "Message before: " << hexstr << std::endl ;
        single_char_xor(cur_eval.cipher, *(cur_eval.message_ptr), decryption) ;
        std::cout << "Message after: " << decryption << std::endl ;
        std::cout << "\n" ;
        std::cout << "Is this a valid decryption? [y/n]" ;
        std::cin >> response ;
        notfound = (response == 'n') ;
        std::cout << "\n" ;
        if (cur_ptr >= bottom_ptr) {
            std::cout << "Sorry, valid decryption not found." ;
            break ;
        } else {
            cur_ptr++ ;
            cur_eval = *cur_ptr ;
        }
    }
}

void repeating_key_XOR(const std::string key, const std::string plaintext, std::string& encrypted) {
    // XORs against key, repeating as necessary until entire message encrypted
    size_t message_length = plaintext.size() ;
    size_t key_length = key.size() ;
    encrypted.resize( message_length ) ;
    for (size_t i = 0; i < message_length; i++) {
        encrypted[i] = plaintext[i] ^ key[i%key_length] ;
    }
}

int count_bits(const char ch) {
    int total = 0 ;
    int intch = int(ch) ;
    for (int i = 0; i < 8; i++) {
        total += intch&1 ;
        intch >>= 1 ;
    }
    return total ;
}

int hamming_distance(const std::string str1, const std::string str2) {
    // Computes Hamming distance between two strings. Strings must be the same size
    int total = 0 ;
    for (size_t i = 0; i < str1.size(); i++) {
        total += count_bits( str1[i]^str2[i] ) ;
    }
    return total ;
}

size_t guess_key_size(const std::string message_str) {
    size_t cur_guess ;
    float best_dist = 32 ; // Note: we are looking for best distance over four blocks, normalized by blocklength, which must be at most 32
    float cur_dist ;
    for ( size_t i = 2; i < 40; i++ ) {
        cur_dist = 0 ;
        for ( size_t j = 1; j < 5; j++ ) {
            cur_dist += float( hamming_distance( message_str.substr( 20, i ),
                                                 message_str.substr( 20+i*j, i))) ;
        }
        cur_dist /= i ;
        std::cout << "Best so far: " << best_dist << ", Current: " << cur_dist << ", Length guess: " << cur_guess << "\n";
        if (cur_dist < best_dist) {
            cur_guess = i ;
            best_dist = cur_dist ;
        }SXQgaXMgYSB0cnV0aCB1bml2ZXJzYWxseSBhY2tub3dsZWRnZWQsIHRoYXQgYSBzaW5nbGUgbWFuIGluIHBvc3Nlc3Npb24gb2YgYSBnb29kIGZvcnR1bmUsIG11c3QgYmUgaW4gd2FudCBvZiBhIHdpZmUu
    }
    return cur_guess ;
}

void printsafe (const std::string str, std::string& safestr) {
    safestr = "" ;
    for (size_t i = 0; i < str.size(); i++) {
        safestr.append(1, (std::isprint(str[i])) ? str[i] : '_') ;
    }
}

void decrypt_repeating_key(const std::string encrypted,
                           std::string& decrypted,
                           std::string& key) {
    // Finds length of repeating key, then evaluates most likely key
    size_t keylength = guess_key_size(encrypted) ;
    int num_rows = encrypted.size()/keylength ;
    int last_row = encrypted.size()%keylength ;
    std::cout<< keylength << '\n';

    // Create array of evaluations, and initialize key_guess_array with most likely guess
    auto top_eval_array = new xor_eval_summary[keylength][21] ;
    xor_eval_summary** key_guess_ptrs = new xor_eval_summary*[keylength] ;
    char* key_guess_chars = new char[keylength] ;
    std::string ith_column ;
    std::string hex_start ;
    for (size_t i = 0; i < keylength; i++) {
        ith_column = "" ;
        for (size_t j = i; j < encrypted.size(); j += keylength) {
            ith_column.append(1, encrypted[j]) ;
        }
        std::cout << top_eval_array[i] << '\n';
        std::cout << top_eval_array[i] + 19 << '\n';
        single_byte_xor_tester(&ith_column, &(top_eval_array[i][0]), &(top_eval_array[i][19])) ;
        key_guess_ptrs[i] = top_eval_array[i] ;
        key_guess_chars[i] = (*(key_guess_ptrs[i])).cipher ;
    }

    // Ask for user input to check keys
    bool notfound = true ;
    std::string command ;
    int char_to_change ;
    std::string safeprint ;
    while (notfound) {
        key = "";
        for (size_t i = 0; i < keylength; i++) {
            key.append(1, key_guess_chars[i]) ;
            std::cout << (*(key_guess_ptrs[i])).eval << "\n" ;
        }
        repeating_key_XOR(key, encrypted, decrypted) ;
        std::cout << "Current decryption:\n------------------\n" ;
        for (int row = 0; row < num_rows; row++) {
            printsafe(decrypted.substr(row*keylength,keylength), safeprint) ;
            std::cout << safeprint << ((row%4 == 0) ? "\n" : " | ") ;
        }
        std::cout << decrypted.substr(num_rows*keylength, last_row) << "\n\n" ;
        std::cout << "Enter y if this is correct, or position of key character to change and u/d to shift up the likelihood list: " ;
        std::cin >> command ;
        if (command[0] == 'y') {
            notfound = false;
        } else {
            char_to_change = int(command[0]) ;
            if (command[1]=='u') {
                if (key_guess_ptrs[char_to_change] > top_eval_array[char_to_change]) {
                    key_guess_ptrs[char_to_change]-- ;
                } else std::cout << "This character can not be changed up.\n" ;
            } else {
                if (key_guess_ptrs[char_to_change] < top_eval_array[char_to_change] + 19) {
                    key_guess_ptrs[char_to_change]++ ;
                } else std::cout << "This character can not be changed down.\n" ;
            }
            key_guess_chars[char_to_change] = (*(key_guess_ptrs[char_to_change])).cipher ;
        }
    }
    delete top_eval_array ;
    delete key_guess_ptrs ;
    delete key_guess_chars ;
}

void multiline_file_to_string(std::string filename, std::string& outstr) {
    //reads multiline file and saves to outstr
    outstr = "" ;
    std::ifstream infile(filename) ;
    std::string line ;
    while (std::getline(infile, line)) {
        outstr.append(line) ;
    }
}

int main() {

    // Challenge 3
    // std::cout << "Enter hex message to decrypt: " ;
    // std::string hexmessage, rawmessage ;
    // std::cin >> hexmessage ;
    // hex_to_stream(hexmessage, rawmessage) ;
    // xor_eval_summary best_decrypts[21] ;
    // single_byte_xor_tester( rawmessage, best_decrypts, best_decrypts + 20) ;
    // user_check_decryptions( best_decrypts, best_decrypts + 20) ;

    // Challenge 4
    // std::string filename ;
    // std::cout << "Enter filename: " ;
    // std::cin >> filename ;
    // std::ifstream messages(filename) ;

    // std::vector<std::string> message_list ;
    // xor_eval_summary best_decrypts[21] ;
    // detect_xor_decryption( messages, message_list, best_decrypts, best_decrypts + 20 ) ;
    // user_check_decryptions( best_decrypts, best_decrypts + 20 ) ;

    //Challenge 5
    // std::string message = "Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal" ;
    // std::string key = "ICE" ;
    //
    // std::string encrypted, hexout ;
    // repeating_key_XOR(key, message, encrypted) ;
    // stream_to_hex(encrypted, hexout) ;
    // std::cout << hexout ;

    //Challenge 6
    std::string filename = "Set1Challenge6.txt" ;
    std::string b64str, plainstr, decrypted, key ;
    multiline_file_to_string(filename, b64str) ;
    b64_to_stream(b64str, plainstr) ;
    std::string hexstr ;
    stream_to_hex(plainstr.substr(0,20), hexstr) ;
    std::cout << hexstr << "\n" ;
    decrypt_repeating_key(plainstr, decrypted, key) ;

    return 0 ;
}
