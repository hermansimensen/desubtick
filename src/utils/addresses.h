#pragma once
#include "platform.h"
#include "stdint.h"
#include "utils/module.h"
#include "utlstring.h"


struct Signature {
	const char *data = nullptr;
	size_t length = 0;

	template<size_t N>
	Signature(const char(&str)[N]) {
		data = str;
		length = N - 1;
	}
};
#define DECLARE_SIG(name, sig) inline const Signature name = Signature(sig);

namespace modules
{
	inline CModule *server;
	void Initialize();
}

namespace sigs
{
#ifdef _WIN32
	DECLARE_SIG(ProcessUsercmds, "\x48\x8B\xC4\x44\x88\x48\x20\x44\x89\x40\x18\x48\x89\x50\x10\x53");
	DECLARE_SIG(ProcessMovement, "\x40\x56\x57\x48\x81\xEC\xA8\x00\x00\x00\x4C\x8B\x49\x30");
#else
	DECLARE_SIG(ProcessUsercmds, "\x55\x48\x89\xE5\x41\x57\x41\x56\x41\x89\xD6\x41\x55\x49\x89\xFD\x41\x54\x53");
	DECLARE_SIG(ProcessMovement, "\x55\x48\x89\xE5\x41\x57\x41\x56\x41\x55\x41\x54\x49\x89\xFC\x53\x48\x83\xEC\x2A\x48\x8B\x7F\x2A\x8B");
#endif
}