#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "config.h"

#include "menu.h"
#include "main.h"
#include "ruleset.h"

string javascript_write(string ext, string replace, string end)
{
	int substr_e;
	int span, pos;
	int ins, last;

	string tok, temp;

	for (substr_e = 0; (substr_e = ext.find(replace, substr_e)) != string::npos;)
	{
		substr_e += replace.length();

		char delim = ext.at(substr_e);
		if (delim != '"' && delim != '\'')
			continue;

		pos = ext.find(ext.at(substr_e) + end, substr_e) + 1;
		span = pos - substr_e;
		tok = ext.substr(substr_e, span);

		for (ins = 0; (ins = tok.find(delim, ins)) != string::npos; ins++)
		{
			ins += 1;
			last = tok.find(delim, ins);
			temp.append(tok.substr(ins, last - ins));
			ins = last;
		}
	}

	return temp;
}

void apply_ruleset(string *html, char *url)
{
	int substr_b, substr_e;
	int span, pos, tagend;

	string ext, tok;
	struct block frame;
	const char *urls;

	/* mediafire downloads */
	if (!strncmp(url, "http://www.mediafire.com/download/", 34))
	{
		substr_b = html->find("kNO = ", 0) + 6;
		char delim = html->at(substr_b++);

		if (delim == '"' || delim == '\'')
		{
			substr_e = html->find(delim, substr_b);
			span = substr_e - substr_b;
			ext = html->substr(substr_b, span);

			tok.assign("<b><a href=\"");
			tok.append(ext);
			tok.append("\">DOWNLOAD FILE</a></b>");

			substr_b = html->find("<div class=\"dl-btn-label\">", 0) + 26;
			html->insert(substr_b, tok);
		}
	}

	/* javascript document.write */
#ifdef DOCWRITE
	if (Settings.DocWrite)
	{
		for (substr_b = 0; (substr_b = html->find("<script", substr_b)) != string::npos;)
		{
			tagend = html->find("</script>", substr_b) + 9;
			span = tagend - substr_b;

			string temp;
			ext = html->substr(substr_b, span);

			temp.append(javascript_write(ext, "document.write(", ")"));
			temp.append(javascript_write(ext, "document.writeln(", ")"));
			temp.append(javascript_write(ext, "innerHTML=", ";"));

			span = tagend - substr_b;
			html->replace(substr_b, span, temp);
		}

		for (substr_b = 0; (substr_b = html->find("\\n", substr_b)) != string::npos;)
			html->erase(substr_b, 2);
	}
#endif

	/* iframe html tag */
#ifdef IFRAME
	if (Settings.IFrame)
	{
		for (substr_b = 0; (substr_b = html->find("<iframe", substr_b)) != string::npos;)
		{
			substr_e = html->find("src=", substr_b) + 5;
			tagend = html->find("</iframe>", substr_e) + 9;
			pos = html->find(html->at(substr_e - 1), substr_e);

			span = pos - substr_e;
			urls = html->substr(substr_e, span).c_str();

			if (urls && urls[0])
			{
				ext = adjustUrl(urls, url);
				frame = downloadfile(curl_handle, ext.c_str(), NULL);

				if (frame.size > 0)
				{
					html->replace(substr_b, tagend - substr_b, frame.data);
					continue;
				}
			}
			substr_b++;
		}
	}
#endif
}
