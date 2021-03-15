#include "html/ParserDom.h"
#include "html/utils.h"
#include "html/wincstring.h"
#include "css/parser_pp.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <cstdio>

#include "liste.h"
#include "main.h"
#include "ruleset.h"

#define VERSION "0.6"

extern "C" {
#include "entities.h"
}

using namespace std;
using namespace htmlcxx;

enum {title,a,form,text,all};

void save_mem(string str)
{
    ofstream file;
    file.open ("debug.txt", ios::out | ios::app);
    file << str << "\r\n\r\n";
    file.close();
}

void save_mem_int(float str)
{
    ofstream file;
    file.open ("debug.txt", ios::out | ios::app);
    file << str << "\r\n\r\n";
    file.close();
}

bool equal(string s1, string s2)
{
    return (strcasecmp(s1.c_str(), s2.c_str()) == 0);
}

void merge(Tag *tag, vector<Value> out)
{
    tag->value.insert(tag->value.end(), out.begin(), out.end());
}

int checkparent(vector<string> tag, int start)
{
    unsigned int i, k;
    for (i=0; i<tag.size(); i++)
    {
        for (k=start; k<END; k++)
        {
            if (equal(tag[i], tags[k]))
            {
                return 1;
            }
        }
    }
    return 0;
}

int checkTag(vector<string> tag, string name)
{
    unsigned int i;
    for (i=0; i<tag.size(); i++)
    {
        if (equal(tag[i], name))
            return 1;
    }
    return 0;
}

int checkchr (string value)
{
    unsigned int i;
    for (i=0; i<value.length(); i++)
        if (value[i]>32)
            return 1;
    return 0;
}

int isContainer(string name)
{
    string array[] = {"h1","h2","h3","h4","h5","h6","p","div","br","center","hr","li","title","form","td"/*,"img"*/};
    vector<string> container (array, array + sizeof(array) / sizeof(string));
    return checkTag(container, name);
}

bool prevTagIs(tree<HTML::Node>::iterator it, string tag)
{
    bool ret = false;

    if(equal((--it)->tagName(), tag))
        ret = true;

    return ret;
}

string strToUrl(string attr)
{
    char *link = (char *)attr.c_str();
    string dest;

    int i = strcspn(link, "'\"");
    if(i >= (int)strlen(link))
        return dest;

    char ch = attr.at(i++);
    int span;

    if ((ch == '"' || ch == '\''))
    {
        span = attr.find(ch, i)-i;
        dest.assign(attr.substr(i, span));
    }

    return dest;
}

string createLink(tree<HTML::Node>::iterator it)
{
    string href(it->attribute("href").second);
    string onclick(it->attribute("onclick").second);
    string ret;

    if(href.length() && href[0] != '#')
    {
        if(strncasecmp("javascript", href.c_str(), 10))
            return href;

        if((ret = strToUrl(href)).length())
            return ret;
    }

    if(strcasestr(onclick.c_str(), "location"))
    {
        ret = strToUrl(onclick);
    }

    return ret;
}

string stripEntities(string input)
{
    string text = input;
    int begin = 0;
    int start, end = 0;

    while((start = text.find('&', begin)) != string::npos)
    {
        end = text.find(';', start);
        if(end++ != (int)string::npos)
            text.erase(start, end-start);

        if(end - 1 == (int)string::npos)
            begin = start + 1;
        else begin = end;
    }

    return text;
}

vector<Value> splitText(string text, vector<string> parent, int chars, int rows)
{
    vector<Value> data;
    int maxLen = chars * rows;

    Value value;
    value.mode = parent;

    for (unsigned int index = 0; index < text.length(); index += maxLen)
    {
        value.text = text.substr(index, MIN(maxLen, text.length() - index));
        data.push_back(value);
    }

    return data;
}

string findText(tree<HTML::Node>::iterator it)
{
    tree<HTML::Node>::iterator begin, ends;
    string text;
    begin = it;
    ends = it;
    ends.skip_children();
    ++ends;
    for (; begin != ends; ++begin)
    {
        if (!begin->isTag() && !begin->isClosing() && !begin->isComment())
        {
            char *decode=(char*)malloc(begin->text().length()+1);
            decode_html_entities_utf8(strcpy(decode, begin->text().c_str()),NULL);
            text.append(decode);
            free(decode);
        }
    }
    return text;
}

string search(string id, tree<HTML::Node>::iterator it)
{
    tree<HTML::Node>::iterator begin, ends;
    begin = it;
    ends = it;
    ends.skip_children();
    ++ends;
    for (; begin != ends; ++begin)
    {
        if (equal(begin->tagName(), "label"))
        {
            begin->parseAttributes();
            if(begin->attribute("for").second==id)
                return findText(begin);
        }
    }
    return "noLabel";
}

tree<HTML::Node>::iterator thtml(tree<HTML::Node>::iterator it, tree<HTML::Node>::iterator end, string dest)
{
    for (; it!=end; it++)
    {
        if (!it->isTag())
            continue;
        if (equal(it->tagName(), dest))
            break;
    }
    return it;
}

