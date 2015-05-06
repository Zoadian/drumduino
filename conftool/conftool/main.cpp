#include "stdafx.h"
#include "conftool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#if 0
	QPalette p;
	//p = qApp->palette();

	p.setColor(QPalette::Base, QColor(53, 53, 53)); // lineedit & progress background
	p.setColor(QPalette::Window, QColor(53, 53, 53)); //window & combobox background
	p.setColor(QPalette::Button, QColor(53, 53, 53)); //tab & button background (light)
	p.setColor(QPalette::Text, QColor(255, 255, 255)); //combobox text
	p.setColor(QPalette::ButtonText, QColor(255, 255, 255)); // button text
	p.setColor(QPalette::WindowText, QColor(0xff6600)); // tab and onWindow text

	p.setColor(QPalette::BrightText, QColor(0xff0000)); // ???
	p.setColor(QPalette::Shadow, QColor(Qt::black)); // ???
	p.setColor(QPalette::Dark, QColor(0xff6600)); // qdial notches
	p.setColor(QPalette::Mid, QColor(43, 43, 43)); // ???
	p.setColor(QPalette::Midlight, QColor(73, 73, 73)); // ???
	p.setColor(QPalette::Light, QColor(93, 93, 93)); // ???

	p.setColor(QPalette::NoRole, QColor(0x00ff00)); // ???
	p.setColor(QPalette::AlternateBase, QColor(0x00ff00)); // ???
	p.setColor(QPalette::ToolTipBase, QColor(0x00ff00)); // ???
	p.setColor(QPalette::Link, QColor(0x00ff00)); // ???
	p.setColor(QPalette::LinkVisited, QColor(0x00ff00)); // ???

	p.setColor(QPalette::HighlightedText, QColor(0xffffff)); //current selection text of combobox
	p.setColor(QPalette::Highlight, QColor(0xff9966)); //combobox current selection background, progress chunk background
	qApp->setPalette(p);
#endif

	conftool w;
	w.show();
	return a.exec();
}
