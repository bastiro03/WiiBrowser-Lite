#include "handle.h"
#include "main.h"

GuiToolbar *Toolbar = NULL;

void MoveWind (int up, int dir, int line, GuiImage * image);
void MoveText (int up, int dir, int line, Indice Index);

bool hidden=true;
enum { TEXT=0, BUTTON, HIDDEN, UNKNOWN };

int inputType(ListaDiInput lista);
int noSubmit(ListaDiBottoni btn);

int HandleForm(GuiWindow* parentWindow, GuiWindow* mainWindow, ListaDiBottoni btn) {
    GuiImageData Textbox (keyboard_textbox_png, keyboard_textbox_png_size);
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

    char *encode; int offset=0;
    while (!NoInput(lista)) {
        if (inputType(lista)==BUTTON) {
            button=InsButton(button);
            GuiImage *ButtonImg=new GuiImage(&Button);
            button->btn=new GuiButton(ButtonImg->GetWidth(), ButtonImg->GetHeight());
            button->label=new GuiText((char*)lista->value.c_str(), 20, (GXColor){0, 0, 255, 255});
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
        else if (inputType(lista)==HIDDEN) {
            button=InsButton(button);
            button->btn=NULL;
        }
        else if (inputType(lista)==TEXT) {
            button=InsButton(button);
            button->refs=(Tag*)button;   //Dummy Assignment
            GuiImage *TextboxImg=new GuiImage(&Textbox);
            button->btn=new GuiButton(TextboxImg->GetWidth(), TextboxImg->GetHeight());
            if (lista->label!="noLabel") {
                button->label=new GuiText((char*)lista->label.c_str(), 20, (GXColor){0, 0, 255, 255});
                button->label->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
                button->label->SetPosition(0,-25);
                button->label->SetMaxWidth(TextboxImg->GetWidth()-5);
				button->label->SetScroll(SCROLL_HORIZONTAL);
                button->btn->SetLabel(button->label);
                offset+=25;
            }
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
            offset+=60;
        }
        else if (inputType(lista)==UNKNOWN) {
            button=InsButton(button);
            button->btn=NULL;
        }
        button->url.assign(lista->name);
        button->url.append("=");
        if (button->url.length()>1 && lista->value.length()>0) {
            encode=url_encode((char*)lista->value.c_str());
            button->url.append(encode);
            free(encode);
        }
        lista=lista->prox;
    }

    if (button) {
        while (!(userInput[0].wpad->btns_d & WPAD_BUTTON_B) && !choice) {
            usleep(100);
            for (bottone=button; !NoButton(bottone); bottone=bottone->prox) {
                if(bottone->btn && bottone->btn->GetState() == STATE_CLICKED) {
                    if(bottone->refs) {
                        size_t found=bottone->url.find("=")+1;
                        if (found!=string::npos) bottone->url.erase(found);
                        char save[MAXLEN]; memset(save, 0, sizeof(save));
                        OnScreenKeyboard(parentWindow, save, MAXLEN);
                        encode=url_encode(save);
                        bottone->url.append(encode);
                        free(encode);
                    }
                    else
                        choice=bottone;
                }
            }
        }
    }

    for (bottone=button; !NoButton(bottone); bottone=bottone->prox) {
        if (!bottone->btn || bottone->refs || bottone==choice) {
            if (bottone->url.length()>1)
            {
                btn->url.append(bottone->url);
                if (!NoButton(bottone->prox))
                    btn->url.append("&");
            }
        }
    }

    btn->url.append("\\");
    btn->url.append(btn->refs->form.method);

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

void HandleHtmlPad(int *offset, GuiButton *btnup, GuiButton *btndown, Indice ext, Indice Index, GuiWindow *mainWindow) {
    if (btnup->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_UP)) {
        usleep(15000); btnup->ResetState();
        if (!ext || ext->elem->GetYPosition()+25<screenheight)
            MoveText(1,+1,1, Index);
    }

    if (btndown->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN)) {
        usleep(15000); btndown->ResetState();
        if (!Index || Index->elem->GetYPosition()+Index->screenSize>25)
            MoveText(1,-1,1, Index);
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_LEFT) {
        if (*offset<screenwidth) {
            usleep(15000);
            MoveText(0,+1,1, Index);
            *offset+=25;
        }
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_RIGHT) {
        if (*offset>-screenwidth) {
            usleep(15000);
            MoveText(0,-1,1, Index);
            *offset-=25;
        }
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_1) {
        if (*offset) {
            usleep(15000);
            MoveText(0,-1,(*offset)/25, Index);
            *offset=0;
        }
    }
}

