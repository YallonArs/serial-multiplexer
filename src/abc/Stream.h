#pragma once

#include "utils.h"

class Stream {
private:
	Stream(const Stream &)			  = delete;
	Stream &operator=(const Stream &) = delete;

public:
	Stream() = default;
	virtual ~Stream() = default;

	virtual ssize_t read(byte *buffer, const size_t length) = 0;
};

class WriteableStream : public Stream {
public:
	virtual void write(const byte *data, const size_t length) = 0;
};
