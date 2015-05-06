#include "stdafx.h"
#include "serial.h"





Serial::Serial(const wchar_t* portName, DWORD baudRate /*= 115200*/)
	: _hSerial(INVALID_HANDLE_VALUE)
{
	//Try to connect to the given port throuh CreateFile
	_hSerial = ::CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//Check if the connection was successfull
	if(_hSerial == INVALID_HANDLE_VALUE) {
		throw std::exception("Serial connection failed to create");
	}

	//If connected we try to set the comm parameters
	DCB dcbSerialParams = {0};

	//Try to get the current
	if(!::GetCommState(_hSerial, &dcbSerialParams)) {
		throw std::exception("failed to get serial parameters");
	}

	//Define serial connection parameters for the arduino board
	dcbSerialParams.BaudRate = baudRate;
	dcbSerialParams.fBinary = TRUE;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.fNull = FALSE;

	//Set the parameters and check for their proper application
	if(!::SetCommState(_hSerial, &dcbSerialParams)) {
		throw std::exception("Could not set Serial Port parameters");
	}

	//We wait 2s as the arduino board will be reseting
	::Sleep(ARDUINO_WAIT_TIME);
}


Serial::~Serial(void)
{
	if(_hSerial != INVALID_HANDLE_VALUE) {
		::CloseHandle(_hSerial);
		_hSerial = INVALID_HANDLE_VALUE;
	}
}

size_t Serial::available()
{
	::ClearCommError(_hSerial, &_errors, &_status);
	return _status.cbInQue;
}

size_t Serial::readBytes(byte* data, size_t size)
{
	auto len = std::min(size, available());

	DWORD bytesRead;
	::ReadFile(_hSerial, data, len, &bytesRead, NULL);

	return bytesRead;
}

size_t Serial::write(const byte* data, size_t size)
{
	DWORD bytesSend;

	if(!WriteFile(_hSerial, (void*)data, size, &bytesSend, 0)) {
		ClearCommError(_hSerial, &_errors, &_status);
	}

	return bytesSend;
}