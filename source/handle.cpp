#include "handle.h"

GuiToolbar *Toolbar = NULL;

void MoveAll(int dir, int space, GuiWindow * mainWindow);
void MoveWind (int up, int dir, int space, GuiImage * image);
void MoveText (int up, int dir, int space, Indice Index);

void HandleFormPad(GuiButton *btnup, GuiButton *btndown, GuiWindow *mainWindow);

bool hidden=true;
enum { TEXT=0, BUTTON, HIDDEN, RADIO, TEXTAREA, UPLOAD, UNKNOWN };

int inputType(ListaDiInput lista);
int noSubmit(ListaDiBottoni btn);

int x = 0;
int y = 0;

int HandleForm(GuiWindow* parentWindow, GuiWindow* mainWindow, ListaDiBottoni btn)
{
    GuiImageData Textbox (keyboard_textbox_png, keyboard_textbox_png_size);
    GuiImageData TextboxImgData (bg_options_png, bg_options_png_size);
    GuiImageData Button (button_png, button_png_size);

    GuiSound *btnSoundOver= new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiTrigger *trigA=new GuiTrigger;
    trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

    GuiWindow Form(screenwidth, screenheight);
    Form.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    Form.SetPosition(20,20);

    HaltGui();
    parentWindow->Remove(mainWindow);
    Form.SetEffect(EFFECT_FADE, 50);
    parentWindow->Append(&Form);
    parentWindow->ChangeFocus(&Form);
    ResumeGui();

    btn->url.assign(btn->refs->form.action);
    btn->url.append("?");

    ListaDiInput lista=btn->refs->form.input;
    ListaDiBottoni bottone, choice=NULL, button=NULL;
    GuiText *label = NULL;

    char *encode;
    int offset=0;
    for (; !NoInput(lista); lista=lista->prox)
    {
        if (inputType(lista)==BUTTON)
        {
            button=InsButton(button, BUTTON);
            GuiImage *ButtonImg=new GuiImage(&Button);
            button->btn=new GuiButton(ButtonImg->GetWidth(), ButtonImg->GetHeight());
            button->label=new GuiText((char*)lista->value.c_str(), 20, (GXColor)
            {
                0, 0, 255, 255
            });
            button->label->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
            button->label->SetMaxWidth(ButtonImg->GetWidth()-5);
            button->label->SetScroll(SCROLL_HORIZONTAL);

            button->btn->SetLabel(button->label);
            button->btn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            button->btn->SetPosition(40, offset);
            button->btn->SetImage(ButtonImg);
            button->btn->SetSoundOver(btnSoundOver);
            button->btn->SetTrigger(trigA);
            button->btn->SetEffectGrow();
            button->btn->SetEffect(EFFECT_FADE, 50);
            HaltGui();
            Form.Append(button->btn);
            ResumeGui();
            offset+=button->btn->GetHeight()+10;
        }
        else if (inputType(lista)==HIDDEN)
        {
            button=InsButton(button, HIDDEN);
            button->btn=NULL;
        }
        else if (inputType(lista)==UPLOAD)
        {
            button=InsButton(button, UPLOAD);
            GuiImage *TextboxImg=new GuiImage(&Textbox);
            button->btn=new GuiButton(TextboxImg->GetWidth(), TextboxImg->GetHeight());
            button->label=new GuiText("", 20, (GXColor)
            {
                0, 0, 0, 255
            });

            label=new GuiText("Upload", 20, (GXColor)
            {
                0, 0, 255, 255
            });
            label->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            label->SetPosition(0,-25);
            label->SetMaxWidth(TextboxImg->GetWidth()-5);
            label->SetScroll(SCROLL_HORIZONTAL);
            button->btn->SetLabel(label);
            offset+=25;

            button->btn->SetLabel(button->label,1);
            button->btn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            button->btn->SetPosition(0, offset);
            button->btn->SetImage(TextboxImg);
            button->btn->SetSoundOver(btnSoundOver);
            button->btn->SetTrigger(trigA);
            button->btn->SetEffectGrow();
            button->btn->SetEffect(EFFECT_FADE, 50);
            HaltGui();
            Form.Append(button->btn);
            ResumeGui();
            offset+=TextboxImg->GetHeight()+10;
        }
        else if (inputType(lista)==TEXT)
        {
            button=InsButton(button, TEXT);
            GuiImage *TextboxImg=new GuiImage(&Textbox);
            button->btn=new GuiButton(TextboxImg->GetWidth(), TextboxImg->GetHeight());
            button->label=new GuiText("", 20, (GXColor)
            {
                0, 0, 0, 255
            });
            if (lista->label!="noLabel")
            {
                label=new GuiText((char*)lista->label.c_str(), 20, (GXColor)
                {
                    0, 0, 255, 255
                });
                label->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                label->SetPosition(0,-25);
                label->SetMaxWidth(TextboxImg->GetWidth()-5);
                label->SetScroll(SCROLL_HORIZONTAL);
                button->btn->SetLabel(label);
                offset+=25;
            }
            button->btn->SetLabel(button->label,1);
            button->btn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            button->btn->SetPosition(0, offset);
            button->btn->SetImage(TextboxImg);
            button->btn->SetSoundOver(btnSoundOver);
            button->btn->SetTrigger(trigA);
            button->btn->SetEffectGrow();
            button->btn->SetEffect(EFFECT_FADE, 50);
            HaltGui();
            Form.Append(button->btn);
            ResumeGui();
            offset+=TextboxImg->GetHeight()+10;
        }
        else if (inputType(lista)==RADIO)
        {
            if (lista->option=="checked")
            {
                button=InsButton(button, RADIO);
                button->btn=NULL;
            }
            else
                continue;
        }
        else if (inputType(lista)==TEXTAREA)
        {
            button=InsButton(button, TEXT);
            GuiImage *TextboxImg = new GuiImage(&TextboxImgData);
            TextboxImg->SetScale(0.86);
            button->btn=new GuiButton(TextboxImg->GetRealWidth(), TextboxImg->GetRealHeight());
            button->label=new GuiLongText("", 20, (GXColor)
            {
                0, 0, 0, 255
            });
            button->label->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
            button->label->SetLinesToDraw(TextboxImg->GetRealHeight()/25);
            button->label->SetMaxWidth(TextboxImg->GetRealWidth()-45);
            button->label->SetPosition(20, 25);
            if (lista->label!="noLabel")
            {
                label=new GuiText((char*)lista->label.c_str(), 20, (GXColor)
                {
                    0, 0, 255, 255
                });
                label->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                label->SetPosition(0,-25);
                label->SetMaxWidth(TextboxImg->GetRealWidth()-5);
                label->SetScroll(SCROLL_HORIZONTAL);
                button->btn->SetLabel(label);
                offset+=25;
            }
            button->btn->SetLabel(button->label, 1);
            button->btn->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
            button->btn->SetPosition(0, offset);
            button->btn->SetImage(TextboxImg);
            button->btn->SetSoundOver(btnSoundOver);
            button->btn->SetTrigger(trigA);
            button->btn->SetEffect(EFFECT_FADE, 50);
            HaltGui();
            Form.Append(button->btn);
            ResumeGui();
            offset+=TextboxImg->GetRealHeight()+10;
        }
        else if (inputType(lista)==UNKNOWN)
        {
            button=InsButton(button, UNKNOWN);
            button->btn=NULL;
        }
        button->url.assign(lista->name);
        button->url.append("=");
        if (button->url.length()>1 && lista->value.length()>0)
        {
            encode=url_encode((char*)lista->value.c_str());
            button->url.append(encode);
            free(encode);
        }
    }

    char save[MAXLEN];
    memset(save, 0, sizeof(save));

    if (button)
    {
        while (!(userInput[0].wpad->btns_d & WPAD_BUTTON_B) && !choice)
        {
            HandleFormPad(NULL, NULL, &Form);
            usleep(100);
            for (bottone=button; !NoButton(bottone); bottone=bottone->prox)
            {
                if(bottone->btn && bottone->btn->GetState() == STATE_CLICKED)
                {
                    if(bottone->outline==TEXT)
                    {
                        size_t found=bottone->url.find("=")+1;
                        if (found!=string::npos)
                            bottone->url.erase(found);
                        sprintf(save, bottone->label->GetText());
                        OnScreenKeyboard(parentWindow, save, MAXLEN);
                        bottone->label->SetText(save);
                        encode=url_encode(save);
                        bottone->url.append(encode);
                        free(encode);
                    }
                    else if(bottone->outline==UPLOAD)
                    {
                        size_t found=bottone->url.find("=")+1;
                        if (found!=string::npos)
                            bottone->url.erase(found);
                        if(GuiBrowser(&Form, parentWindow, save, "Upload"))
                        {
                            bottone->label->SetText(save);
                            bottone->url.append(save);
                            bottone->url.append("_UPLOAD");
                        }
                        else bottone->label->SetText("");
                    }
                    else
                        choice=bottone;
                    bottone->btn->ResetState();
                }
            }
        }
    }

    for (bottone=button; !NoButton(bottone); bottone=bottone->prox)
    {
        if (!bottone->btn || bottone==choice
                || bottone->outline==TEXT || bottone->outline==UPLOAD)
        {
            if (bottone->url.length()>1)
            {
                btn->url.append(bottone->url);
                if (!NoButton(bottone->prox))
                    btn->url.append("&");
            }
        }
    }

    btn->url.append("\\");
    if (strcasestr(btn->refs->form.enctype.c_str(), "multipart/form-data"))
        btn->url.append(btn->refs->form.enctype);
    else btn->url.append(btn->refs->form.method);

    HaltGui();
    parentWindow->Remove(&Form);
    mainWindow->SetEffect(EFFECT_FADE, 50);
    parentWindow->Append(mainWindow);
    ResumeGui();

    DistruggiButton(button);
    delete(btnSoundOver);
    delete(trigA);
    if (choice || noSubmit(btn))
        return 1;
    return 0;
}

