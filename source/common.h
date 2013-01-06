#ifndef _COMMON_H_
#define _COMMON_H_

#include "libwiigui/gui.h"
#include <iostream>
#include <list>
#define LEN 15

extern "C" {
    #include <httplib.h>
}

using namespace std;
typedef string TipoElemento;

enum {image,guitext,guibutton,null};

typedef struct Input {
  TipoElemento name;
  TipoElemento type;
  TipoElemento value;
  TipoElemento label;
  struct Input *prox;
} Input;

typedef struct Value {
  TipoElemento text;
  vector<string> mode;
} Value;

typedef struct Form {
  TipoElemento action;
  TipoElemento method;
  Input *input;
} Form;

typedef struct Tag {
  TipoElemento name;
  TipoElemento attribute;
  vector<Value> value;
  struct Form form;
} Tag;

typedef struct Text {
  GuiText *txt;
  struct Text *prox;
} Text;

typedef struct Image {
  GuiImageData *imgdata;
  GuiImage *img;
  TipoElemento url;
  struct Image *prox;
} Image;

typedef struct Button {
  GuiButton *btn;
  GuiText *label;
  GuiTooltip *tooltip;
  TipoElemento url;
  struct Tag *refs;
  struct Button *prox;
} Button;

typedef struct Index {
  GuiElement *elem;
  int content;
  int screenSize;
  struct Index *prec;
  struct Index *prox;
} Index;

typedef struct Url {
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

extern enum html {HTML,HEAD,BODY,META,TITLE,FORM,P,A,DIV,BR,IMG,H1,H2,H3,H4,H5,H6,B,BIG,BLOCKQUOTE,TD,DD,DT,/*CENTER,*/LI,CITE,FONT,END} htm;

extern char tags [END][LEN];
extern History history;

extern Lista getTag(char *buffer);
extern int checkTag(vector<string> tag, string name);

extern void OnScreenKeyboard(GuiWindow *keyboardWindow, char *var, u16 maxlen);
extern int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label);

extern void save_mem(string str);
extern void save_mem_int(float str);

extern string DisplayHTML(struct block *HTML, GuiWindow *parentWindow, GuiWindow *mainWindow, char *url);
extern string adjustUrl(string link, const char* url);
extern char *getHost(char *url);

extern void HaltGui();
extern void ResumeGui();

#endif
