
//#include <Windows.h>
#include <DirectXMath.h>
#include <iostream>
#include <vector>

#include <QtWidgets/QApplication>
#include "VulnusEditor.h"

int main( int argc, char* argv[] )
{

	// open console when compiling for debugging
//#ifdef _DEBUG
	AllocConsole();
	AttachConsole( GetCurrentProcessId() );

#pragma warning(suppress: 6031)
	freopen( "CON", "w", stdout );					//redirect stdout to the created console
//#endif

	QApplication a(argc, argv);
	Vulnus::VulnusEditor w;
	w.show();
	return a.exec();
}
