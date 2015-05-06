#ifndef CONFTOOL_H
#define CONFTOOL_H

#include <QtWidgets/QMainWindow>
#include "ui_conftool.h"

#include <serial.h>
#include "qcustomplot.h"

class conftool : public QMainWindow {
	Q_OBJECT


	std::shared_ptr<Serial> _serial;

	std::array<QCustomPlot*, PAD_CNT> m_plots;
	std::array<QVector<qreal>, PAD_CNT> m_x;
	std::array<QVector<qreal>, PAD_CNT> m_y;
	int m_frameCnt = 0;

public:
	conftool(QWidget* parent = 0);
	~conftool();

private:
	Ui::conftoolClass ui;
};

#endif // CONFTOOL_H
