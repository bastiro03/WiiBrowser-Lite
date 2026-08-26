#include <cassert>
#include <iostream>
#include <cstring>
#include <strings.h>

// Host-only copy of strtokcmp (from src/utils/stringop.cpp) — avoids Wii headers
int strtokcmp_host(const char *string, const char *compare, const char *separator) {
    if(!string || !compare) return -1;
    char TokCopy[512];
    strncpy(TokCopy, compare, sizeof(TokCopy));
    TokCopy[511] = '\0';
    char * strTok = strtok(TokCopy, separator);
    while (strTok != NULL) {
        if (strcasecmp(string, strTok) == 0) return 0;
        strTok = strtok(NULL,separator);
    }
    return -1;
}

void test_strtokcmp() {
    assert(strtokcmp_host("apple", "apple,banana,cherry", ",") == 0);
    assert(strtokcmp_host("banana", "apple,banana,cherry", ",") == 0);
    assert(strtokcmp_host("cherry", "apple,banana,cherry", ",") == 0);
    assert(strtokcmp_host("durian", "apple,banana,cherry", ",") == -1);
    assert(strtokcmp_host(nullptr, "a,b", ",") == -1);
    assert(strtokcmp_host("a", nullptr, ",") == -1);
    assert(strtokcmp_host("APPLE", "apple,banana", ",") == 0);
    std::cout << "test_strtokcmp PASS\n";
}

int main() {
    test_strtokcmp();
    std::cout << "ALL stringop tests PASS\n";
    return 0;
}
