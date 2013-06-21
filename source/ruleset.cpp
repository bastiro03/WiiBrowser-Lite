#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "config.h"

#include "menu.h"
#include "main.h"
#include "ruleset.h"

extern "C"
{
    #include <lua/lua.h>
    #include <lua/lauxlib.h>
    #include <lua/lualib.h>
}

/* call a function `f' defined in Lua */
const char *func (lua_State *L, const char *url, const char *html)
{
    const char *ret = NULL;

    /* push functions and arguments */
    lua_getglobal(L, "main");  /* function to be called */
    lua_pushstring(L, url);   /* push 1st argument */
    lua_pushstring(L, html);   /* push 2nd argument */

    /* do the call (2 arguments, 1 result) */
    if (lua_pcall(L, 2, 1, 0) != 0)
        printf("%s", lua_tostring(L, -1));

    /* retrieve result */
    if (lua_isstring(L, -1))
        ret = lua_tostring(L, -1);

    lua_pop(L, 1);  /* pop returned value */
    return ret;
}

bool executeLua(string *html, char *url)
{
    /*
     * All Lua contexts are held in this structure. We work with it almost
     * all the time.
     */
    lua_State *L = lua_open();

    /* Load Lua libraries */
    luaL_openlibs(L);

    /* Load the file containing the script we are going to run */
    int status = luaL_loadfile(L, "scripts/main.lua");
    if (status)
    {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        return false;
    }

    /* Initial run through of file */
    if(lua_pcall(L,0,0,0)) {
		printf("\n\nFailed running file initially - Could be a syntax error\n");
		return false;
	}

	const char *ret = func(L, url, html->c_str());
	if(ret)
        *html = ret;

	/* Cya, Lua */
    lua_close(L);
    return true;
}

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
    if(Settings.DocWrite)
    {
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
    }
#endif

    /* iframe html tag */
#ifdef IFRAME
    if(Settings.IFrame)
    {
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
    }
#endif

    if(Settings.ExecLua)
        executeLua(html, url);
}

/*
    // javascript coolrom.com
    if(!strncmp(url, "http://coolrom.com/dlpop.php", 28))
    {
        if((substr_b = html->find("<font size=\"3\">")) == string::npos)
            return;

        substr_e = html->find("</form>") + 7;
        pos = html->find("id=\"dl\"") + 8;
        span = substr_e - substr_b;

        html->insert(pos, *html, substr_b, span);
    }
*/
