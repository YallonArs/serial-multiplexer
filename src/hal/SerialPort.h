#pragma once

#include <asm/termbits.h>
#include <stdint.h>

#include "utils.h"
#include "abc/Stream.h"

class SerialPort : public WriteableStream {
public:
	enum class Mode {
		Terminal,
		Binary
	};

	enum class Parity {
		None,
		Even,
		Odd
	};

	enum class DataBits {
		Five  = 5,
		Six	  = 6,
		Seven = 7,
		Eight = 8
	};

	enum class StopBits {
		One,
		Two
	};

	struct Settings {
		string path;
		uint64_t speed;
		Parity parity	   = Parity::None;
		DataBits data_bits = DataBits::Eight;
		StopBits stop_bits = StopBits::One;
		bool rtscts		   = false;
	};

private:
	int fd;
	termios2 tty;
	const Settings *settings = nullptr;
	Mode mode;

	void setup();
	void applySettings();

public:
	SerialPort(const Settings &settings, Mode mode = Mode::Binary);
	~SerialPort();

	void open();
	void close();
	bool isOpen() const;

	void setMode(Mode mode);

	void write(const byte *data, const size_t length);
	void read(byte *buffer, const size_t length, size_t &bytesRead);
};
