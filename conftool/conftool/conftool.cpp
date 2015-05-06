#include "stdafx.h"
#include "conftool.h"

#include "qcustomplot.h"

enum { DRUMDU_BUFFER_SIZE  = 1024 * 2};


void readNextFrame(std::shared_ptr<Serial>& serial, std::function<void(unsigned long, std::array<byte, PAD_CNT>&)> cbFrame) {
AGAIN:
	auto available = serial->available();

	if(available < sizeof(byte) + sizeof(byte) + sizeof(byte) + sizeof(unsigned long) + sizeof(byte) * PAD_CNT + sizeof(byte)) {
		goto AGAIN;
	}

	byte sentinel;
	serial->readBytes(&sentinel, sizeof(sentinel));

	if(sentinel != 0xf0) {
		goto AGAIN;
	}

	byte manufacturer;
	serial->readBytes(&manufacturer, sizeof(manufacturer));

	byte msgType;
	serial->readBytes((byte*)&msgType, sizeof(msgType));

	unsigned long time1;
	serial->readBytes((byte*)&time1, sizeof(time1));

	std::array<byte, PAD_CNT> frame = { 0 };
	serial->readBytes(frame.data(), frame.size());

	cbFrame(time1, frame);

	serial->readBytes(&sentinel, sizeof(sentinel));
}



#include <iostream>
#include <fstream>


conftool::conftool(QWidget* parent)
	: QMainWindow(parent) {
	ui.setupUi(this);

	setWindowIcon(QIcon(":/drumduino/ship11.svg"));
	
	// Setup Pad Selector & buffers
	for(int pad = 0; pad < PAD_CNT; ++pad) {
		ui.cbPadSelector->addItem(QIcon(":/drumduino/ship11.svg"), QString("Pad %0").arg(pad + 1), pad);
		m_x[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);
		m_y[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);

		for (int i = 0; i < DRUMDU_BUFFER_SIZE; ++i) {
			m_x[pad][i] = i;
			m_y[pad][i] = (sin(i*0.03) + 1) * 128 / 2;
		}
	}

	connect(ui.cbPadSelector, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int idx) {
		m_pad = ui.cbPadSelector->itemData(idx).toInt();

		ui.lePadName->setPlaceholderText(QString("Pad %0").arg(m_pad + 1));
		ui.lePadName->setText(QString("Pad %0").arg(m_pad + 1));
	});


	// Setup Graph
	auto wgtPlot = new QCustomPlot(ui.centralWidget);
	wgtPlot->setAntialiasedElements(QCP::aeAll);
	wgtPlot->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	wgtPlot->setMinimumHeight(100);
	auto curve = wgtPlot->addGraph();
	wgtPlot->xAxis->setRange(0, DRUMDU_BUFFER_SIZE);
	wgtPlot->yAxis->setRange(0, 128);
	wgtPlot->xAxis->setTicks(false);
	ui.gbGraph->layout()->addWidget(wgtPlot);


	wgtPlot->setBackground(qApp->palette().button());
	QPen pen(qApp->palette().midlight().color());
	pen.setStyle(Qt::PenStyle::DotLine);
	wgtPlot->xAxis->grid()->setPen(pen);
	wgtPlot->yAxis->grid()->setPen(pen);
	wgtPlot->xAxis->setBasePen(qApp->palette().windowText().color());
	wgtPlot->yAxis->setBasePen(qApp->palette().windowText().color());
	wgtPlot->xAxis->setTickPen(qApp->palette().windowText().color());
	wgtPlot->yAxis->setTickPen(qApp->palette().windowText().color());



	// Install EventFilter to change styles
	for(auto dial : this->findChildren<QDial*>()) {
		dial->installEventFilter(this);
	}



	ui.cbPadSelector->setCurrentIndex(1);
	ui.cbPadSelector->setCurrentIndex(0);






#if 0


	m_serial = std::make_shared<Serial>(L"COM5", 2000000);

	std::thread th([this]() {


#if 0
		std::ofstream file;
		file.open("dump.txt");

		for(;;) {
			readNextFrame(_serial, [this/*, &file*/](unsigned long time, std::array<byte, PAD_CNT>& frame) {
				std::string str;

				str += QString::number(time).toStdString();
				str += ': ';

				for(int pad = 0; pad < PAD_CNT; ++pad) {
					str += QString::number(frame[pad]).toStdString();
					str += ', ';
				}

				file << str << "\n";
			});

		}

		file.close();
#else

		for(int pad = 0; pad < PAD_CNT; ++pad) {
			m_x[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);
			m_y[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);
		}

		for(;;) {
			readNextFrame(m_serial, [this/*, &file*/](unsigned long time, std::array<byte, PAD_CNT>& frame) {

				for(int pad = 0; pad < PAD_CNT; ++pad) {
					m_x[pad][m_frameCnt % m_x[pad].size()] = m_frameCnt % DRUMDU_BUFFER_SIZE;
					m_y[pad][m_frameCnt % m_y[pad].size()] = frame[pad];
				}

				++m_frameCnt;
			});

		}

#endif

	});

	th.detach();

#endif


	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this, wgtPlot]() {
		try {
			wgtPlot->graph(0)->setData(m_x[m_pad], m_y[m_pad]);
			wgtPlot->replot();
		}
		catch(...) {
			qDebug() << "eception";
		}
	});
	timer->start(100);
}

conftool::~conftool() {

}