typedef struct
{
    bool open;
    Tag* p;
} last;

Lista getTag(char * buffer, char * url)
{
    bool parse_css=true;
    bool parsed=true;
    char *decode=NULL;

    string css_code;
    tree<HTML::Node> dom;
    Lista l1;

    try
    {
        string html(buffer);
        apply_ruleset(&html, url);

        HTML::ParserDom parser;
        CSS::Parser css_parser;

        //Dump all text of the document
        dom = parser.parseTree(html);
        tree<HTML::Node>::iterator it = dom.begin();
        tree<HTML::Node>::iterator end = dom.end();

        vector<string> parent;
        vector<Value> out;

        last open[all] = { {false},{false},{false},{false} };
        it=thtml(it,end,"html");

        if(it==end)
        {
            decode=(char*)malloc(html.length()+1);
            decode_html_entities_utf8(strcpy(decode, html.c_str()),NULL);
            string item(decode);
            free(decode);

            out = splitText(item, parent, 50, 40);
            if(checkchr(item))
            {
                l1.push_back( {"text"});
                open[text].open=true;
                open[text].p=&(*l1.rbegin());
                merge(open[text].p, out);
            }
        }

        if(css_code.length())
        {
            css_parser.parse(css_code);
        }

        for (; it!=end; ++it)
        {
            if (equal(it->tagName(), "script"))
            {
                it.skip_children();
                continue;
            }

            if (!it->isTag() && !it->isClosing() && !it->isComment())
            {
                if (!parent.empty() && checkparent(parent,TITLE))
                {
                    decode=(char*)malloc(it->text().length()+1);
                    decode_html_entities_utf8(strcpy(decode, it->text().c_str()),NULL);
                    string item(decode);
                    free(decode);

                    out = splitText(item, parent, 50, 40);
                    if(!checkchr(item))
                        continue;

                    if (open[title].open)
                        merge(open[title].p, out);
                    else if (open[a].open)
                        merge(open[a].p, out);
                    else if (open[text].open)
                        merge(open[text].p, out);

                    else
                    {
                        l1.push_back( {"text"});
                        open[text].open=true;
                        open[text].p=&(*l1.rbegin());
                        merge(open[text].p, out);
                    }
                }
            }

            else if (it->isTag())
            {
                if (parse_css)
                {
                    CSS::Parser style_parser;
                    vector<CSS::Parser::Selector> v;
                    tree<HTML::Node>::iterator k = it;
                    it->parseAttributes();

                    if (it->attribute("style").second.length() > 0)
                    {
                        string css_attr(it->tagName());
                        css_attr.append(" { " + it->attribute("style").second + " }");
                        style_parser.parse(css_attr);
                        parsed = false;
                    }

                    while (k != dom.begin())
                    {
                        CSS::Parser::Selector s;
                        s.setElement(k->tagName());
                        s.setId(k->attribute("id").second);
                        s.setClass(k->attribute("class").second);
                        s.setPseudoClass(CSS::Parser::NONE_CLASS);
                        s.setPseudoElement(CSS::Parser::NONE_ELEMENT);
                        v.push_back(s);
                        k = dom.parent(k);
                    }

                    map<string, string> attributes = css_parser.getAttributes(v);
                    map<string, string>::const_iterator mit = attributes.begin();
                    map<string, string>::const_iterator mend = attributes.end();

                    map<string, string> styles;
                    bool skip=false;

                    if (!parsed)
                    {
                        vector<CSS::Parser::Selector> vt;
                        CSS::Parser::Selector st;
                        st.setElement(it->tagName());
                        vt.push_back(st);

                        styles = style_parser.getAttributes(vt);
                        map<string, string>::const_iterator sit = styles.begin();
                        map<string, string>::const_iterator send = styles.end();

                        for (parsed = true; sit != send; ++sit)
                        {
                            if (sit->first=="display" && sit->second=="none")
                                skip=true;
                        }
                    }

                    for (; mit != mend; ++mit)
                    {
                        if (mit->first=="display" && mit->second=="none")
                            skip=true;
                    }

                    if (equal(it->tagName(), "STYLE"))
                    {
                        tree<HTML::Node>::iterator begin, end;
                        begin = it;
                        end = it;
                        end.skip_children();
                        ++end;
                        string css_snippet;

                        for (skip = true; begin != end; ++begin)
                        {
                            if (!begin->isTag())
                                css_snippet.append(begin->text());
                        }
                        css_parser.parse(css_snippet);
                    }

                    if (skip && !open[form].open)
                    {
                        it.skip_children();
                        continue;
                    }
                }

                if (it->closingText().length()>0)
                {
                    string tag=it->tagName();
                    parent.push_back(tag);
                }
                if (isContainer(it->tagName()))
                {
                    if (equal(it->tagName(), "p"))
                        l1.push_back( {"p"});
                    else l1.push_back( {"return"});
                    open[text].open=false;
                }
                if (equal(it->tagName(), "title"))
                {
                    l1.push_back( {"title"});
                    open[title].open=true;
                    open[title].p=&(*l1.rbegin());
                }
                else if (equal(it->tagName(), "a"))
                {
                    l1.push_back( {"a"});
                    l1.rbegin()->attribute.append (createLink(it));

                    open[a].open=true;
                    open[a].p=&(*l1.rbegin());
                    open[text].open=false;
                }
                else if (equal(it->tagName(), "img"))
                {
                    // if(!prevTagIs(it, "img") && !open[a].open)
                    if(!prevTagIs(it, "img") && !prevTagIs(it, "a"))
                    {
                        l1.push_back( {"return"});
                        open[text].open=false;
                    }

                    l1.push_back( {"img"});
                    l1.rbegin()->attribute.append (it->attribute("src").second);

                    l1.rbegin()->value.push_back ( {it->attribute("width").second});
                    l1.rbegin()->value.push_back ( {it->attribute("height").second});
                    l1.rbegin()->value.push_back ( {open[a].open ? open[a].p->attribute : "parent", parent});
                }
                else if (equal(it->tagName(), "form"))
                {
                    l1.push_back( {"form"});
                    l1.rbegin()->form.action.append (it->attribute("action").second);
                    l1.rbegin()->form.method.append (it->attribute("method").second);
                    l1.rbegin()->form.enctype.append (it->attribute("enctype").second);
                    open[form].open=true;
                    open[form].p=&(*l1.rbegin());
                }
                else if (equal(it->tagName(), "input"))
                {
                    if (open[form].open)
                    {
                        tree<HTML::Node>::iterator k = it;
                        while (!equal(it->tagName(), "form") && k != dom.begin())
                            k = dom.parent(k);
                        string label=search(it->attribute("id").second, k);
                        open[form].p->form.input=InsInFondo(open[form].p->form.input, it->attribute("name").second, it->attribute("type").second, stripEntities(it->attribute("value").second), label);
                        SetOption(open[form].p->form.input, it->attribute("checked").second);

                        if(it->attribute("type").second == "button")
                        {
                            l1.push_back( {"a"});
                            l1.rbegin()->attribute.append (createLink(it));
                            l1.rbegin()->value.push_back ( {it->attribute("value").second});
                        }
                    }
                }
                else if (equal(it->tagName(), "textarea"))
                {
                    if (open[form].open)
                    {
                        tree<HTML::Node>::iterator k = it;
                        while (!equal(it->tagName(), "form") && k != dom.begin())
                            k = dom.parent(k);
                        string label=search(it->attribute("id").second, k);
                        open[form].p->form.input=InsInFondo(open[form].p->form.input, it->attribute("name").second, it->tagName(), "", label);
                    }
                }
                else if (equal(it->tagName(), "meta"))
                {
                    l1.push_back( {"meta"});
                    l1.rbegin()->value.push_back ( {it->attribute("http-equiv").second});
                    l1.rbegin()->attribute.append (it->attribute("content").second);
                }
                else if (equal(it->tagName(), "base"))
                {
                    l1.push_back( {"base"});
                    l1.rbegin()->attribute.append (it->attribute("href").second);
                }
            }

            else if (!it->isComment())
            {
                if (!parent.empty())
                {
                    parent.pop_back();
                    if (equal(it->tagName(), "a"))
                        open[a].open=false;
                    else if (equal(it->tagName(), "title"))
                        open[title].open=false;
                    else if (equal(it->tagName(), "form"))
                        open[form].open=false;
                }
                if (isContainer(it->tagName()))
                {
                    if (equal(it->tagName(), "p"))
                        l1.push_back( {"p"});
                    else l1.push_back( {"return"});
                    open[text].open=false;
                }
                else if (equal(it->tagName(), "a"))
                    open[text].open=false;
            }
        }
    }

    catch (exception &e)
    {
        cerr << "Exception " << e.what() << " caught" << endl;
        exit(1);
    }
    catch (...)
    {
        cerr << "Unknow exception caught " << endl;
    }
    return l1;
}

string ParseList(char *buffer)
{
    tree<HTML::Node> dom;
    string res, html(buffer);
    HTML::ParserDom parser;

    res.append("<!DOCTYPE NETSCAPE-Bookmark-file-1>");
    res.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>");

    res.append("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"></meta>");
    res.append("<title>Bookmarks</title>");
    res.append("<dl>");

    //Dump all links of the document
    dom = parser.parseTree(html);
    tree<HTML::Node>::iterator it = dom.begin();
    tree<HTML::Node>::iterator end = dom.end();

    for (; it!=end; ++it)
    {
        if (equal(it->tagName(), "a"))
        {
            it->parseAttributes();
            string attr = it->attribute("href").second;
            string content = it->content(html);

            if(attr.length() > 0)
            {
                res.append("<dt>" + content + "</dt>");
            }
        }
    }

    res.append("</dl>");
    return res;
}
