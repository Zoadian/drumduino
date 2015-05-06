#pragma once


class Serial
{
	enum { ARDUINO_WAIT_TIME = 2000 };

private:
	HANDLE _hSerial;
	COMSTAT _status;
	DWORD _errors;

public:
	Serial(const wchar_t* portName, DWORD baudRate = 115200);
	~Serial(void);

public:
	size_t available();
	size_t readBytes(byte* data, size_t size);
	size_t write(const byte* data, size_t size);
};

