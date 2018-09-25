#include "../../include/onion-debug.h"

fastDebuger::fastDebuger() :verbosityLevel(-1) {}
fastDebuger::fastDebuger(int verbosityLevel) : verbosityLevel(verbosityLevel) {}
fastDebuger::fastDebuger(const fastDebuger & src) {
	this->verbosityLevel = src.getVerbosity();
}
fastDebuger::~fastDebuger() {}

inline void fastDebuger::setVerbosity (int verbosityLevel) {
	this->verbosityLevel = verbosityLevel;
}

inline int fastDebuger::getVerbosity () const {
	return verbosityLevel;
}

void fastDebuger::print (int severity, const char* msg, ...) {
	va_list 	argptr;	
	if (verbosityLevel >= severity) {
		va_start(argptr, msg);
		vprintf(msg, argptr);
		va_end(argptr);
	}
}

void fastDebuger::operator=(const fastDebuger & rhs) {
	this->verbosityLevel = rhs.getVerbosity();
}

bool fastDebuger::operator==(const fastDebuger & rhs) {
	return this->verbosityLevel == rhs.getVerbosity();
}