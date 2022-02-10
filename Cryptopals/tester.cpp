#include<iostream>
using namespace std;

int main () {
    unsigned int i = 0x49276d20 ;
    unsigned int mask = 0xfc000000 ;
    i &= mask ;
    i >>= 26 ;
    cout << i ;
    return 0 ;
}
