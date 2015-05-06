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

	_serial = std::make_shared<Serial>(L"COM5", 2000000);

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
		for (int pad = 0; pad < PAD_CNT; ++pad) {
			m_x[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);
			m_y[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);
		}

		for(;;) {
			readNextFrame(_serial, [this/*, &file*/](unsigned long time, std::array<byte, PAD_CNT>& frame) {

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



	for (int pad = 0; pad < PAD_CNT; ++pad) {
		m_plots[pad] = new QCustomPlot(ui.centralWidget);

		m_plots[pad]->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		m_plots[pad]->setMinimumHeight(100);
		auto curve = m_plots[pad]->addGraph();
		m_plots[pad]->xAxis->setRange(0, DRUMDU_BUFFER_SIZE);
		m_plots[pad]->yAxis->setRange(-1, 128);

		ui.centralWidget->layout()->addWidget(m_plots[pad]);
	}


	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this]() {
		try {
			for (int pad = 0; pad < PAD_CNT; ++pad) {
				//m_plots[pad]->xAxis->setRange(m_x[pad][(m_frameCnt + 1) % DRUMDU_BUFFER_SIZE], m_x[pad][(m_frameCnt) % DRUMDU_BUFFER_SIZE]);

				m_plots[pad]->graph(0)->setData(m_x[pad], m_y[pad]);
				m_plots[pad]->replot();
			}
		}
		catch(...) {
			qDebug() << "eception";
		}
	});
	timer->start(100);






#if 0

	for(int pad = 0; pad < 1; ++pad) {
		m_x[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);
		m_y[pad] = QVector<qreal>(DRUMDU_BUFFER_SIZE);
	}


	for(int pad = 0; pad < 1; ++pad) {
		m_plots[pad] = new QCustomPlot(ui.centralWidget);

		m_plots[pad]->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
		m_plots[pad]->setMinimumHeight(100);
		auto curve = m_plots[pad]->addGraph();
		m_plots[pad]->xAxis->setRange(0, 127);
		m_plots[pad]->yAxis->setRange(-10, 256);

		ui.centralWidget->layout()->addWidget(m_plots[pad]);


	}


	_serial = std::make_shared<Serial>(L"COM5", 2000000);


	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this]() {
		try {
			for(int i = 0; i < DRUMDU_BUFFER_SIZE; ++i) {
				readNextFrame(_serial, [this](unsigned long time, std::array<byte, PAD_CNT>& frame) {


					for(int pad = 0; pad < 1; ++pad) {
						m_x[pad][m_frameCnt % DRUMDU_BUFFER_SIZE] = time;
						m_y[pad][m_frameCnt % DRUMDU_BUFFER_SIZE] = frame[pad];


					}


					++m_frameCnt;


				});
			}

			for(int pad = 0; pad < 1; ++pad) {
				m_plots[pad]->xAxis->setRange(m_x[pad][(m_frameCnt + 1) % DRUMDU_BUFFER_SIZE], m_x[pad][(m_frameCnt) % DRUMDU_BUFFER_SIZE]);

				m_plots[pad]->graph(0)->setData(m_x[pad], m_y[pad]);
				m_plots[pad]->replot();
			}
		}
		catch(...) {
			qDebug() << "eception";
		}
	});
	timer->start(10);

#endif
}

conftool::~conftool() {

}
