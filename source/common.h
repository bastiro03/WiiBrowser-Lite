#ifndef _COMMON_H_
#define _COMMON_H_

#include <httplib.h>
#include <iostream>
#include <list>

#include <libwiigui/gui.h>
#define LEN 15

using namespace std;
typedef string TipoElemento;

enum {image,guitext,guibutton,null};

typedef struct Input
{
    TipoElemento name;
    TipoElemento type;
    TipoElemento value;
    TipoElemento option;
    TipoElemento label;
    struct Input *prox;
} Input;

typedef struct Value
{
    TipoElemento text;
    vector<string> mode;
} Value;

typedef struct Form
{
    TipoElemento action;
    TipoElemento method;
    TipoElemento enctype;
    Input *input;
} Form;

typedef struct Tag
{
    TipoElemento name;
    TipoElemento attribute;
    vector<Value> value;
    struct Form form;
} Tag;

typedef struct Text
{
    GuiText *txt;
    struct Text *prox;
} Text;

typedef struct Image
{
    GuiImageData *imgdata;
    GuiImage *img;
    bool fetched;
    struct Tag *tag;
    struct Image *prox;
} Image;

typedef struct Button
{
    GuiButton *btn;
    GuiText *label;
    GuiTooltip *tooltip;
    TipoElemento url;
    int outline;
    struct Tag *refs;
    struct Button *prox;
} Button;

typedef struct Index
{
    GuiElement *elem;
    int content;
    int screenSize;
    struct Index *prec;
    struct Index *prox;
} Index;

typedef struct Page
{
    int XPos;
    int YPos;
    int Height;
} Page;

typedef struct Url
{
    TipoElemento url;
    struct Url *prec;
    struct Url *prox;
} Url;

typedef Text * ListaDiTesto;
typedef Image * ListaDiImg;
typedef Button * ListaDiBottoni;
typedef Input * ListaDiInput;

typedef list<Tag> Lista;
typedef Index * Indice;
typedef Url * History;

extern enum html {
    HTML,HEAD,BODY,BASE,META,TITLE,FORM,P,A,DIV,BR,IMG,
    H1,H2,H3,H4,H5,H6,B,BIG,BLOCKQUOTE,PRE,TD,DD,DT,/*CENTER,*/LI,CITE,FONT,END
}
htm;

extern char tags [END][LEN];
extern History history;

extern Lista getTag(char *buffer, char *url);
extern int checkTag(vector<string> tag, string name);

extern int OnScreenKeyboard(GuiWindow *keyboardWindow, char *var, u16 maxlen);
extern int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label, const char *longText = NULL);
extern void ToggleButtons(GuiToolbar *toolbar, bool checkState = 0);

extern void save_mem(string str);
extern void save_mem_int(float str);

extern string DisplayHTML(struct block *HTML, GuiWindow *parentWindow, GuiWindow *mainWindow, char *url);
extern string adjustUrl(string link, const char* url);
extern char *getHost(char *url);

extern void ResumeGui();

#ifdef __cplusplus
extern "C" {
#endif
    extern void HaltGui();
#ifdef __cplusplus
}
#endif

#endif