void HandleFormPad(GuiButton *btnup, GuiButton *btndown, GuiWindow *mainWindow)
{
    if (userInput[0].wpad->btns_h & WPAD_BUTTON_UP)
    {
        usleep(15000);
        MoveAll(+1,25, mainWindow);
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN)
    {
        usleep(15000);
        MoveAll(-1,25, mainWindow);
    }
}

void HandleHtmlPad(int *offset, GuiButton *btnup, GuiButton *btndown, Indice ext, Indice Index, GuiWindow *mainWindow, GuiWindow *parentWindow)
{
    if (btnup->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_UP))
    {
        usleep(15000);
        btnup->ResetState();
        if (!ext || ext->elem->GetYPosition()+25<screenheight)
            MoveText(1,+1,25, Index);
    }

    if (btndown->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN))
    {
        usleep(15000);
        btndown->ResetState();
        if (!Index || Index->elem->GetYPosition()+Index->screenSize>25)
            MoveText(1,-1,25, Index);
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_LEFT)
    {
        if (*offset<screenwidth)
        {
            usleep(15000);
            MoveText(0,+1,25, Index);
            *offset+=25;
        }
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_RIGHT)
    {
        if (*offset>-screenwidth)
        {
            usleep(15000);
            MoveText(0,-1,25, Index);
            *offset-=25;
        }
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_1)
    {
        char path[256];

        if (GuiBrowser(mainWindow, parentWindow, path, "Take screenshot"))
        {
            HaltGui();
            DoMPlayerGuiDraw();
            SaveScreenshot(path);
            ResumeGui();
        }
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_2)
    {
        if (*offset)
        {
            usleep(15000);
            MoveText(0,-1,(*offset), Index);
            *offset=0;
        }
    }

    if(userInput[0].wpad->btns_d & WPAD_BUTTON_B)
    {
        if(userInput[0].wpad->ir.valid)
        {
            x = userInput[0].wpad->ir.x;
            y = userInput[0].wpad->ir.y;
        }
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_B)
    {
        usleep(15000);
        if(userInput[0].wpad->ir.valid)
        {
            int dist = userInput[0].wpad->ir.y - y;
            if(dist > 200) dist = 200;
            else if (dist < -200) dist = -200;

            if (!ext || ext->elem->GetYPosition()+25<screenheight)
            {
                if (dist > 10)
                    MoveText(1,1,dist/8, Index);
            }

            if (!Index || Index->elem->GetYPosition()+Index->screenSize>25)
            {
                if (dist < -10)
                    MoveText(1,1,dist/8, Index);
            }
        }
    }
}

