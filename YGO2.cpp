#pragma warning(disable:26495)

#include "YGO2.h"
#include <filesystem>
#define YGO2_MODVERSION				11
static int ygoVer = 0;
static std::string ygoVerStr = "unk";

// ### Trampoline returns
static YGO2::hooktype_fprintf fprintfHook_Return = nullptr;
static YGO2::hooktype_sprintf sprintfHook_Return = nullptr;
static YGO2::hooktype_debuglog_file debuglogFileHook_Return = nullptr;
static YGO2::hooktype_debuglog_verb debuglogVerbHook_Return = nullptr;

static YGO2::hooktype_scn_mainloop sceneMainLoopHook_Return = nullptr;
static YGO2::hooktype_duelstart duelStartHook_Return = nullptr;
static YGO2::hooktype_dueldeck duelDeckHook_Return = nullptr;
static YGO2::hooktype_sub_5ADCB0 sub_5ADCB0_Return = nullptr;
static YGO2::hooktype_sub_64F8E0 sub_64F8E0_Return = nullptr;

static int lastSceneId = -1;

DeckData deckData;
DeckData deckDataNPC;

// Helper function to check if an address is within the stack range
bool IsStackAddress(void* address)
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(address, &mbi, sizeof(mbi));
	return mbi.Type == MEM_PRIVATE && mbi.AllocationProtect == PAGE_READWRITE && mbi.State == MEM_COMMIT;
}

// Function to get the description of the exception code
const char* GetExceptionCodeDescription(DWORD code)
{
	switch (code)
	{
	case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
	case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
	case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
	case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
	case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
	case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
	case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
	case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
	case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
	case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
	case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
	case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
	case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
	case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
	case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
	case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
	case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
	default: return "UNKNOWN_EXCEPTION";
	}
}

// Function that handles exceptions
LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
	// Build a detailed log message
	std::stringstream logMessage;
	DWORD exceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
	void* exceptionAddress = ExceptionInfo->ExceptionRecord->ExceptionAddress;

	logMessage << "Exception code: " << std::hex << exceptionCode
		<< " (" << GetExceptionCodeDescription(exceptionCode) << ")\n";
	logMessage << "Exception address: " << exceptionAddress << "\n";
	logMessage << "Number of parameters: " << ExceptionInfo->ExceptionRecord->NumberParameters << "\n";

	// Determine if the exception occurred on the stack or in main memory
	if (IsStackAddress(exceptionAddress))
	{
		logMessage << "Exception occurred on the stack.\n";
	}
	else
	{
		logMessage << "Exception occurred in main memory.\n";
	}

	// Log each exception information parameter
	for (DWORD i = 0; i < ExceptionInfo->ExceptionRecord->NumberParameters; ++i)
	{
		logMessage << "Parameter[" << i << "]: "
			<< ExceptionInfo->ExceptionRecord->ExceptionInformation[i] << "\n";
	}

	// Convert log message to string
	std::string logStr = logMessage.str();

	// Write to the log file
	log_write(YGO2_LOGFILE_NAME, logStr.c_str(), false);

	// For other exceptions, continue execution
	return EXCEPTION_EXECUTE_HANDLER;
}


// ### Implementations
void YGO2::EmptyStub() {
	return;
}

void __fastcall YGO2::EmptyStubFast()
{
	return;
}

void YGO2::debug_log(char* msg, ...)
{
	// INFO: at duel, sometimes there is garbage in log call? (atleast in YGO2_2006_10, maybe due to diff. call conv "__cdecl")
	if (msg == NULL || msg == (char*)0x1 || msg == (char*)0x2) {
		printf("Invalid message in stub memory register leftover detected.\n");
		return;
	}

	char buffer[512];
	sprintf(buffer, "debug log: ");

	va_list args;
	va_start(args, msg);
	vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), msg, args);
	va_end(args);

	// Check if buffer ends with a newline (logs have always newline) - filters out garbage logs
	if (buffer[strlen(buffer) - 1] == '\n') {
		printf(buffer);

		if (!strstr(buffer, "\n")) {
			printf("\n");
		}

		log_write(YGO2_LOGFILE_NAME, buffer, false);
	}

	return;
}

void __fastcall YGO2::network_response_log_stub(const char* a)
{
	return;
}

char* YGO2::debug_log_buffer_ret(char* msg, ...)
{
	char buffer[512];

	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);

	printf(buffer);
	if (!strstr(buffer, "\n")) {
		printf("\n");
	}

	log_write(YGO2_LOGFILE_NAME, buffer, true);
	return buffer;
}

