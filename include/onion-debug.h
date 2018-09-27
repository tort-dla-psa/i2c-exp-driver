#ifndef _ONION_DEBUG_H_
#define _ONION_DEBUG_H_

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define ONION_SEVERITY_FATAL			(-1)
#define ONION_SEVERITY_INFO				(0)
#define ONION_SEVERITY_DEBUG			(1)
#define ONION_SEVERITY_DEBUG_EXTRA		(2)

#define ONION_VERBOSITY_NONE			(ONION_SEVERITY_FATAL)
#define ONION_VERBOSITY_NORMAL			(ONION_SEVERITY_INFO)
#define ONION_VERBOSITY_VERBOSE			(ONION_SEVERITY_DEBUG)
#define ONION_VERBOSITY_EXTRA_VERBOSE	(ONION_SEVERITY_DEBUG_EXTRA)

class fastDebuger {
	int 	verbosityLevel;
public:
	fastDebuger();
	fastDebuger(int verbosityLevel);
	fastDebuger(const fastDebuger &src);
	~fastDebuger();
	// debug functions
	inline void 	setVerbosity(int verbosityLevel);
	inline int 	getVerbosity() const;
	void 	print(int severity, const char* msg, ...);
	void	operator = (const fastDebuger &rhs);
	bool	operator == (const fastDebuger &rhs);
};
#endif // _ONION_DEBUG_H_ 
