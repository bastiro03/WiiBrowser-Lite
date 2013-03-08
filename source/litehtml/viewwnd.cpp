#include <wctype.h>
#include <algorithm>

#include "..\FreeTypeGX.h"
#include "..\httplib.h"
#include "..\video.h"

#include "viewwnd.h"
#include "common.h"

wchar_t *load_text_file( const wchar_t *url, bool use_iconv )
{
    struct block HTML;
    CURL *curl_upd = curl_easy_init();

    char *ascii = new char[wcslen(url) + 1];
    int bt = wcstombs(ascii, url, wcslen(url));
    ascii[bt] = 0;

    FILE *file = fopen("c.txt", "wb");
    HTML = downloadfile(curl_upd, ascii, file);
    delete(ascii);
    if(HTML.size == 0)
        return NULL;

	curl_easy_cleanup(curl_upd);
	wchar_t *text = NULL;

	if(use_iconv)
        text = converIconv(HTML.data, "UTF-8");
    else text = charToWideChar(HTML.data);

	free(HTML.data);
	return text;
}

enum
{
    SB_LINEDOWN,
    SB_PAGEDOWN,
    SB_LINEUP,
    SB_PAGEUP,
};

CHTMLViewWnd::CHTMLViewWnd(litehtml::context* ctx)
{
	m_doc		= NULL;
	m_top		= 0;
	m_left		= 0;
	m_max_top	= 0;
	m_max_left	= 0;
	m_context	= ctx;
}

CHTMLViewWnd::~CHTMLViewWnd(void)
{
}

void CHTMLViewWnd::OnCreate()
{

}

void CHTMLViewWnd::OnPaint( litehtml::position clip )
{
	if(m_doc)
	{
		m_doc->draw(0, -m_left, -m_top, &clip);
	}
}

void CHTMLViewWnd::OnSize( int width, int height )
{
	render();
}

void CHTMLViewWnd::OnDestroy()
{

}

void CHTMLViewWnd::create( void )
{

}

void CHTMLViewWnd::open( wstring path )
{
	if(!m_doc_path.empty())
	{
		m_history_back.push_back(m_doc_path);
	}

	m_doc_path  = make_url(path, NULL);
	m_doc		= NULL;
	m_base_path = m_doc_path;

	wchar_t *html_text = load_text_file(m_doc_path.c_str(), false);
	if(html_text)
	{
		m_doc = litehtml::document::createFromString(html_text, this, m_context);
	}

	m_top	= 0;
	m_left	= 0;
	render();
}

void CHTMLViewWnd::render()
{
	if(!m_doc)
	{
		return;
	}

	litehtml::position rcClient;
	get_client_rect(rcClient);

	int width	= rcClient.right() - rcClient.left();
	int height	= rcClient.bottom() - rcClient.top();

	m_doc->render(width);

	m_max_top = m_doc->height() - height;
	if(m_max_top < 0)
        m_max_top = 0;

	m_max_left = m_doc->width() - width;
	if(m_max_left < 0)
        m_max_left = 0;

	redraw();
}

void CHTMLViewWnd::redraw()
{

}

void CHTMLViewWnd::set_base_url( const wchar_t* base_url )
{
	if(base_url && base_url[0])
	{
        wstring temp(base_url);
        m_base_path = make_url(temp, m_base_path.c_str());
	}
	else
	{
		m_base_path = m_doc_path;
	}
}

void CHTMLViewWnd::link( litehtml::document* doc, litehtml::element::ptr el )
{
	const wchar_t* rel = el->get_attr(L"rel");
	if(rel && !wcscmp(rel, L"stylesheet"))
	{
		const wchar_t* media = el->get_attr(L"media", L"screen");
		if(media && (wcsstr(media, L"screen") || wcsstr(media, L"all")))
		{
			const wchar_t* href = el->get_attr(L"href");
			if(href)
			{
				std::wstring url, temp(href);
				url = make_url(temp, m_base_path.c_str());
				wchar_t *css = load_text_file(url.c_str(), true);
				if(css)
				{
					doc->add_stylesheet(css, url.c_str());
					delete css;
				}
			}
		}
	}
}

wchar_t *wcsndup( const wchar_t *wideString, size_t  length )
{
    /* Local variables. */
    wchar_t  *duplicate ;

    duplicate = (wchar_t *) calloc (length + 1, sizeof (wchar_t)) ;
    if (duplicate == NULL) {
        return (NULL) ;
    }

    if (wideString == NULL) {
        duplicate[0] = 0 ;
    } else {
        wcsncpy (duplicate, wideString, length) ;
        duplicate[length] = 0 ;
    }

    return (duplicate) ;
}

wchar_t *getHost( wchar_t *url )
{
    wchar_t *p = wcschr(url, L'/')+2;
    wchar_t *c = wcschr(p, L'/');
    if (c != NULL)
        return wcsndup(url,(c+1)-url);
    return url;
}

wstring getRoot( wchar_t *url )
{
    wchar_t *i = wcschr(url, L'/');
    wchar_t *p = wcsrchr(i+2, L'/');
    wstring root(url);
    if (p != NULL)
        root.assign(url,(p+1)-url);
    else root.append(L"/");
    return root;
}

