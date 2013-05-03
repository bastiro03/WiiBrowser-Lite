#include "ruleset.h"
#include "menu.h"
#include "common.h"
#include "config.h"

#include <string.h>
#include <stdlib.h>

void apply_ruleset(string *html, char *url)
{
    int substr_b, substr_e;
    int span, pos, tagend;
    int ins, last;

    string ext, tok;
    struct block frame;
    const char *urls;

    /* javascript document.write */
#ifdef DOCWRITE
    for(substr_b = 0; (substr_b = html->find("<script", substr_b)) != string::npos;)
    {
        tagend = html->find("</script>", substr_b) + 9;
        span = tagend - substr_b;

        string temp;
        ext = html->substr(substr_b, span);

        for(substr_e = 0; (substr_e = ext.find("document.write(", substr_e)) != string::npos;)
        {
            substr_e += 15;
            pos = ext.find("\");", substr_e) + 1;
            span = pos - substr_e;
            tok = ext.substr(substr_e, span);

            for(ins = 0; (ins = tok.find("\"", ins)) != string::npos; ins++)
            {
                ins += 1;
                last = tok.find("\"", ins);
                temp.append(tok.substr(ins, last - ins));
                ins = last;
            }
        }
        span = tagend - substr_b;
        html->replace(substr_b, span, temp);
    }

    for(substr_b = 0; (substr_b = html->find("\\n", substr_b)) != string::npos;)
        html->erase(substr_b, 2);

    /* iframe html tag */
#elif defined IFRAME
    for(substr_b = 0; (substr_b = html->find("<iframe", substr_b)) != string::npos;)
    {
        substr_e = html->find("src=", substr_b) + 5;
        tagend = html->find("</iframe>", substr_e) + 9;
        pos = html->find(html->at(substr_e-1), substr_e);

        span = pos - substr_e;
        urls = html->substr(substr_e, span).c_str();

        if(urls && urls[0])
        {
            ext = adjustUrl(urls, url);
            frame = downloadfile(curl_handle, ext.c_str(), NULL);

            if(frame.size>0)
            {
                html->replace(substr_b, tagend-substr_b, frame.data);
                continue;
            }
        }
        substr_b++;
    }
#endif

    /* javascript coolrom.com */
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