int YGO2::fprint_reimpl(FILE* const Stream, const char* const Format, char* bootleg_va)
{
	// TODO: this is broken. so i think it should be done like sprintf_reimpl... but its never called in code (at least debug mode sooo...)
	char bufferA[512];
	sprintf(bufferA, "fprintf HOOK: ");
	sprintf(bufferA + strlen(bufferA), Format, bootleg_va);
	log_write(YGO2_LOGFILE_NAME, bufferA, true);

	return fprintfHook_Return(Stream, Format, bootleg_va);
}

int YGO2::sprintf_reimpl(char* const Buffer, const char* const Format, ...)
{
	va_list args;
	va_start(args, Format);

	char bufferA[256];
	snprintf(bufferA, sizeof(bufferA), "sprintf HOOK: %s", Format);

	// Create a separate buffer for the formatted string
	char formattedString[256];
	vsnprintf(formattedString, sizeof(formattedString), Format, args);
	strncat(bufferA, formattedString, sizeof(bufferA) - strlen(bufferA) - 1);

	// MOD: Change debug duel playernames
	if (strcmp(formattedString, "KONAMI_PLAYER_001") == 0) {
		char* modPlayer = strdup("Player");
		strncpy(formattedString, modPlayer, strlen(modPlayer) + 1);
	}
	else if (strcmp(formattedString, "KONAMI_PLAYER_002") == 0) {
		char* cpu = strdup("CPU (Generic)");
		strncpy(formattedString, cpu, strlen(cpu) + 1);
	}

	log_write(YGO2_LOGFILE_NAME, formattedString, true);

	// Pass the pre-formatted string to sprintfHook_Return to parse it again dawg (HOLY HAX)
	int result = sprintfHook_Return(Buffer, "%s", formattedString);

	va_end(args);

	return result;
}

// INFO: bootleg thiscall to fastcall, x is "thiscall" ecx register
int __fastcall  YGO2::debug_log_verb(void* _this, int a, const char* b, unsigned int c)
{
	// do not print empty msgs
	if (strlen(b) <= 1) return debuglogVerbHook_Return(_this, a, b, c);

	// Spam decrease
	if (strstr(b, "Shape") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "Rotate") != NULL)        return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "Scale") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "Flags") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "Depth") != NULL)         return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "CTextElm") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "Xmin") != NULL)          return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "Ymin") != NULL)          return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "FileName") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "CharacterID") != NULL)   return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "Sprite") != NULL)        return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "MultTerm") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "unknown") != NULL)       return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "GameLoop") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "GameStep") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "_btn") != NULL)          return debuglogVerbHook_Return(_this, a, b, c);
	if (strstr(b, "ﾌﾌﾌﾌﾌﾌﾌﾌ") != NULL)      return debuglogVerbHook_Return(_this, a, b, c);

	char bufferA[512];
	sprintf(bufferA, "verbose HOOK: ");
	sprintf(bufferA + strlen(bufferA), b, a);

	log_write(YGO2_LOGFILE_V_NAME, bufferA, true);

	return debuglogVerbHook_Return(_this, a, b, c);
}

// Define the hook function
void* __cdecl debug_log_file(FILE* Stream, int arg1, char arg2) {
	// Add your logging or modification code here
	printf("debug_log_file called with args: %d, %c\n", arg1, arg2);

	// Call the original function
	return debuglogFileHook_Return(Stream, arg1, arg2);
}