wstring CHTMLViewWnd::make_url( wstring link, const wchar_t* url )
{
    if(!url || !url[0])
        url = m_base_path.c_str();

    wstring result;
    if (link.find(L"http://")==0 || link.find(L"https://")==0)
        return link;
    else if (link.at(0) == L'/' && link.at(1) == L'/')
        result.assign(L"http:"); // https?
    else if (link.at(0) == L'/')
    {
        result = getHost((wchar_t *)url);
        if (*result.rbegin() == '/')
            link.erase(link.begin());
    }
    else result = getRoot((wchar_t *)url);
    result.append(link);
    return result;
}

void CHTMLViewWnd::on_anchor_click( const wchar_t* url, litehtml::element::ptr el )
{
    wstring temp(url);
    m_anchor = make_url(temp, m_base_path.c_str());
}

/*
void CHTMLViewWnd::update_scroll()
{
	if(!m_doc)
	{
		ShowScrollBar(m_hWnd, SB_BOTH, FALSE);
		return;
	}

	if(m_max_top > 0)
	{
		ShowScrollBar(m_hWnd, SB_VERT, TRUE);
    }
}
*/

void CHTMLViewWnd::OnVScroll( int pos, int flags )
{
	litehtml::position rcClient;
	get_client_rect(rcClient);

	int lineHeight	= 16;
	int pageHeight	= rcClient.bottom() - rcClient.top() - lineHeight;

	int newTop = m_top;

	switch(flags)
	{
	case SB_LINEDOWN:
		newTop = m_top + lineHeight;
		if(newTop > m_max_top)
		{
			newTop = m_max_top;
		}
		break;
	case SB_PAGEDOWN:
		newTop = m_top + pageHeight;
		if(newTop > m_max_top)
		{
			newTop = m_max_top;
		}
		break;
	case SB_LINEUP:
		newTop = m_top - lineHeight;
		if(newTop < 0)
		{
			newTop = 0;
		}
		break;
	case SB_PAGEUP:
		newTop = m_top - pageHeight;
		if(newTop < 0)
		{
			newTop = 0;
		}
		break;
	}

	if(newTop != m_top)
	{
		// ScrollWindowEx(m_hWnd, 0, m_top - newTop, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_ERASE);
		m_top  = newTop;
	}
}

void CHTMLViewWnd::OnKeyDown( int vKey )
{
	switch(vKey)
	{
	// case VK_F5:
	default:
		// refresh();
		break;
	}
}

void CHTMLViewWnd::refresh()
{
	open(m_doc_path.c_str());
	redraw();
}

void CHTMLViewWnd::OnMouseMove( int x, int y )
{
	if(m_doc)
	{
		litehtml::position::vector redraw_boxes;
		if(m_doc->on_mouse_over(x + m_left, y + m_top, redraw_boxes))
		{
			for(litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
			{
				box->x -= m_left;
				box->y -= m_top;
				redraw();
			}
		}
	}
}

void CHTMLViewWnd::OnMouseLeave()
{
	if(m_doc)
	{
		litehtml::position::vector redraw_boxes;
		if(m_doc->on_mouse_leave(redraw_boxes))
		{
			for(litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
			{
				box->x -= m_left;
				box->y -= m_top;
				redraw();
			}
		}
	}
}

void CHTMLViewWnd::OnLButtonDown( int x, int y )
{
	if(m_doc)
	{
		litehtml::position::vector redraw_boxes;
		if(m_doc->on_lbutton_down(x + m_left, y + m_top, redraw_boxes))
		{
			for(litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
			{
				box->x -= m_left;
				box->y -= m_top;
				redraw();
			}
		}
	}
}

void CHTMLViewWnd::OnLButtonUp( int x, int y )
{
	if(m_doc)
	{
		m_anchor = L"";
		litehtml::position::vector redraw_boxes;
		if(m_doc->on_lbutton_up(x + m_left, y + m_top, redraw_boxes))
		{
			for(litehtml::position::vector::iterator box = redraw_boxes.begin(); box != redraw_boxes.end(); box++)
			{
				box->x -= m_left;
				box->y -= m_top;
				redraw();
			}
		}
		if(!m_anchor.empty())
		{
			open(m_anchor.c_str());
		}
	}
}

void CHTMLViewWnd::back()
{
	if(!m_history_back.empty())
	{
		if(!m_doc_path.empty())
		{
			m_history_forward.push_back(m_doc_path);
		}
		std::wstring url = m_history_back.back();
		m_history_back.pop_back();
		m_doc_path = L"";
		open(url.c_str());
	}
}

void CHTMLViewWnd::forward()
{
	if(!m_history_forward.empty())
	{
		if(!m_doc_path.empty())
		{
			m_history_back.push_back(m_doc_path);
		}
		std::wstring url = m_history_forward.back();
		m_history_forward.pop_back();
		m_doc_path = L"";
		open(url.c_str());
	}
}

void CHTMLViewWnd::import_css( std::wstring& text, const std::wstring& url, std::wstring& baseurl, const string_vector& media )
{
	if(media.empty() || std::find(media.begin(), media.end(), std::wstring(L"all")) != media.end() || std::find(media.begin(), media.end(), std::wstring(L"screen")) != media.end())
	{
		std::wstring css_url;
		css_url = make_url(url, baseurl.c_str());
		wchar_t *css = load_text_file(css_url.c_str(), true);
		if(css)
		{
			baseurl = css_url;
			text = css;
			delete css;
		}
	}
}
