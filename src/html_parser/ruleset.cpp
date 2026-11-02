#include "ruleset.h"

// First-release policy: parse documents as documents. The inherited local Lua,
// document.write emulation and iterative iframe injection are not executed,
// including when an older settings file enables them.
void apply_ruleset(std::string *html, char *url)
{
    (void)html;
    (void)url;
}