void applyPlayerDeckToMemory()
{
	// Setups the deck (x cards, x side deck cards, x extra deck card) - 4 bytes per param
	BYTE playerDeckHeader[12] = { 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	playerDeckHeader[0] = deckData.mainCardsCount;
	playerDeckHeader[4] = deckData.sideCardsCount;
	playerDeckHeader[8] = deckData.extraCardsCount;

	DWORD sizePlayer = sizeof(playerDeckHeader) + sizeof(deckData.buffer);
	PrintMemoryVariable(NPC_DECK_PTR_200610, sizePlayer, "\n0xPLAYER_DECK_PRE\n");
	ApplyBytesDirect(PLAYER_DECK_PTR_200610, playerDeckHeader, sizeof(playerDeckHeader));
	ApplyBytesDirect(PLAYER_DECK_PTR_200610 + sizeof(playerDeckHeader), deckData.buffer, sizeof(deckData.buffer));
	PrintMemoryVariable(NPC_DECK_PTR_200610, sizePlayer, "\n0xPLAYER_DECK_POST\n");
}

void applyNPCDeckToMemory()
{
	// Setups the deck (x cards, x side deck cards, x extra deck card) - 4 bytes per param
	BYTE npcDeckHeader[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	npcDeckHeader[0] = deckDataNPC.mainCardsCount;
	npcDeckHeader[4] = deckDataNPC.sideCardsCount;
	npcDeckHeader[8] = deckDataNPC.extraCardsCount;

	DWORD size = sizeof(npcDeckHeader) + sizeof(deckDataNPC.buffer);
	PrintMemoryVariable(NPC_DECK_PTR_200610, size, "\n0xPLAYER_NPC_EDITOR_PRE\n");
	ApplyBytesDirect(NPC_DECK_PTR_200610, npcDeckHeader, sizeof(npcDeckHeader));
	ApplyBytesDirect(NPC_DECK_PTR_200610 + sizeof(npcDeckHeader), deckDataNPC.buffer, sizeof(deckDataNPC.buffer));
	PrintMemoryVariable(NPC_DECK_PTR_200610, size, "\n0xPLAYER_NPC_EDITOR_POST\n");
	return;
}

int(__thiscall* scene_mainloop)(void*, int);
int __fastcall YGO2::scene_mainloop_reimpl(void* _this, void* x, int sceneNumber) {
	// here we are calling the trampoline & executing the code
	// of the target function, while also being able to run our own code afterwards/before

	// TEST 1 (player stack deck)
	// Get the handle to the current process
	HANDLE hProcess = GetCurrentProcess();

	// Read the pointer from the given offset
	DWORD_PTR baseAddress;
	if (!ReadProcessMemory(hProcess, (LPCVOID)PLAYER_DECKEDIT_PTR_200610, &baseAddress, sizeof(baseAddress), NULL)) {
		MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
	}

	PrintMemory(baseAddress, "Deck editor memory (pre-hook)\n");

	//DWORD_PTR unknownPtr;
	//if (!ReadProcessMemory(hProcess, (LPCVOID)baseAddress, &unknownPtr, sizeof(unknownPtr), NULL)) {
	//	std::cerr << "Failed to read memory" << std::endl;
	//	return 1;
	//}

	//// Read the value at unknown POINTER
	//WORD unkValue;
	//if (!ReadProcessMemory(hProcess, (LPCVOID)unknownPtr, &unkValue, sizeof(unkValue), NULL)) {
	//	std::cerr << "Failed to read memory" << std::endl;
	//	return 1;
	//}

	// Add the offset to the base address
	DWORD_PTR deckEditAddress_Card = baseAddress + 4;
	DWORD_PTR deckEditAddress_Side = deckEditAddress_Card + (80*2);			//80 cards * 2 bytes
	DWORD_PTR deckEditAddress_Xtra = deckEditAddress_Side + (15*2);			//15 side cards * 2 bytes
	DWORD_PTR deckEditAddress_CardCnt = deckEditAddress_Xtra + (30*2);		//30 xtra cards * 2 bytes + 1 (variable)
	DWORD_PTR deckEditAddress_SideCnt = deckEditAddress_CardCnt + 1;
	DWORD_PTR deckEditAddress_XtraCnt = deckEditAddress_SideCnt + 1;

	// MEMORY PRINT FROM DECK (x cards + 0 side + ALL extra fusion)
	//AC 56 73 00 01 00 02 00  03 00 04 00 05 00 06 00
	//07 00 08 00 09 00 0A 00  0B 00 0C 00 0D 00 0E 00
	//0F 00 10 00 11 00 12 00  13 00 14 00 15 00 16 00
	//17 00 18 00 19 00 1A 00  1B 00 1C 00 1D 00 1F 00
	//20 00 21 00 22 00 23 00  24 00 25 00 26 00 27 00
	//28 00 29 00 2A 00 2B 00  00 00 00 00 00 00 00 00
	//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
	//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
	//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
	//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
	//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
	//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
	//00 00 0A 00 0A 00 1E 00  1E 00 26 00 1E 00 26 00
	//30 00 30 00 30 00 88 00  88 00 88 00 8E 00 8E 00
	//8E 00 B5 00 B5 00 B5 00  E3 00 E3 00 E3 00 04 01
	//04 01 04 01 0B 01 0B 01  0B 01 20 01 20 01 
	PrintMemoryVariable(baseAddress, 253, "Deck editor in memory (post)\n");
	// after this follows the amount of cards in the fixed size byte array (2A) aka 42 / side deck (0) / and extra cards (1E) aka 30
	// array allows max of 80/15/30 *2 = 250 bytes
	// 2A 00 1E

	// Read the value at the deckEditAddress_CardCnt pointer (to verify)
	BYTE valueAtCardCount;
	if (!ReadProcessMemory(hProcess, (LPCVOID)deckEditAddress_CardCnt, &valueAtCardCount, sizeof(valueAtCardCount), NULL)) {
		std::cerr << "Failed to read memory" << std::endl;
		return 1;
	}

	// Check if the value is 0x0200 (Armored Starfish)
	//if (valueAtSecondCard == 0x0000) {
	//	// Prepare the data to write
	//	WORD dataToWrite[5] = { 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 }; //CARD_ID

	//	// Write the data to the final address
	//	if (!WriteProcessMemory(hProcess, (LPVOID)secondCard, dataToWrite, sizeof(dataToWrite), NULL)) {
	//		MessageBox(NULL, "Failed to write memory", "Error", MB_OK | MB_ICONERROR);
	//	}
	//}

	// TEST2 (kaban) - can be used to block broken cards
	// PLAYER_KABAN_PTR_200610 for base
	// PLAYER_KABAN_PTR_200610 + 4 for counter of set kaban
	// PLAYER_KABAN_PTR_200610 + 14 for first card entry
	// MAX: 2690 entrys (0A82)
	// Read the pointer from the given offset
	DWORD_PTR baseAddressKB;
	if (!ReadProcessMemory(hProcess, (LPCVOID)PLAYER_KABAN_PTR_200610, &baseAddressKB, sizeof(baseAddressKB), NULL)) {
		MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
	}

	PrintMemory(baseAddressKB, "\n0xKABAN\n");
	DWORD_PTR addressKbCnt = baseAddressKB + 4;
	DWORD_PTR addressKbFirst = baseAddressKB + 15;

	DWORD kabanCount;
	if (!ReadProcessMemory(hProcess, (LPCVOID)addressKbCnt, &kabanCount, sizeof(kabanCount), NULL)) {
		std::cerr << "Failed to read memory" << std::endl;
		return 1;
	}

	WORD firstKabanEntry;
	if (!ReadProcessMemory(hProcess, (LPCVOID)addressKbFirst, &firstKabanEntry, sizeof(firstKabanEntry), NULL)) {
		std::cerr << "Failed to read memory" << std::endl;
		return 1;
	}

	// TEST: (unknown deck ptr?) - but seems to be not needed
	//WORD dataToWriteTEST[2] = { 0x56AC, 0x7300 };
	//if (!WriteProcessMemory(hProcess, (LPVOID)baseAddress, dataToWriteTEST, sizeof(dataToWriteTEST), NULL)) {
	//		MessageBox(NULL, "Failed to write memory", "Error", MB_OK | MB_ICONERROR);
	//}
	//PrintMemory(baseAddress, "DECK_EDIT_UNK_POINTER\n");

	// Check if the value is 0x0003
	//if (firstKabanEntry == 0x0000) { //0x0003
	//	WORD kabanWrite[1] = { 0x03 }; //yo2 max is 255
	//	// Write the data to the final address
	//	if (!WriteProcessMemory(hProcess, (LPVOID)addressKbFirst, kabanWrite, sizeof(kabanWrite), NULL)) {
	//		MessageBox(NULL, "Failed to write memory", "Error", MB_OK | MB_ICONERROR);
	//	}
	//	if (!WriteProcessMemory(hProcess, (LPVOID)addressKbCnt, kabanWrite, sizeof(kabanWrite), NULL)) {
	//		MessageBox(NULL, "Failed to write memory", "Error", MB_OK | MB_ICONERROR);
	//	}
	//}
	//PrintMemory(baseAddressKB, "0xKABAN_POST\n");

	// Scene implementation
	char scnStr[32];
	sprintf(scnStr, "Loading scene id: %d", sceneNumber);
	log_write(YGO2_LOGFILE_NAME, scnStr, true);
	POINT mouse = GetMousePositionInWindow();
	//int* duelModeDword = (int*)0x012A9084; // 200811
 
	// override scene
	// important scene ids: 11 (unknown scene)
	// 12 main ingame lobby (broken, starts deck edit scene when no deck), 13 tournament mode/tournament scene
	// 15 (debug menu)
	// 28 replay duel, 29 duel end scene, 30 replay janken
	// 36 deck edit
	//sceneNumber = 15; // forces debug menu at start

	// 2006-12 Offline mode
	if (ygoVer == 0) {
		// ONLY FOR TESTING: force debug menu
		//MH_DisableHook(sceneMainLoopHook_Return);
		//if (lastSceneId == -1) { sceneNumber = 13; lastSceneId = 26; }

		if (lastSceneId == -1) { sceneNumber = 2; } // Overwrite default to logo scene (needed to init UI returns)
		if (lastSceneId == 2 && sceneNumber == 3) { // logo scene -> cardswap engine init. scene
			sceneNumber = 33;
		}		
		if (lastSceneId == 33 && sceneNumber == 13) { // cardswap scene -> menu

			// changed for faster fustion dbg
			//sceneNumber = 3;
			sceneNumber = 24;
			// Load player deck from file & Apply for Player 0 (Human)
			PrintMemoryVariable(deckEditAddress_Card, 253, "\n0xDECK_EDITOR_PRE\n");
			LoadDeckFromFileToMemory(hProcess, (LPCVOID)deckEditAddress_Card, "deckOfflineFusionDebug.ydc", &deckData);
			//LoadDeckFromFileToMemory(hProcess, (LPCVOID)deckEditAddress_Card, "deckOffline.ydc", &deckData); // this also adds it to the deck editor
			PrintMemoryVariable(deckEditAddress_Card, 253, "\n0xDECK_EDITOR_POST\n");
			applyPlayerDeckToMemory();

			// Load npc deck from file & Apply for Player 1 (NPC)
			LoadDeckFromFileToMemory(hProcess, NULL, "deckOfflineCPU.ydc", &deckDataNPC, true); // this DOES NOT add to the deck editor (bool)
			applyNPCDeckToMemory();
		}

		//menu --> login (implicated)
		//menu --> deck editor
		if (lastSceneId == 3 && sceneNumber == 4){
			if (mouse.x >= 320 && mouse.x <= 704 && mouse.y >= 514 && mouse.y <= 561) {
				// TODO: maybe we will need to use 31->32 here because of weird init stuff
				sceneNumber = 32;
			}
		}

		// FASTER SHORTCUT TO DUEL SETUP (menu --> play)
		if (lastSceneId == 3 && sceneNumber == 4) { //24 normal duel, 26 janken

			// Shuffle and reapply player deck to memory
			//ShuffleDeckBuffer(&deckData);

			// disabled for fusion debug faster
			applyPlayerDeckToMemory();

			// Shuffle and reapply npc deck to memory
			ShuffleDeckBuffer(&deckDataNPC);
			applyNPCDeckToMemory();

			sceneNumber = 24;
		} 

		// SETUP SWITCH (TODO)

		// NORMAL FLOW (BUT I AM TOO LAZY TO IMPLEMENT IT NOW PROPERLY WITH SETUP STUFF)
		// if (lastSceneId == 4  && sceneNumber == 5) { sceneNumber = 35; }  // login-username -> our fake "return" setup scene
		// if (lastSceneId == 35 && sceneNumber == 4) { sceneNumber = 24; }  // stack duel (TODO: we should read it out from setup scene, 24 (normal) or 26(janken))

		// match end goes to scn (24->25->13) - but we go back to return screen
		//if (lastSceneId == 24 && sceneNumber == 25) { sceneNumber = 3; } // alterative
		if (lastSceneId == 25 && sceneNumber == 13) { sceneNumber = 3; }

		// OUTSIDE OF THE STANDARD FLOW TODO:
		//if (lastSceneId == 3 && sceneNumber == 4) { sceneNumber = 3; } // Delete records dialog (bricks main btn xD - diabled via txt)
		if (lastSceneId == 32 && sceneNumber == 13) { // deck editor back to menu + save deck in ydc
			sceneNumber = 3;
			WriteDeckFromMemoryToFile(hProcess, (LPCVOID)deckEditAddress_Card, "deckOffline.ydc");
			// Enforce this behaviour, because the deck editor sometimes doesn't write it back to player?
			LoadDeckFromFileToMemory(hProcess, (LPCVOID)deckEditAddress_Card, "deckOffline.ydc", &deckData); // this also adds it to the deck editor
		}
		// TODO: we should also do a NPC deck edit dialog
	}
	else {
		// Newer builds (easier replay mode)
		if (sceneNumber == 12) {
			return scene_mainloop(_this, 13);
		}

		// for replay recordings
		if (sceneNumber == 15) {
			sceneNumber = 13;
		}
	}

	// Disable this hook after one usage - only for testing
	//MH_DisableHook(sceneMainLoopHook_Return);

	sprintf(scnStr, "Loading (final) scene id: %d", sceneNumber);
	log_write(YGO2_LOGFILE_NAME, scnStr, true);
	lastSceneId = sceneNumber;
	return scene_mainloop(_this, sceneNumber);
}

int(__cdecl* duel_start)(int);
int __cdecl YGO2::duel_start_reimpl(int mode) {
	// 2008-11
	// int* duelModeDword = (int*)0x012A9084;
	// int* duelTimerDword = (int*)0x012A9080;

	// modes: 0 (single duel), 1 (match duel) 2,5,9,12
	return duelStartHook_Return(mode); //TODO: this crashes because, who knows...
}

int(__cdecl* duel_deck_prepare)(int);
int __cdecl YGO2::duel_deck_prepare_reimpl(int player, int x, int y) {
	// 0: player, 1: NPC
	return duelDeckHook_Return(player);
}

char(__fastcall* sub_5adcb0)(unsigned int, unsigned int);
char __fastcall YGO2::sub_5adcb0_reimpl(unsigned int a, unsigned int b) {
	printf("sub_5adcb0: a=%d, b=%d\n", a, b);

	// read jump table selector
	HANDLE hProcess = GetCurrentProcess();
	DWORD jmpIdx;
	if (!ReadProcessMemory(hProcess, (LPCVOID)WORD_00BEDAE0, &jmpIdx, sizeof(jmpIdx), NULL)) {
		MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
	}
	printf("word_00BEDAE0 = %d\n", jmpIdx);
	char ret = sub_5ADCB0_Return(a, b);
	printf("return = %d\n", ret);

	return ret;
}

int __cdecl YGO2::sub_64f8e0_reimpl(int a, int b, int c, unsigned int d, unsigned int e) {
	// ID table at: https://github.com/derplayer/YuGiOh-PoC-ModTools/wiki/YGO:-Information-corner-(v2)#card-board-operation-ids-2007-03
	printf("BOARD_HANDLE_64f8e0: a=%d, b=%d, c=%d, d=%d, e=%d\n", a, b, c, d, e);
	int ret = sub_64F8E0_Return(a, b, c, d, e);
	printf("return = %d\n", ret);

	// face-up/down & atack/defence position states for all card types
	if (a == 63 || a == 64 || a == 65 || a == 66 || a == 67 || a == 68 || a == 69 || a == 70 || a == 71 || a == 72 || a == 73) {
		//					   id,   unk, cardId, faceflag
		// BOARD_HANDLE_64f8e0: a=67, b=0, c=4837, d=0 # magic card set face-down
		// BOARD_HANDLE_64f8e0: a=67, b=0, c=4837, d=1 # maigc card set face-up (activate)
		// not enough to change the face-up state tho...
		// d = 1;
		printf("!BOARD_HANDLE_OVERRIDE: a=%d, b=%d, c=%d, d=%d, e=%d\n", a, b, c, d, e);
		return sub_64F8E0_Return(a, b, c, d, e);
	}
	//if (a == 24)
	//	return sub_64F8E0_Return(2, 1, c, d); // DUEL_VIEW_CARD_MOVE - aka draw card
	//if (a == 28)
	//	return sub_64F8E0_Return(2, 1, c, d); // DUEL_VIEW_CARD_SET iPlayer=%d iLocate=%d fTurn=%d

	return ret;
}

// ### CONSTRUCTOR
YGO2::YGO2(int ver, std::string verStr) {
	ygoVer = ver;
	ygoVerStr = verStr;

	// Debug console
	AllocConsole();
	SetConsoleOutputCP(932);
	SetConsoleCP(932);
	std::wcout.imbue(std::locale("ja_JP.utf-8"));
	SetConsoleTitleA("YGO2 DEBUG CONSOLE");

	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	// Set console screen buffer size
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD bufferSize = { 700, 768 };
	SetConsoleScreenBufferSize(hConsole, bufferSize);

	// Set console window size
	HWND hwndConsole = GetConsoleWindow();
	MoveWindow(hwndConsole, 0, 0, 700, 768, TRUE);
	//MoveWindow(hwndConsole, 0, 0, 320, 778, TRUE); //Youtube OBS

	std::cout << "Yu-Gi-Oh! Duel Evolution: Offline Mod - alpha test version 0." << YGO2_MODVERSION << ") started." << std::endl;
	std::cout << "YGO2 (" << ygoVerStr << " - exeVer " << ygoVer << ") detected!" << std::endl;

	// Add the vectored exception handler
	AddVectoredExceptionHandler(1, VectoredHandler);

	// Debug menu change text
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wide_ygoVerStr = converter.from_bytes(ygoVerStr); // convert to std::wstring
	wchar_t* debugTextOverridePtr;
	char* debugParam;
	char* apiGate;
	char* website;
	char* title;
	char* playerDummyDeck;

	std::string playerDeck;
	std::wstringstream debugWStrStream;
	size_t byteArraySize;
	unsigned char* byteArray;

	debugWStrStream << "@9 Yu-Gi-Oh! Online: Duel Evolution (" << wide_ygoVerStr << " - exeVer " << ygoVer << ")\
                    \n@9 Press the key '1' to load a deck from memory. Clicking the duel button will only create a dummy deck. \
                    \n@1 You need a kaban.bin to enable cards in deck builder! \
                    \n\n@9 Created by DerPlayer and PhilYeahz";

	// the beta have no debug hotkeys
	if (ygoVer != 0) {
		debugWStrStream << "\n\n@9 Hotkeys for additional debug output : SHIFT + W, SHIFT + E, SHIFT + X, SHIFT + C";
	}

	SIZE_T size;
	DWORD oldProtect;
	DWORD offsets[2];
	DWORD offsetsTXT[2];
	char* txtPtr = "\nThis is an alpha version of the Duel Evolution\noffline mod. Bugs are expected. Do not share this build!\nFor more infos read the @3README.TXT";
	std::string windowTitleTxt = "Yu-Gi-Oh! Duel Evolution: Offline Mod - alpha test 0." + std::to_string(YGO2_MODVERSION);

	switch (ver)
	{
	case YGO2_2006_10:
		// Force activate debug mode by nulling the param string
		debugParam = (char*)DEBUG_PARAMFLAG_200610;
		strncpy(debugParam, "", 5);

		// nop the api gate
		//apiGate = (char*)API_GATE_ADDR_200610;
		//strncpy(apiGate, "127.0.0.1\0", 28);

		// discord link
		//website = (char*)HTTP_WEBSITE_LINK_200610;
		//strncpy(website, "127.0.0.1\0", 28);
		
		// SwapEngine scene string override to make it into a splash info screen
		//PrintMemory(SCN_CARDSWAP_BTN_RETURN_PTR_200610, "STR_RESIZE_PRE\n");
		offsets[0] = SCN_CARDSWAP_BTN_RETURN_PTR_200610 + 1;
		offsets[1] = SCN_CARDSWAP_BTN_RETURN_PTR_200610 + 16;
		ApplyTextToNewOffset("@1Click here to start.", offsets, sizeof(offsets) / sizeof(offsets[0]));
		offsets[0] = 0x00509361 + 1;
		offsets[1] = 0x00509361 + 16;
		// make the second button invisible
		ApplyTextToNewOffset("", offsets, sizeof(offsets) / sizeof(offsets[0]));
		offsets[0] = 0x005092BA + 1;
		offsets[1] = 0x005092BA + 16;
		ApplyTextToNewOffset(txtPtr, offsets, sizeof(offsets) / sizeof(offsets[0]));
		//PrintMemory(SCN_CARDSWAP_BTN_RETURN_PTR_200610, "STR_RESIZE_POST\n");

		// Change window title
		offsets[0] = 0x004EBAC4 + 1;
		offsets[1] = 0x004EBAFB + 1;
		ApplyTextToNewOffset(windowTitleTxt.c_str(), offsets, sizeof(offsets) / sizeof(offsets[0]));

		// Text edit 200610
		debugTextOverridePtr = (wchar_t*)DEBUG_TEXTSTRING_200610;
		wcsncpy(debugTextOverridePtr, debugWStrStream.str().c_str(), 470); // ~474 is max!

		debuglogHook = hooktype_debuglog(DEBUG_LOG_ADDR_200610);
		debuglogNetworkHook = hooktype_debuglog_net(DEBUG_LOG_ADDR_NETWORK_200610);
		printfHook = hooktype_printf(DEBUG_LOG_PRINTF_200610);
		sprintfHook = hooktype_sprintf(DEBUG_LOG_SPRINTF_200610);
		debuglogFileHook = hooktype_debuglog_file(DEBUG_LOG_FILE_200610);

		//debuglogVerbHook = hooktype_debuglog_verb(DEBUG_LOG_VERB_200610);
		sceneMainLoopHook = hooktype_scn_mainloop(SCN_MAINLOOP_200610);
		duelStartHook = hooktype_duelstart(DUEL_START_200610);
		duelDeckHook = hooktype_dueldeck(DUEL_DECK_PREPARE_200610);
		sub_5adcb0_hook = hooktype_sub_5ADCB0(SUB_5ADCB0);
		sub_64f8e0_hook = hooktype_sub_64F8E0(SUB_64F8E0);
		break;
	case YGO2_2008_01:
		// Force activate debug mode by nulling the param string
		debugParam = (char*)DEBUG_PARAMFLAG_200801;
		strncpy(debugParam, "", 5);

		debuglogHook = hooktype_debuglog(DEBUG_LOG_ADDR_200801);
		debuglogNetworkHook = hooktype_debuglog_net(DEBUG_LOG_ADDR_NETWORK_200801);
		printfHook = hooktype_printf(DEBUG_LOG_PRINTF_200801);
		fprintfHook = hooktype_fprintf(DEBUG_LOG_FPRINTF_200801);
		sprintfHook = hooktype_sprintf(DEBUG_LOG_SPRINTF_200801);
		//debuglogVerbHook = hooktype_debuglog_verb(DEBUG_LOG_VERB_200801);

		//sceneMainLoopHook = hooktype_scn_mainloop(SCN_MAINLOOP_200801);
		break;
	case YGO2_2008_11:
		// Force activate debug scene mode by nulling the param string
		debugParam = (char*)DEBUG_PARAMFLAG_200811;
		strncpy(debugParam, "", 5);

		// Text edit 200811
		debugTextOverridePtr = (wchar_t*)DEBUG_TEXTSTRING_200811;
		wcsncpy(debugTextOverridePtr, debugWStrStream.str().c_str(), 420); // ~426 is max!

		debuglogHook = hooktype_debuglog(DEBUG_LOG_ADDR_200811);
		debuglogNetworkHook = hooktype_debuglog_net(DEBUG_LOG_ADDR_NETWORK_200811);
		printfHook = hooktype_printf(DEBUG_LOG_PRINTF_200811);
		fprintfHook = hooktype_fprintf(DEBUG_LOG_FPRINTF_200811);
		sprintfHook = hooktype_sprintf(DEBUG_LOG_SPRINTF_200811);
		//debuglogVerbHook = hooktype_debuglog_verb(DEBUG_LOG_VERB_200811);

		sceneMainLoopHook = hooktype_scn_mainloop(SCN_MAINLOOP_200811);
		//duelHook = hooktype_duelscene(DUEL_ADDR_200811);
		//duelStartHook = hooktype_duelstart(DUEL_START_200811); - corrupts the game too much
		break;
	case -1:
		MessageBoxW(0, L"This YGO2 game version is not yet supported by the DLL plugin. Please submit it.", L"", 0);
		return;
	}
	MH_STATUS dlogRes;

	// BASE #00 - Stub log restoration Hook
	dlogRes = MH_CreateHook(debuglogHook, &debug_log, NULL);
	if (dlogRes == MH_OK) MH_EnableHook(debuglogHook);

	// BASE #01 - Network log
	dlogRes = MH_CreateHook(debuglogNetworkHook, &network_response_log_stub, NULL);
	if (dlogRes == MH_OK) MH_EnableHook(debuglogNetworkHook);

	// BASE #02 - printf
	dlogRes = MH_CreateHook(printfHook, &debug_log_buffer_ret, NULL);
	if (dlogRes == MH_OK) MH_EnableHook(printfHook);

	// BASE #03 - fprintf - needs tramp. return
	dlogRes = MH_CreateHook(fprintfHook, &fprint_reimpl, reinterpret_cast<LPVOID*>(&fprintfHook_Return));
	if (dlogRes == MH_OK) MH_EnableHook(fprintfHook);

	// BASE #04 - sprintf - needs tramp. return
	dlogRes = MH_CreateHook(sprintfHook, &sprintf_reimpl, reinterpret_cast<LPVOID*>(&sprintfHook_Return));
	if (dlogRes == MH_OK) MH_EnableHook(sprintfHook);

	// BASE #05 - file log hook
	dlogRes = MH_CreateHook(debuglogFileHook, &debug_log_file, reinterpret_cast<LPVOID*>(&debuglogFileHook_Return));
	if (dlogRes == MH_OK) MH_EnableHook(debuglogFileHook);

	// BASE #05 - Stub log restoration (error) Hook - (disabled by default because slows the app too much)
	//dlogRes = MH_CreateHook(debuglogVerbHook, &debug_log_verb, reinterpret_cast<LPVOID*>(&debuglogVerbHook_Return));
	//if (dlogRes == MH_OK) MH_EnableHook(debuglogVerbHook);

	// Extra #01 - Scene main loop (with scene id)
	dlogRes = MH_CreateHook(
		sceneMainLoopHook, // target
		reinterpret_cast<void*>(&scene_mainloop_reimpl),
		reinterpret_cast<void**>(&scene_mainloop)
	);

	if (dlogRes == MH_OK) MH_EnableHook(sceneMainLoopHook);

	// Extra #02 - Duel deck init. (deprecated, we can't enforce this mode via code easily)
	dlogRes = MH_CreateHook(duelDeckHook, &duel_deck_prepare_reimpl, reinterpret_cast<LPVOID*>(&duelDeckHook_Return));
	//if (dlogRes == MH_OK) MH_EnableHook(duelDeckHook);

	dlogRes = MH_CreateHook(sub_5adcb0_hook, &sub_5adcb0_reimpl, reinterpret_cast<LPVOID*>(&sub_5ADCB0_Return));
	if (dlogRes == MH_OK) MH_EnableHook(sub_5adcb0_hook);

	dlogRes = MH_CreateHook(sub_64f8e0_hook, &sub_64f8e0_reimpl, reinterpret_cast<LPVOID*>(&sub_64F8E0_Return));
	if (dlogRes == MH_OK) MH_EnableHook(sub_64f8e0_hook);

	// Extra #03 - Duel start (with duel mode id)
	//dlogRes = MH_CreateHook(duelStartHook, &duel_start_reimpl, reinterpret_cast<LPVOID*>(&duelStartHook_Return));
	//if (dlogRes == MH_OK) MH_EnableHook(duelStartHook);
}