void HandleImgPad(GuiButton *btnup, GuiButton *btndown, GuiImage * image)
{
    if (btnup->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_UP))
    {
        if (userInput[0].wpad->btns_h & WPAD_BUTTON_UP)
            usleep(15000);
        btnup->ResetState();
        MoveWind(1,+1,25, image);
    }

    if (btndown->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN))
    {
        if (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN)
            usleep(15000);
        btndown->ResetState();
        MoveWind(1,-1,25, image);
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_LEFT)
    {
        usleep(15000);
        MoveWind(0,+1,25, image);
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_RIGHT)
    {
        usleep(15000);
        MoveWind(0,-1,25, image);
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_PLUS)
    {
        usleep(15000);
        image->SetScale(image->GetScale()+image->GetScale()/100*5);
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_MINUS)
    {
        usleep(15000);
        image->SetScale(image->GetScale()-image->GetScale()/100*5);
    }

    if(userInput[0].wpad->btns_d & WPAD_BUTTON_B)
    {
        if(userInput[0].wpad->ir.valid)
        {
            x = userInput[0].wpad->ir.x;
            y = userInput[0].wpad->ir.y;
        }
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_B)
    {
        usleep(15000);
        if(userInput[0].wpad->ir.valid)
        {
            int distx = userInput[0].wpad->ir.x - x;
            if(distx > 200) distx = 200;
            else if (distx < -200) distx = -200;

            if (distx > 10)
                MoveWind(0,+1,distx/8, image);
            else if (distx < -10)
                MoveWind(0,+1,distx/8, image);

            int disty = userInput[0].wpad->ir.y - y;
            if(disty > 200) disty = 200;
            else if (disty < -200) disty = -200;

            if (disty > 10)
                MoveWind(1,+1,disty/8, image);
            else if (disty < -10)
                MoveWind(1,+1,disty/8, image);
        }
    }
}

