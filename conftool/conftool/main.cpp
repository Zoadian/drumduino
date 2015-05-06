#include "stdafx.h"
#include "conftool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	conftool w;
	w.show();
	return a.exec();
}
