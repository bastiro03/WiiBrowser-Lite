/****************************************************************************
 * WiiBrowser
 *
 * gave92 2012
 *
 * document.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

/**
 * Constructor for the Document class.
 */

Document::Document()
{
    document = NULL;
}

/**
 * Destructor for the Document class.
 */

Document::~Document()
{
    document = NULL;
}

void Document::SetOffset(int *offset)
{
    document = offset;
}

int Document::GetOffset()
{
    return (document ? (*document+40) : 0);
}
