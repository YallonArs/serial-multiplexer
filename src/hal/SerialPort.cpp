#include "SerialPort.h"

#include <stdexcept>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <format>
#include <system_error>

SerialPort::SerialPort(const Settings &settings, Mode mode) : fd(-1), settings(&settings), mode(mode) {}

SerialPort::~SerialPort() {
	close();
}

void SerialPort::setup() {
	// 1. speed
	tty.c_cflag &= ~CBAUD;
	tty.c_cflag |= BOTHER;

	tty.c_ispeed = settings->speed;
	tty.c_ospeed = settings->speed;

	// 2. parity
	if (settings->parity == Parity::None) {
		tty.c_cflag &= ~PARENB;
	} else {
		tty.c_cflag |= PARENB;

		if (settings->parity == Parity::Even) {
			tty.c_cflag &= ~PARODD;
		} else {
			tty.c_cflag |= PARODD;
		}
	}

	// 3. stop bits
	if (settings->stop_bits == StopBits::One) {
		tty.c_cflag &= ~CSTOPB;
	} else {
		tty.c_cflag |= CSTOPB;
	}

	// 4. data bits
	tty.c_cflag &= ~CSIZE;
	switch (settings->data_bits) {
		case DataBits::Five:
			tty.c_cflag |= CS5;
			break;
		case DataBits::Six:
			tty.c_cflag |= CS6;
			break;
		case DataBits::Seven:
			tty.c_cflag |= CS7;
			break;
		case DataBits::Eight:
			tty.c_cflag |= CS8;
			break;
	}

	// 5. flow control
	if (settings->rtscts) {
		tty.c_cflag |= CRTSCTS;
	} else {
		tty.c_cflag &= ~CRTSCTS;
	}

	// 6. misc
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off software flow control
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
	tty.c_oflag &= ~(OPOST | ONLCR);

	setMode(mode);

	applySettings();
}

void SerialPort::applySettings() {
	if (ioctl(fd, TCSETS2, &tty) < 0) {
		throw std::runtime_error(std::format("Error applying port settings (TCSETS2): {}", strerror(errno)));
	}
}

void SerialPort::open() {
	fd = ::open(settings->path.c_str(), O_RDWR);

	if (fd < 0) {
		throw std::runtime_error("cannot open serial port");
	}

	struct termios2 tty;
	if (ioctl(fd, TCGETS2, &tty) < 0) {
		throw std::runtime_error(std::format("Error reading port settings (TCGETS2): {}", strerror(errno)));
	}
	this->tty = tty;
	setup();
}

bool SerialPort::isOpen() const {
	return fd > 0;
}

void SerialPort::setMode(Mode mode) {
	if (!isOpen()) {
		return;
	}

	this->mode = mode;
	if (mode == Mode::Binary) {
		tty.c_cc[VMIN]  = 1;
		tty.c_cc[VTIME] = 0;
	} else {
		tty.c_cc[VMIN]  = 64;
		tty.c_cc[VTIME] = 1;
	}

	applySettings();
}

void SerialPort::close() {
	if (fd > 0) {
		::close(fd);
		fd = -1;
	}
}

void SerialPort::write(const byte *data, const size_t length) {
	if (!isOpen()) {
		throw std::runtime_error("Serial port is not open");
	}

	ssize_t bytes_written = ::write(fd, data, length);
	if (bytes_written < 0) {
		throw std::runtime_error(std::format("Error writing to serial port: {}", strerror(errno)));
	}
}

void SerialPort::read(byte *buffer, const size_t length, size_t &bytesRead) {
	if (!isOpen()) {
		throw std::runtime_error("Serial port is not open");
	}

	ssize_t result = ::read(fd, buffer, length);
	if (result < 0) {
		throw std::runtime_error(std::format("Error reading from serial port: {}", strerror(errno)));
	}

	bytesRead = static_cast<size_t>(result);
}
