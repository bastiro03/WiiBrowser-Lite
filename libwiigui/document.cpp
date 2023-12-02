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
	document = nullptr;
}

/**
 * Destructor for the Document class.
 */

Document::~Document()
{
	document = nullptr;
}

void Document::SetOffset(int* offset)
{
	document = offset;
}

int Document::GetOffset()
{
	return (document ? (*document + 40) : 0);
}
