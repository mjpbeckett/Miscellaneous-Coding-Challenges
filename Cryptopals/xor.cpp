#include <iostream>
#include <string>
#include <map>
#include <bitset>
#include <algorithm>
#include <fstream>
#include <vector>

void hex_to_stream (const std::string hexstr, std::string& str) ; // defined in encodings.cpp

void stream_to_hex (const std::string str, std::string& hexstr, bool capital = false) ; // defined in encodings.cpp

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
    std::string filename ;
    std::cout << "Enter filename: " ;
    std::cin >> filename ;
    std::ifstream messages(filename) ;

    std::vector<std::string> message_list ;
    xor_eval_summary best_decrypts[21] ;
    detect_xor_decryption( messages, message_list, best_decrypts, best_decrypts + 20 ) ;
    user_check_decryptions( best_decrypts, best_decrypts + 20 ) ;

    return 0 ;
}
