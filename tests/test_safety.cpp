#include "safety.h"
#include "file_io.h"
#include "url_helper.h"
#include "ParserDom.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    char buffer[5] = "old";
    assert(wbl::copyString(buffer, sizeof(buffer), "1234"));
    assert(!wbl::copyString(buffer, sizeof(buffer), "12345"));
    assert(!strcmp(buffer, "1234"));
    assert(!wbl::appendString(buffer, sizeof(buffer), "x"));
    assert(wbl::safeFilename("100% complete.png"));
    for (const char *name : {"", ".", "..", "../boot.dol", "a/b", "a\\b", "sd:foo", "bad.", "bad "})
        assert(!wbl::safeFilename(name));
    assert(wbl::storagePath("sd:/apps/wiibrowser/"));
    assert(wbl::storagePath("usb:/download/file.zip"));
    for (const char *path : {"/sys", "sd:/../boot.dol", "usb:/a//b", "sdbad:/a", "sd:/a\\b", "sd:/a/./b"})
        assert(!wbl::storagePath(path));
    assert(wbl::httpUrl("https://example.test/a?x=1#fragment"));
    for (const char *url : {"file:///etc/passwd", "http://", "http:///a", "https://u:p@host/", "http://h/\\multipart", "http://h/\n"})
        assert(!wbl::httpUrl(url));
    int number = 7;
    assert(wbl::parseInteger("8", 0, 8, number) && number == 8);
    for (const char *value : {"-1", "9", "1x", "99999999999999999999999999", ""})
        assert(!wbl::parseInteger(value, 0, 8, number));
    size_t bytes = 0;
    assert(wbl::imageSize(640, 480, 4, bytes) && bytes == 1228800);
    assert(!wbl::imageSize(0, 1, 4, bytes));
    assert(!wbl::imageSize(4096, 4096, 4, bytes));
    assert(wbl::markupBudget("<html><body><img src='x'><br>text</body></html>"));
    assert(!wbl::markupBudget(std::string(4096, '<')));
    std::string nested;
    for (int i = 0; i < 65; ++i) nested += "<div>";
    assert(!wbl::markupBudget(nested));
    assert(!wbl::markupBudget("<p title=\"unfinished>"));
    assert(urlAdjust("//cdn.test/a", "https://example.test/b") == "https://cdn.test/a");
    assert(urlAdjust("../c?q=1", "https://example.test/a/b") == "https://example.test/c?q=1");
    assert(urlAdjust("?new", "https://example.test/a?old#x") == "https://example.test/a?new");
    assert(urlAdjust("#new", "https://example.test/a#old") == "https://example.test/a#new");
    assert(urlAdjust("javascript:bad", "https://example.test/").empty());

    htmlcxx::HTML::Node node;
    node.text("<img>"); node.isTag(true); node.parseAttributes();
    assert(!node.attribute("src").first);
    for (const char *markup : {"<html><body>x</body></html>", "<script>x<", "<script>x</script", "<div a=\"\">x</div>"}) {
        htmlcxx::HTML::ParserDom parser;
        parser.parseTree(markup);
    }

    const char *temporaryRoot = getenv("TMPDIR");
    if (!temporaryRoot) temporaryRoot = "/tmp/opencode";
    mkdir(temporaryRoot, 0700);
    std::string pattern = std::string(temporaryRoot) + "/wiibrowser-test-XXXXXX";
    assert(mkdtemp(&pattern[0]));
    const std::string path = pattern + "/settings";
    std::string read;
    assert(wbl::writeRecoverable(path.c_str(), "old", 3));
    assert(wbl::writeRecoverable(path.c_str(), "new", 3));
    assert(wbl::readBounded(path.c_str(), read, 3) && read == "new");
    assert(wbl::readBounded((path + ".bak").c_str(), read, 3) && read == "old");
    assert(!wbl::readBounded(path.c_str(), read, 2));
    const std::string obstructed = pattern + "/obstructed";
    assert(wbl::writeRecoverable(obstructed.c_str(), "old", 3));
    assert(mkdir((obstructed + ".bak").c_str(), 0700) == 0);
    assert(!wbl::writeRecoverable(obstructed.c_str(), "new", 3));
    assert(wbl::readBounded(obstructed.c_str(), read, 3) && read == "old");
    assert(rmdir((obstructed + ".bak").c_str()) == 0);
    remove((obstructed + ".new").c_str()); remove(obstructed.c_str());
    assert(wbl::writeUserFile((path + ".new").c_str(), "pending", 7, false));
    assert(!wbl::writeRecoverable(path.c_str(), "oops", 4));
    assert(wbl::readBounded(path.c_str(), read, 3) && read == "new");
    const std::string user = pattern + "/user";
    assert(wbl::writeUserFile(user.c_str(), "first", 5, false));
    assert(!wbl::writeUserFile(user.c_str(), "second", 6, false));
    assert(wbl::readBounded(user.c_str(), read, 10) && read == "first");
    for (const std::string &name : {user + ".new", user, path + ".new", path + ".bak", path}) remove(name.c_str());
    assert(rmdir(pattern.c_str()) == 0);
    puts("Portable bounds, URL, parser-boundary and recovery tests passed.");
}
