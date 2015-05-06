#include <QtWidgets>

#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <array>
#include <algorithm>

#define PORT_CNT 6
#define CHAN_CNT 8


using byte = unsigned char;

enum { PAD_CNT = 6 * 8 };