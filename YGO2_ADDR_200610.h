#pragma once 
// ##### 2006-10 (also published as "061213_e" or "Ver.060419" (devs didn't change string since dev start) #####
// INFO: This build has no jmp stubs & RTTI data is limited. (different compiler settings)

// Basic
#define DEBUG_PARAMFLAG_200610				0x00773C68
#define API_GATE_ADDR_200610				0x00774498
#define HTTP_WEBSITE_LINK_200610			0x00774448
#define PLAYER_DECK_ADDR_200610				0x007432A0
#define PLAYER_DECK_PTR_200610				0x00BDDE94 // Pointer! the struct after this has some unknown metadata? (2bytes) just skip for now
#define PLAYER_KABAN_PTR_200610				0x00BDC960

#define DEBUG_LOG_ADDR_200610				0x0050AC40	// default log, "Trunk Get" for example
#define DEBUG_LOG_ADDR_NETWORK_200610		0x00667FD0	// network msg log (real) - "get MSG()..."
#define DEBUG_LOG_PRINTF_200610				0x0059BF10	// aka va_print_stub - "MyRoom"
#define DEBUG_LOG_FPRINTF_200610			0x0			// NOT EXISTING
#define DEBUG_LOG_SPRINTF_200610			0x006CF9D4  // lib impl.
#define DEBUG_LOG_VERB_200610				0x00405880	// verbose log(?) - but seems to do more, so dont use

// Extra
#define DEBUG_TEXTSTRING_200610				0x007768E8	// END: 00776C9D

// Scene IDs
#define SCN_MAINLOOP_200610					0x00596340