void HandleImgPad(GuiButton *btnup, GuiButton *btndown, GuiImage * image) {
    if (btnup->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_UP)) {
        if (userInput[0].wpad->btns_h & WPAD_BUTTON_UP) usleep(15000); btnup->ResetState();
        MoveWind(1,+1,1, image);
    }

    if (btndown->GetState() == STATE_CLICKED || (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN)) {
        if (userInput[0].wpad->btns_h & WPAD_BUTTON_DOWN) usleep(15000); btndown->ResetState();
        MoveWind(1,-1,1, image);
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_LEFT) {
        usleep(15000);
        MoveWind(0,+1,1, image);
    }

    if (hidden && userInput[0].wpad->btns_h & WPAD_BUTTON_RIGHT) {
        usleep(15000);
        MoveWind(0,-1,1, image);
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_PLUS) {
        usleep(15000);
        image->SetScale(image->GetScale()+image->GetScale()/100*5);
    }

    if (userInput[0].wpad->btns_h & WPAD_BUTTON_MINUS) {
        usleep(15000);
        image->SetScale(image->GetScale()-image->GetScale()/100*5);
    }
}

void MoveWind (int up, int dir, int line, GuiImage * image) {
    if (up)
        image->SetPosition(image->GetXPosition(), image->GetYPosition()+(line*25*dir));
    else
        image->SetPosition(image->GetXPosition()+(line*25*dir), image->GetYPosition());
}

void MoveText (int up, int dir, int line, Indice Index) {
    if (up) {
        for (; !NoIndex(Index); Index=Index->prox)
            Index->elem->SetPosition(Index->elem->GetXPosition(), Index->elem->GetYPosition()+(line*25*dir));
    }
    else {
        for (; !NoIndex(Index); Index=Index->prox)
            Index->elem->SetPosition(Index->elem->GetXPosition()+(line*25*dir), Index->elem->GetYPosition());
    }
}

int inputType(ListaDiInput lista) {
    if (lista->type=="submit" /*|| lista->type=="reset"*/) return BUTTON;
    if (lista->type=="hidden") return HIDDEN;
    if (lista->type=="text" || lista->type=="email" || lista->type=="password" || lista->type=="search" || lista->type.length()==0) return TEXT;
    return UNKNOWN;
}

int noSubmit(ListaDiBottoni btn) {
    ListaDiInput lista=btn->refs->form.input;
    for (;!NoInput(lista);lista=lista->prox) {
        if (lista->type=="submit")
            return 0;
    }
    return 1;
}

void showBar(GuiWindow *mainWindow, GuiWindow *parentWindow) {
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

void hideBar(GuiWindow *mainWindow, GuiWindow *parentWindow) {
    if (Toolbar) {
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

void HandleMenuBar(string *link, char* url, int *choice, int img, GuiWindow *mainWindow, GuiWindow *parentWindow) {
    if ((img ? userInput[0].wpad->btns_d & WPAD_BUTTON_1 : userInput[0].wpad->btns_d & WPAD_BUTTON_PLUS) && hidden) {
        showBar(mainWindow, parentWindow);
    }
    if ((img ? userInput[0].wpad->btns_d & WPAD_BUTTON_2 : userInput[0].wpad->btns_d & WPAD_BUTTON_MINUS) && !hidden) {
        hideBar(mainWindow, parentWindow);
    }

    if (Toolbar)
    {
        if (Toolbar->btnWWW->GetState() == STATE_CLICKED) {
            Toolbar->btnWWW->ResetState();
            *choice=1;
        }
        if (Toolbar->btnSave->GetState() == STATE_CLICKED) {
            Toolbar->btnSave->ResetState();
            *choice=2;
        }

        if (Toolbar->btnHome->GetState() == STATE_CLICKED) {
            Toolbar->btnHome->ResetState();
            if (strncmp(Settings.Homepage,"http",4))
                link->assign("http://");
            link->append(Settings.Homepage);
            *choice=1;
        }
        if (Toolbar->btnReload->GetState() == STATE_CLICKED) {
            Toolbar->btnReload->ResetState();
            link->assign(url);
            *choice=1;
        }

        if (Toolbar->btnBack->GetState() == STATE_CLICKED) {
            Toolbar->btnBack->ResetState();
            if (history->prec) {
                history=history->prec;
                link->assign(history->url);
                *choice=1;
            }
        }

        if (Toolbar->btnForward->GetState() == STATE_CLICKED) {
            Toolbar->btnForward->ResetState();
            if (history->prox) {
                history=history->prox;
                link->assign(history->url);
                *choice=1;
            }
        }
    }
}

int HandleMeta(Lista::iterator lista, string *link, struct block *html) {
    int choice = 0;
    if (lista->value[0].text=="refresh") {
        sleep(getTime(lista->attribute));
        link->assign(getUrl(&choice, lista->attribute));
    }
    return choice;
}

int getTime(string url) {
    int found, t=0;
    if ((found=url.find_first_of(";", 0))!=string::npos)
        t=atoi(url.substr(0,found).c_str());
    return t>0 ? t:1;
}

string getUrl(int *choice, string url) {
    int str,found;
    if ((str=url.find("url="))!=string::npos) {
        while ((found=url.find("&#39;"))!=string::npos)
            url.erase (found,5);
        *choice=1;
        return url.substr(str+4);
    }
    return "";
}
