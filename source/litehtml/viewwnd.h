#pragma once
#include "../include/litehtml.h"
#include "wii_container.h"

using namespace litehtml;
using namespace std;

class CHTMLViewWnd : public wii_container
{
	int m_top;
	int m_left;
	int m_max_top;
	int m_max_left;
	std::wstring m_base_path;
	std::wstring m_doc_path;
	context* m_context;
	std::wstring m_anchor;
	string_vector m_history_back;
	string_vector m_history_forward;
	position m_clip;

public:
	document::ptr m_doc;

	CHTMLViewWnd(context* ctx);
	~CHTMLViewWnd(void) override;

	void create();
	void open(wstring path);
	void refresh();
	void back();
	void forward();

	// litehtml::document_container members
	void set_base_url(const wchar_t* base_url) override;
	void link(document* doc, element::ptr el) override;
	void import_css(std::wstring& text, const std::wstring& url, std::wstring& baseurl,
	                const string_vector& media) override;
	void on_anchor_click(const wchar_t* url, element::ptr el) override;

	virtual void OnCreate();
	virtual void OnPaint();
	virtual void SetClip(position clip);
	virtual void OnSize(int width, int height);
	virtual void OnDestroy();
	virtual void OnVScroll(int pos, int flags);
	virtual void OnKeyDown(int vKey);
	virtual void OnMouseMove(int x, int y);
	virtual void OnLButtonDown(int x, int y);
	virtual wchar_t* OnLButtonUp(int x, int y);
	virtual void OnMouseLeave();

	void set_wind(GuiWindow* main);

protected:
	std::wstring make_url(std::wstring link, const wchar_t* url) override;
	void render();
	void redraw();
};
