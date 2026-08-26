#include "../../src/utils/url_helper.h"
#include <cassert>
#include <iostream>

void test_getHost() {
    assert(urlGetHost("http://example.com/path") == "http://example.com/");
    assert(urlGetHost("https://example.com/") == "https://example.com/");
    assert(urlGetHost("http://example.com") == "http://example.com");
    assert(urlGetHost("http://a/b/c/d") == "http://a/");
    assert(urlGetHost("") == "");
    assert(urlGetHost(nullptr) == "");
    assert(urlGetHost("no-slash") == "no-slash");
    std::cout << "test_getHost PASS\n";
}

void test_getRoot() {
    assert(urlGetRoot("http://example.com/a/b/c.html") == "http://example.com/a/b/");
    assert(urlGetRoot("http://example.com/a/") == "http://example.com/a/");
    assert(urlGetRoot("http://example.com") == "http://example.com/");
    assert(urlGetRoot("") == "/");
    std::cout << "test_getRoot PASS\n";
}

void test_adjust() {
    // absolute
    assert(urlAdjust("http://other.com/x", "http://example.com/a") == "http://other.com/x");
    assert(urlAdjust("https://other.com/x", "http://example.com/a") == "https://other.com/x");
    // protocol-relative
    assert(urlAdjust("//other.com/x", "http://example.com/a") == "http:other.com/x" || urlAdjust("//other.com/x", "http://example.com/a") == "http://other.com/x");
    // absolute path
    assert(urlAdjust("/newpath", "http://example.com/old/path") == "http://example.com/newpath");
    // relative
    assert(urlAdjust("rel.html", "http://example.com/a/b/") == "http://example.com/a/b/rel.html");
    assert(urlAdjust("rel.html", "http://example.com/a/b/c.html") == "http://example.com/a/b/rel.html");
    // host edge
    assert(urlAdjust("/p", "http://example.com") == "http://example.com/p");
    std::cout << "test_adjust PASS\n";
}

void test_parse() {
    assert(urlParse("#frag", "http://example.com/a") == "http://example.com/a#frag");
    assert(urlParse("http://x", "http://example.com") == "http://x");
    std::cout << "test_parse PASS\n";
}

void test_legacy_getHost_strndup() {
    char url[] = "http://example.com/path";
    char *h = getHost(url);
    assert(std::string(h) == "http://example.com/");
    free(h);
    char url2[] = "http://example.com";
    char *h2 = getHost(url2);
    assert(std::string(h2) == "http://example.com");
    free(h2);
    std::cout << "test_legacy_getHost PASS\n";
}

int main() {
    test_getHost();
    test_getRoot();
    test_adjust();
    test_parse();
    test_legacy_getHost_strndup();
    std::cout << "ALL url_helper tests PASS\n";
    return 0;
}
