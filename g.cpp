#include <iostream>
#include <string>
#include <cstring>

int main() {
    char str1[] = "Hello";
    char str2[] = "Hello";
    if(!std::strcmp(str1, str2)) {
        std::cout << "Strings are equal.\n";
    }
    else {
        std::cout << "Strings are not equal.\n";
    }
    return 0;
}