void MoveWind (int up, int dir, int space, GuiImage * image)
{
    if (up)
        image->SetPosition(image->GetXPosition(), image->GetYPosition()+(space*dir));
    else
        image->SetPosition(image->GetXPosition()+(space*dir), image->GetYPosition());
}

void MoveText (int up, int dir, int space, Indice Index)
{
    if (up)
    {
        for (; !NoIndex(Index); Index=Index->prox)
            Index->elem->SetPosition(Index->elem->GetXPosition(), Index->elem->GetYPosition()+(space*dir));
    }
    else
    {
        for (; !NoIndex(Index); Index=Index->prox)
            Index->elem->SetPosition(Index->elem->GetXPosition()+(space*dir), Index->elem->GetYPosition());
    }
}

void MoveAll(int dir, int space, GuiWindow *mainWindow)
{
    int i = mainWindow->GetSize() - 1;
    GuiElement *element;

    if (i < 0)
        return;

    element = mainWindow->GetGuiElementAt(0);
    if (dir > 0 && element->GetYPosition()+25 > screenheight)
        return;

    element = mainWindow->GetGuiElementAt(i);
    if (dir < 0 && element->GetYPosition()+element->GetHeight() < 25)
        return;

    for (; i>=0; i--)
    {
        element = mainWindow->GetGuiElementAt(i);
        element->SetPosition(element->GetXPosition(), element->GetYPosition()+(space*dir));
    }
}

int inputType(ListaDiInput lista)
{
    if (lista->type=="submit" /*|| lista->type=="reset"*/)
        return BUTTON;
    if (lista->type=="hidden")
        return HIDDEN;
    if (lista->type=="text" || lista->type=="email" || lista->type=="password"
            || lista->type=="search" || lista->type=="tel" || lista->type.length()==0)
        return TEXT;
    if (lista->type=="radio")
        return RADIO;
    if (lista->type=="textarea")
        return TEXTAREA;
    if (lista->type=="file")
        return UPLOAD;
    return UNKNOWN;
}

int noSubmit(ListaDiBottoni btn)
{
    ListaDiInput lista=btn->refs->form.input;
    for (; !NoInput(lista); lista=lista->prox)
    {
        if (lista->type=="submit")
            return 0;
    }
    return 1;
}

void showBar(GuiWindow *mainWindow, GuiWindow *parentWindow)
{
    if (!Toolbar)
        Toolbar = new GuiToolbar(NAVIGATION);
    ToggleButtons(Toolbar, true);
    Toolbar->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 20);
    hidden=false;
    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    parentWindow->Append(Toolbar);
    parentWindow->ChangeFocus(Toolbar);
    ResumeGui();
}

void hideBar(GuiWindow *mainWindow, GuiWindow *parentWindow)
{
    if (Toolbar)
    {
        Toolbar->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 20);
        while(Toolbar->GetEffect() > 0) usleep(100);
        HaltGui();
        parentWindow->Remove(Toolbar);
        mainWindow->SetState(STATE_DEFAULT);
        parentWindow->ChangeFocus(mainWindow);
        ResumeGui();
    }
    hidden=true;
}

