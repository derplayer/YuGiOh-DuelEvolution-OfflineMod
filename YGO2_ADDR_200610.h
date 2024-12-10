#pragma once 
// ##### 2006-10 (also published as "061213_e" or "Ver.060419" (devs didn't change string since dev start) #####
// INFO: This build has no jmp stubs & RTTI data is limited. (different compiler settings)

// Basic
#define DEBUG_PARAMFLAG_200610				0x00773C68
#define API_GATE_ADDR_200610				0x00774498
#define HTTP_WEBSITE_LINK_200610			0x00774448

#define DEBUG_LOG_ADDR_200610				0x0050AC40	// default log, "Trunk Get" for example
#define DEBUG_LOG_ADDR_NETWORK_200610		0x00667FD0	// network msg log (real) - "get MSG()..."
#define DEBUG_LOG_PRINTF_200610				0x0059BF10	// aka va_print_stub - "MyRoom"
#define DEBUG_LOG_FPRINTF_200610			0x0			// NOT EXISTING
#define DEBUG_LOG_SPRINTF_200610			0x006CF9D4  // lib impl.
#define DEBUG_LOG_VERB_200610				0x00405880	// verbose log(?) - but seems to do more, so dont use

// Extra
#define DEBUG_TEXTSTRING_200610				0x007768E8	// END: 00776C9D
#define DUEL_DECK_PREPARE_200610			0x0064E340
#define DUEL_START_200610					0x00675720

// Deck & Kaban
#define PLAYER_DECK_ADDR_200610				0x007432A0
#define PLAYER_DECK_PTR_200610				0x00BDDE94 // Pointer! the struct after this has some unknown metadata? (2bytes) just skip for now
#define NPC_DECK_PTR_200610					0x00BED9A8
#define PLAYER_KABAN_PTR_200610				0x00BDC960
// Dummy Deck (MODE1)
#define DUMMY_DECK_AMOUNT_200610			0x0074329C // default is 40 (int)
#define DUMMY_DECK_PTR_200610				0x007432A0 // cards are in 4 byte format here, ends with 0x0100(?)
#define DUMMY_DECK_PTR_REAL_200610			0x0064E379 // 4 byte DWORD (used by duel_deck_prepare)
#define DUMMY_DECK_EXTRA_AMOUNT_200610		0x00743340 // default is 1 (int)
#define DUMMY_DECK_EXTRA_PTR_200610			0x00743344

// Labels
#define SCN_CARDSWAP_BTN_KSAVE_200610		0x00775F08
#define SCN_CARDSWAP_BTN_DSAVE_200610		0x00775EFC
#define SCN_CARDSWAP_BTN_RETURN_200610		0x00775EE8
#define SCN_CARDSWAP_BTN_RETURN_PTR_200610	0x00509408	//+16 for second

// Scene IDs
#define SCN_MAINLOOP_200610					0x00596340
