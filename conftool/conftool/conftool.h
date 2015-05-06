#ifndef CONFTOOL_H
#define CONFTOOL_H

#include <QtWidgets/QMainWindow>
#include "ui_conftool.h"

#include <serial.h>
#include "qcustomplot.h"

class conftool : public QMainWindow {
	Q_OBJECT


	std::shared_ptr<Serial> m_serial;

	std::array<QCustomPlot*, PAD_CNT> m_plots;
	std::array<QVector<qreal>, PAD_CNT> m_x;
	std::array<QVector<qreal>, PAD_CNT> m_y;
	int m_frameCnt = 0;

	int m_pad = 0;

public:
	conftool(QWidget* parent = 0);
	~conftool();

private:
	Ui::drumduino ui;

private:
	bool eventFilter(QObject* o, QEvent* e) {
		auto dial = qobject_cast<QDial*>(o);

		if(dial && e->type() == QEvent::Paint) {
			QPaintEvent* paintEvent = static_cast<QPaintEvent*>(e);

			QStylePainter p(dial);
			QStyleOptionSlider option;

			option.initFrom(dial);
			option.minimum = dial->minimum();
			option.maximum = dial->maximum();
			option.sliderPosition = dial->value();
			option.sliderValue = dial->value();
			option.singleStep = dial->singleStep();
			option.pageStep = dial->pageStep();
			option.upsideDown = !dial->invertedAppearance();
			option.notchTarget = 0;
			option.dialWrapping = dial->wrapping();
			option.subControls = QStyle::SC_All;
			option.activeSubControls = QStyle::SC_None;

			//option.subControls &= ~QStyle::SC_DialTickmarks;
			//option.tickPosition = QSlider::TicksAbove;
			option.tickPosition = QSlider::NoTicks;

			option.tickInterval = dial->notchSize();
			/*
			p.setPen(qApp->palette().buttonText().color());
			p.setBrush(qApp->palette().buttonText().color());*/
			p.drawComplexControl(QStyle::CC_Dial, option);
			p.setPen(qApp->palette().buttonText().color());
			p.drawText(dial->rect(), Qt::AlignCenter, QString::number(dial->value()));
			return true;
		}

		return QMainWindow::eventFilter(o, e);
	}
};

#endif // CONFTOOL_H
