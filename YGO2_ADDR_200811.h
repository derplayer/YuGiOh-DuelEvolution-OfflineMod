#pragma once 
// ##### 2008-11 ########

// Basic
#define DEBUG_PARAMFLAG_200811				0x00C898E8

#define DEBUG_LOG_ADDR_200811				0x00403EF9	// default log, "Trunk Get" for example
#define DEBUG_LOG_ADDR_NETWORK_200811		0x00410FA0	// network msg log (stub) - "get MSG()..."
#define DEBUG_LOG_PRINTF_200811				0x0040818E	// aka va_print_stub - "illegal PartIdx"
#define DEBUG_LOG_FPRINTF_200811			0x00B0A523  // lib impl.
#define DEBUG_LOG_SPRINTF_200811			0x00B0B654  // lib impl.
#define DEBUG_LOG_VERB_200811				0x00409953	// verbose log(?) - but seems to do more, so dont use	

// Extra
#define SCN_MAINLOOP_200811					0x0061DC30

#define SCN_JANKEN_200811					0x00408C10
#define SCN_JANKEN_IMPL_200811				0x007C3600

#define DUEL_ADDR_200811					0x00405637
#define DUEL_START_200811					0x00794D90
#define DEBUG_TEXTSTRING_200811				0x00C95D04	// OLD: 0x00C95D10
#define DEBUG_HTTPTESTUPLOADCALL_200811		0x00411F28	// jumptable call