void HandleMenuBar(string *link, char* url, int *choice, int img, GuiWindow *mainWindow, GuiWindow *parentWindow)
{
    if ((img ? userInput[0].wpad->btns_d & WPAD_BUTTON_1 : userInput[0].wpad->btns_d & WPAD_BUTTON_PLUS) && hidden)
        showBar(mainWindow, parentWindow);
    if ((img ? userInput[0].wpad->btns_d & WPAD_BUTTON_2 : userInput[0].wpad->btns_d & WPAD_BUTTON_MINUS) && !hidden)
        hideBar(mainWindow, parentWindow);

    if (hidden)
    {
        if ((userInput[0].wpad->btns_h & WPAD_BUTTON_B) && (userInput[0].wpad->btns_h & WPAD_BUTTON_RIGHT))
            *choice=1;
        if ((userInput[0].wpad->btns_h & WPAD_BUTTON_B) && (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN))
            *choice=2;

        if ((userInput[0].wpad->btns_h & WPAD_BUTTON_B) && (userInput[0].wpad->btns_h & WPAD_BUTTON_UP))
        {
            link->assign(url);
            *choice=1;
        }

        if ((userInput[0].wpad->btns_h & WPAD_BUTTON_B) && (userInput[0].wpad->btns_h & WPAD_BUTTON_LEFT))
        {
            if (strncmp(Settings.Homepage,"http",4))
                link->assign("http://");
            link->append(Settings.Homepage);
            *choice=1;
        }

        if ((userInput[0].wpad->btns_h & WPAD_BUTTON_B) && (userInput[0].wpad->btns_h & WPAD_BUTTON_MINUS))
        {
            if (history->prec)
            {
                history=history->prec;
                link->assign(history->url);
                *choice=1;
            }
        }

        if ((userInput[0].wpad->btns_h & WPAD_BUTTON_B) && (userInput[0].wpad->btns_h & WPAD_BUTTON_PLUS))
        {
            if (history->prox)
            {
                history=history->prox;
                link->assign(history->url);
                *choice=1;
            }
        }
    }

    if (Toolbar)
    {
        if (Toolbar->btnWWW->GetState() == STATE_CLICKED)
        {
            Toolbar->btnWWW->ResetState();
            *choice=1;
        }
        if (Toolbar->btnSave->GetState() == STATE_CLICKED)
        {
            Toolbar->btnSave->ResetState();
            *choice=2;
        }

        if (Toolbar->btnHome->GetState() == STATE_CLICKED)
        {
            Toolbar->btnHome->ResetState();
            if (strncmp(Settings.Homepage,"http",4))
                link->assign("http://");
            link->append(Settings.Homepage);
            *choice=1;
        }
        if (Toolbar->btnReload->GetState() == STATE_CLICKED)
        {
            Toolbar->btnReload->ResetState();
            link->assign(url);
            *choice=1;
        }

        if (Toolbar->btnBack->GetState() == STATE_CLICKED)
        {
            Toolbar->btnBack->ResetState();
            if (history->prec)
            {
                history=history->prec;
                link->assign(history->url);
                *choice=1;
            }
        }

        if (Toolbar->btnForward->GetState() == STATE_CLICKED)
        {
            Toolbar->btnForward->ResetState();
            if (history->prox)
            {
                history=history->prox;
                link->assign(history->url);
                *choice=1;
            }
        }
    }
}

int HandleMeta(Lista::iterator lista, string *link, struct block *html)
{
    int choice = 0;
    if (lista->value[0].text=="refresh")
    {
        sleep(getTime(lista->attribute));
        link->assign(getUrl(&choice, lista->attribute));
    }
    return choice;
}

int getTime(string url)
{
    int found, t=0;
    if ((found=url.find_first_of(";", 0))!=string::npos)
        t=atoi(url.substr(0,found).c_str());
    return t>0 ? t:1;
}

string getUrl(int *choice, string url)
{
    int str,found;
    if ((str=url.find("url="))!=string::npos)
    {
        while ((found=url.find("&#39;"))!=string::npos)
            url.erase (found,5);
        *choice=1;
        return url.substr(str+4);
    }
    return "";
}
