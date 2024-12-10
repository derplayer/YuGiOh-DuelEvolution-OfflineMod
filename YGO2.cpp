#pragma warning(disable:26495)

#include "YGO2.h"
#include <filesystem>
static int ygoVer = 0;
static std::string ygoVerStr = "unk";

// ### Trampoline returns
static YGO2::hooktype_fprintf fprintfHook_Return = nullptr;
static YGO2::hooktype_sprintf sprintfHook_Return = nullptr;
static YGO2::hooktype_debuglog_verb debuglogVerbHook_Return = nullptr;

static YGO2::hooktype_scn_mainloop sceneMainLoopHook_Return = nullptr;
static YGO2::hooktype_duelstart duelStartHook_Return = nullptr;
static YGO2::hooktype_dueldeck duelDeckHook_Return = nullptr;

static int lastSceneId = -1;

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
	char* modPlayer = strdup("Player");
	char* cpu = strdup("CPU (Generic)");
	if (strcmp(formattedString, "KONAMI_PLAYER_001") == 0) {
		strncpy(formattedString, modPlayer, strlen(modPlayer) + 1);
	}
	else if (strcmp(formattedString, "KONAMI_PLAYER_002") == 0) {
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

int(__thiscall* scene_mainloop)(void*, int);
int __fastcall YGO2::scene_mainloop_reimpl(void* _this, void* x, int sceneNumber) {
	// here we are calling the trampoline & executing the code
	// of the target function, while also being able to run our own code afterwards/before

	// TEST 1 (player stack deck)
	// Get the handle to the current process
	HANDLE hProcess = GetCurrentProcess();

	// Read the pointer from the given offset
	DWORD_PTR baseAddress;
	if (!ReadProcessMemory(hProcess, (LPCVOID)PLAYER_DECK_PTR_200610, &baseAddress, sizeof(baseAddress), NULL)) {
		MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
	}

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
	DWORD_PTR finalAddress = baseAddress + 3;
	DWORD_PTR finalAddress1 = baseAddress + 4;
	DWORD_PTR secondCard = baseAddress + 5;

	// Read the value at the secondCard pointer
	WORD valueAtSecondCard;
	if (!ReadProcessMemory(hProcess, (LPCVOID)secondCard, &valueAtSecondCard, sizeof(valueAtSecondCard), NULL)) {
		std::cerr << "Failed to read memory" << std::endl;
		return 1;
	}

	// Check if the value is 0x0200 (Armored Starfish)
	if (valueAtSecondCard == 0x0000) {
		// Prepare the data to write
		WORD dataToWrite[5] = { 0x0100, 0x0100, 0x0100, 0x0100, 0x0100 }; //CARD_ID

		// Write the data to the final address
		if (!WriteProcessMemory(hProcess, (LPVOID)secondCard, dataToWrite, sizeof(dataToWrite), NULL)) {
			MessageBox(NULL, "Failed to write memory", "Error", MB_OK | MB_ICONERROR);
		}
	}

	// Read the next 256 bytes from the base address
	BYTE buffer[256];
	if (!ReadProcessMemory(hProcess, (LPCVOID)baseAddress, buffer, sizeof(buffer), NULL)) {
		MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
	}

	// Output the data for verification purposes
	printf("0xPLAYER_DECK\n");
	for (int i = 0; i < 256; ++i) {
		printf("%02X ", buffer[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}

	// TEST2 (kaban)
	// PLAYER_KABAN_PTR_200610 for base
	// PLAYER_KABAN_PTR_200610 + 4 for counter of set kaban
	// PLAYER_KABAN_PTR_200610 + 14 for first card entry
	// MAX: 2690 entrys (0A82)
	// Read the pointer from the given offset
	DWORD_PTR baseAddressKB;
	if (!ReadProcessMemory(hProcess, (LPCVOID)PLAYER_KABAN_PTR_200610, &baseAddressKB, sizeof(baseAddressKB), NULL)) {
		MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
	}

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

	// TEST (unknown deck ptr?)
	//WORD dataToWriteTEST[2] = { 0x56AC, 0x7300 }; //CARD_ID
	//if (!WriteProcessMemory(hProcess, (LPVOID)baseAddress, dataToWriteTEST, sizeof(dataToWriteTEST), NULL)) {
	//		MessageBox(NULL, "Failed to write memory", "Error", MB_OK | MB_ICONERROR);
	//}
	//PrintMemory(baseAddress, "TESTMEM\n");

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

	// Read the next 256 bytes from the base address
	if (!ReadProcessMemory(hProcess, (LPCVOID)baseAddressKB, buffer, sizeof(buffer), NULL)) {
		MessageBox(NULL, "Failed to read memory", "Error", MB_OK | MB_ICONERROR);
	}

	// Output the data for verification purposes
	printf("0xKABAN\n");
	for (int i = 0; i < 256; ++i) {
		printf("%02X ", buffer[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}

	// SCene impl
	char scnStr[32];
	sprintf(scnStr, "Loading scene id: %d", sceneNumber);
	log_write(YGO2_LOGFILE_NAME, scnStr, true);
	//int* duelModeDword = (int*)0x012A9084; // 200811
	// 
	// override scene
	// important scene ids: 11 (unknown scene)
	// 12 main ingame lobby (broken, starts deck edit scene when no deck), 13 tournament mode/tournament scene
	// 15 (debug menu)
	// 28 replay duel, 29 duel end scene, 30 replay janken
	// 36 deck edit
	//sceneNumber = 15; // forces debug menu at start

	// 2006-12 Offline mode
	POINT mouse = GetMousePositionInWindow();
	if (ygoVer == 0) {
		// ONLY FOR TESTING: force debug menu
		//MH_DisableHook(sceneMainLoopHook_Return);
		//if (lastSceneId == -1) { sceneNumber = 13; lastSceneId = 26; }
		//if(sceneNumber == 13) return scene_mainloop(_this, sceneNumber);

		if (lastSceneId == -1) { sceneNumber = 2; } // Overwrite default to logo scene (needed to init UI returns)
		if (lastSceneId == 2 && sceneNumber == 3) { // logo scene -> cardswap engine init. scene
			sceneNumber = 33;
		}		
		if (lastSceneId == 33 && sceneNumber == 13) { // cardswap scene -> menu
			sceneNumber = 3; 
			// Load deck from file
			LoadDeckFromFileToMemory(hProcess, (LPCVOID)finalAddress1, "deckOffline.ydc");
			PrintMemory(finalAddress1, "0xPLAYER_DECK_NEW\n");
			// NPC STACK TEST (THIS IS MEMORY LAYOUT
			//						   28 00 00 00 00 00 00 00
			//01 00 00 00 23 01 23 01  23 01 23 01 23 01 23 01
			//23 01 23 01 23 01 23 01  45 02 45 02 45 02 45 02
			//45 02 45 02 45 02 45 02  45 02 45 02 A6 02 A6 02
			//A6 02 A6 02 A6 02 A6 02  A6 02 A6 02 A6 02 A6 02
			//52 04 52 04 52 04 52 04  52 04 52 04 52 04 52 04
			//52 04 52 04 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 28 04 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			//00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00

			// Setups the NPC deck (40 cards, 0 side deck cards(?), 1 extra deck card)
			BYTE npcDeckHeader[12] = { 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			// Second byte array with specified hex numbers
			BYTE npcDeckTestData[] = {
				0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01,
				0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01, 0x23, 0x01,

				0x45, 0x02, 0x45, 0x02, 0x45, 0x02, 0x45, 0x02, 0x45, 0x02,
				0x45, 0x02, 0x45, 0x02, 0x45, 0x02, 0x45, 0x02, 0x45, 0x02,

				0xA6, 0x02, 0xA6, 0x02, 0xA6, 0x02, 0xA6, 0x02, 0xA6, 0x02,
				0xA6, 0x02, 0xA6, 0x02, 0xA6, 0x02, 0xA6, 0x02, 0xA6, 0x02,

				0x52, 0x04, 0x52, 0x04, 0x52, 0x04, 0x52, 0x04, 0x52, 0x04,
				0x52, 0x04, 0x52, 0x04, 0x52, 0x04, 0x52, 0x04, 0x52, 0x04,
			};
			ApplyBytesDirect(NPC_DECK_PTR_200610, npcDeckHeader, sizeof(npcDeckHeader));
			ApplyBytesDirect(NPC_DECK_PTR_200610+12, npcDeckTestData, sizeof(npcDeckTestData));
			//ApplyBytesToNewOffset(npcDeckHeader, sizeof(npcDeckHeader), npcDeckOffset, sizeof(npcDeckOffset) / sizeof(npcDeckOffset[0]));
			PrintMemory(NPC_DECK_PTR_200610, "NPC DECK ADDR\n");
			//LoadDeckFromFileToMemory(hProcess, (LPCVOID)NPC_DECK_PTR_200610, "deckOffline.ydc");
			// PrintMemory(NPC_DECK_PTR_200610, "0xNPC_DECK_NEW\n");
		}
		//menu --> login (implicated)
		//menu --> deck editor
		if (lastSceneId == 3 && sceneNumber == 4){
			if (mouse.x >= 320 && mouse.x <= 704 && mouse.y >= 514 && mouse.y <= 561) {
				sceneNumber = 32;
			}
		}

		// FASTER SHORTCUT TO DUEL SETUP
		if (lastSceneId == 3 && sceneNumber == 4) { sceneNumber = 24; } //24

		// SETUP SWITCH

		// NORMAL FLOW (BUT I AM TOO LAZY TO IMPLEMENT IT NOW PROPERLY WITH SETUP STUFF)
		if (lastSceneId == 4  && sceneNumber == 5) { sceneNumber = 35; }  // login-username -> our fake "return" setup scene
		if (lastSceneId == 35 && sceneNumber == 4) { sceneNumber = 24; }  // stack duel (TODO: we should read it out from setup scene, 24 (normal) or 26(janken))

		// match end goes to scn (24->25->13) - but we go back to return screen
		//if (lastSceneId == 24 && sceneNumber == 25) { sceneNumber = 3; } // alterative
		if (lastSceneId == 25 && sceneNumber == 13) { sceneNumber = 3; }

		// OUTSIDE OF THE STANDARD FLOW TODO:
		//if (lastSceneId == 3 && sceneNumber == 4) { sceneNumber = 3; } // Delete records dialog (bricks main btn xD - diabled via txt)
		if (lastSceneId == 32 && sceneNumber == 13) { // deck editor back to menu + save deck in ydc
			sceneNumber = 3;
			WriteDeckFromMemoryToFile(hProcess, (LPCVOID)finalAddress1, "deckOffline.ydc");
		}
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

	// Disable this hook after one usage (TODO: make it forceable via config)
	//MH_DisableHook(sceneMainLoopHook_Return);

	sprintf(scnStr, "Loading (final) scene id: %d", sceneNumber);
	log_write(YGO2_LOGFILE_NAME, scnStr, true);
	lastSceneId = sceneNumber;
	return scene_mainloop(_this, sceneNumber);
}

int(__cdecl* duel_start)(int);
int __cdecl YGO2::duel_start_reimpl(int mode) {
	//int* duelModeDword = (int*)0x012A9084;
	//int* duelTimerDword = (int*)0x012A9080;

	// modes: 0 (single duel), 1 (match duel) 2,5,9,12
	//mode = 4;
	return duelStartHook_Return(mode); //TODO: this crashes because, who knows... so we just hardedit the jumptable
}

int(__cdecl* duel_deck_prepare)(int);
int __cdecl YGO2::duel_deck_prepare_reimpl(int player, int x, int y) {
	DWORD offsetDeckAddr[1];
	offsetDeckAddr[0] = DUMMY_DECK_PTR_REAL_200610;
	BYTE buffer[250];
	BYTE bufferFinal[500];		
	BYTE bufferFinalExtra[250]; 
	HANDLE hProcess = GetCurrentProcess();

	// Load deck into buffer
	LoadDeckFromFileToMemory(hProcess, (LPCVOID)buffer, "deckOffline.ydc");

	// Convert the main buffer section from 0x00 to 0xBE
	size_t count = 0;
	size_t bufferIndex = 0;
	for (size_t i = 0; i < (190/2); i += 2) {
		bufferFinal[bufferIndex++] = buffer[i];
		bufferFinal[bufferIndex++] = buffer[i + 1];
		bufferFinal[bufferIndex++] = 0x00; // Add padding or additional bytes as needed
		bufferFinal[bufferIndex++] = 0x00; // Add padding or additional bytes as needed

		if (buffer[i] != 0x00 || buffer[i + 1] != 0x00) {
			count++;
		}
	}

	// Convert the extra buffer section from 0xBF to the end
	size_t bufferExtraIndex = 0;
	for (size_t i = 0xBF; i < sizeof(buffer); i += 2) {
		bufferFinalExtra[bufferExtraIndex++] = buffer[i];
		bufferFinalExtra[bufferExtraIndex++] = buffer[i + 1];
		bufferFinalExtra[bufferExtraIndex++] = 0x00; // Add padding or additional bytes as needed
		bufferFinalExtra[bufferExtraIndex++] = 0x00; // Add padding or additional bytes as needed
	}

	// Update the value at DUMMY_DECK_PTR_REAL_200610
	DWORD baseAddress;
	ReadProcessMemory(hProcess, (LPCVOID)DUMMY_DECK_AMOUNT_200610, &baseAddress, sizeof(baseAddress), NULL);
	count = 43;

	// Change the protection of card count to writable (CRASHES?)
	//PrintMemory(DUMMY_DECK_AMOUNT_200610, "0xCARD_COUNT_PRE\n");
	//DWORD oldProtect;
	//if (VirtualProtect((char*)DUMMY_DECK_AMOUNT_200610, sizeof(count), PAGE_READWRITE, &oldProtect)) {
	//	// Write your data
	//	memcpy((char*)DUMMY_DECK_AMOUNT_200610, &count, sizeof(count));

	//	// Restore the original protection
	//	VirtualProtect((char*)DUMMY_DECK_AMOUNT_200610, sizeof(count), oldProtect, &oldProtect);
	//}
	//else {
	//	std::cerr << "Failed to change protection." << std::endl;
	//}

	//ReadProcessMemory(hProcess, (LPCVOID)DUMMY_DECK_AMOUNT_200610, &baseAddress, sizeof(baseAddress), NULL);
	//PrintMemory(DUMMY_DECK_AMOUNT_200610, "0xCARD_COUNT_POST\n");

	// Apply the new byte arrays to the specified offsets
	ApplyBytesToNewOffset(bufferFinal, bufferIndex, offsetDeckAddr, sizeof(offsetDeckAddr) / sizeof(offsetDeckAddr[0]));
	//ApplyBytesToNewOffset(bufferFinalExtra, bufferExtraIndex, offsetDeckAddr, sizeof(offsetDeckAddr) / sizeof(offsetDeckAddr[0]));

	// 0: player, 1: NPC
	return duelDeckHook_Return(player);
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

	std::cout << "YGO2 (" << ygoVerStr << " - exeVer " << ygoVer << ") detected!" << std::endl;

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
	char* txtPtr = "\nThis is an alpha version of the Duel Evolution\noffline mod. Bugs are expected. Do not share this build!\nFor more infos: discord.gg/GAKKaJYwF7";

	switch (ver)
	{
	case YGO2_2006_10:
		// Force activate debug mode by nulling the param string
		debugParam = (char*)DEBUG_PARAMFLAG_200610;
		strncpy(debugParam, "", 5);

		// nop the api gate
		apiGate = (char*)API_GATE_ADDR_200610;
		strncpy(apiGate, "127.0.0.1\0", 28);

		// discord link
		website = (char*)HTTP_WEBSITE_LINK_200610;
		strncpy(website, "127.0.0.1\0", 28);
		
		// title
		title = (char*)0x00773EC8;
		strncpy(title, "Yu-Gi-Oh OFFLINE 2\0", 18);

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

		// THIS IS OLD, REMOVE I GUESS (STACK DECK HAS ITS OWN ISSUES)
		playerDeck =
			"23010000"
			"23010000"
			"23010000"
			"23010000"
			"23010000"
			"23010000"
			"23010000"
			"23010000"
			"23010000"
			"23010000"
			"45020000"
			"45020000"
			"45020000"
			"45020000"
			"45020000"
			"45020000"
			"45020000"
			"45020000"
			"45020000"
			"45020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"A6020000"
			"52040000"
			"52040000"
			"52040000"
			"52040000"
			"52040000"
			"52040000"
			"52040000"
			"52040000"
			"52040000"
			"52040000";

		//debug098:02C50000					//stack segment?
		//debug095:02BC72D1					  db  56h; V
		//debug095 : 02BC72D2                 db  73h; s
		//debug095 : 02BC72D3                 db    0
		//debug095 : 02BC72D4                 db    1
		//debug095 : 02BC72D5                 db    0
		//debug095 : 02BC72D6                 db    2
		//debug095 : 02BC72D7                 db    0
		//debug095 : 02BC72D8                 db    3
		//debug095 : 02BC72D9                 db    0

		// Byte array to hold the converted bytes
		//byteArraySize = playerDeck.length() / 2;
		//byteArray = new unsigned char[byteArraySize];

		// Convert hex string to byte array
		//hexStringToByteArray(playerDeck, byteArray, byteArraySize);

		// Copy the bytes to the destination
		//playerDummyDeck = (char*)PLAYER_DECK_ADDR_200610;
		//size = byteArraySize; // Size of your byte array

		//// Change the protection to writable
		//if (VirtualProtect(playerDummyDeck, size, PAGE_READWRITE, &oldProtect)) {
		//	// Write your data
		//	memcpy(playerDummyDeck, byteArray, byteArraySize);

		//	// Restore the original protection
		//	VirtualProtect(playerDummyDeck, size, oldProtect, &oldProtect);
		//}
		//else {
		//	std::cerr << "Failed to change protection." << std::endl;
		//}

		// Clean up
		//delete[] byteArray;

		// For demonstration purposes, print the byte array as hex
		//std::cout << "Converted bytes:" << std::endl;
		//for (size_t i = 0; i < byteArraySize; ++i) {
		//	std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byteArray[i];
		//}
		//std::cout << std::endl;

		// Text edit 200610
		debugTextOverridePtr = (wchar_t*)DEBUG_TEXTSTRING_200610;
		wcsncpy(debugTextOverridePtr, debugWStrStream.str().c_str(), 470); // ~474 is max!

		debuglogHook = hooktype_debuglog(DEBUG_LOG_ADDR_200610);
		debuglogNetworkHook = hooktype_debuglog_net(DEBUG_LOG_ADDR_NETWORK_200610);
		printfHook = hooktype_printf(DEBUG_LOG_PRINTF_200610);
		sprintfHook = hooktype_sprintf(DEBUG_LOG_SPRINTF_200610);

		//debuglogVerbHook = hooktype_debuglog_verb(DEBUG_LOG_VERB_200610);
		sceneMainLoopHook = hooktype_scn_mainloop(SCN_MAINLOOP_200610);
		duelStartHook = hooktype_duelstart(DUEL_START_200610);
		duelDeckHook = hooktype_dueldeck(DUEL_DECK_PREPARE_200610);
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
		// Force activate debug mode by nulling the param string
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

	// Extra #03 - Duel start (with duel mode id)
	//dlogRes = MH_CreateHook(duelStartHook, &duel_start_reimpl, reinterpret_cast<LPVOID*>(&duelStartHook_Return));
	//if (dlogRes == MH_OK) MH_EnableHook(duelStartHook);
}