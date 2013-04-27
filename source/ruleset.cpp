#include "ruleset.h"

#include <string.h>
#include <stdlib.h>

void apply_ruleset(string *html, char *url)
{
    int substr_b, substr_e;
    int span, pos;

    if(!strncmp(url, "http://coolrom.com/dlpop.php", 28))
    {
        if((substr_b = html->find("<font size=\"3\">")) == string::npos)
            return;

        substr_e = html->find("</form>") + 7;
        pos = html->find("id=\"dl\"") + 8;
        span = substr_e - substr_b;

        html->insert(pos, *html, substr_b, span);
    }
}
