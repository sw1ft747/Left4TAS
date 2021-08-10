// C++
// Input Manager

#include "input_manager.h"

#include "../prop_offsets.h"
#include "../offsets.h"
#include "../sdk.h"
#include "usercmd.h"
#include "utils.h"

#include <string>

//-----------------------------------------------------------------------------

#ifdef strdup
#undef strdup
#endif

//-----------------------------------------------------------------------------

class CBaseCombatWeapon;
class C_BaseCombatWeapon;

extern std::string sInputsDirectory;
extern const char *g_pszDirectory;

extern IServerGameDLL *g_pServerGameDLL;
extern IClientEntityList *g_pClientEntityList;

//-----------------------------------------------------------------------------

CInputManager g_InputManager;
InputFrame g_inputFrameBuffer;

InputData g_InputDataClient[MAX_SPLITSCREEN_PLAYERS];
InputData g_InputDataServer[MAXCLIENTS + 1];

//-----------------------------------------------------------------------------

const char *CInputManager::GetFilePath(const char *pszFilename)
{
	static char s_szFilepathBuffer[512];
	int result = sprintf_s(s_szFilepathBuffer, sizeof(s_szFilepathBuffer), "%s%s.bin", sInputsDirectory.c_str(), pszFilename);

	if (result == -1)
	{
		Warning("CInputManager::GetFilePath: buffer overflow\n");
		return NULL;
	}

	return strdup(s_szFilepathBuffer);
}

void CInputManager::Record(const char *pszFilename, InputData *inputData, float orientation[3][3])
{
	if (inputData->input)
	{
		Msg("[IM] Already recording/playback\n");
		return;
	}
	else
	{
		Msg("[IM] Recording inputs...\n");
	}

	const char *pszFilepath = GetFilePath(pszFilename);

	if (!pszFilepath)
		return;

	FILE *file = fopen(pszFilepath, "wb");

	if (file)
	{
		InputBaseInfo info;
		int header_buffer = 0;

		memset(&info, 0, sizeof(info));

		info.version = INPUT_MANAGER_VERSION;

		memcpy(&info.origin, orientation, sizeof(*orientation) * 3);

		header_buffer = IM_FILE_HEADER;
		fwrite(&header_buffer, 1, sizeof(short), file);

		header_buffer = IM_FILE_INFO_HEADER;
		fwrite(&header_buffer, 1, sizeof(char), file);

		fwrite(&info, 1, IM_BASE_INFO_SIZE, file);

		header_buffer = IM_FILE_FRAMES_HEADER;
		fwrite(&header_buffer, 1, sizeof(char), file);

		inputData->input = file;
		inputData->filename = pszFilepath;
		inputData->active = true;
		inputData->recording = true;
		inputData->frames = 0;
		inputData->baseInfo = info;
	}
	else
	{
		Warning("CInputManager::Record: failed to create file\n");
	}
}

void CInputManager::Playback(const char *pszFilename, InputData *inputData)
{
	if (inputData->input)
	{
		Msg("[IM] Already playback/recording\n");
		return;
	}
	else
	{
		Msg("[IM] Playback inputs...\n");
	}

	const char *pszFilepath = GetFilePath(pszFilename);

	if (!pszFilepath)
		return;

	FILE *file = fopen(pszFilepath, "rb");

	if (file)
	{
		InputBaseInfo info;
		int header_buffer = 0;

		fread(&header_buffer, 1, sizeof(short), file);

		if (header_buffer == IM_FILE_HEADER)
		{
			header_buffer = 0;
			fread(&header_buffer, 1, sizeof(char), file);

			if (header_buffer == IM_FILE_INFO_HEADER)
			{
				header_buffer = 0;

				fread(&info, 1, IM_BASE_INFO_SIZE, file);
				fread(&header_buffer, 1, sizeof(char), file);

				if (header_buffer == IM_FILE_FRAMES_HEADER)
				{
					inputData->input = file;
					inputData->filename = pszFilepath;
					inputData->active = true;
					inputData->recording = false;
					inputData->frames = 0;
					inputData->baseInfo = info;
					return;
				}
			}
		}

		Warning("CInputManager::Playback: failed to read file\n");
	}
	else
	{
		Warning("CInputManager::Playback: failed to open file\n");
	}
}

void CInputManager::Split(InputData *inputData)
{
	if (!inputData->input)
	{
		Msg("[IM] Nothing playback\n");
		return;
	}

	long size = ftell(inputData->input); // current position of stream's cursor
	char *fileData = new char[size];

	if (!fileData)
	{
		Warning("CInputManager::Split: failed to allocate memory!\n");
		return;
	}

	fseek(inputData->input, 0, SEEK_SET);
	fread(fileData, 1, size, inputData->input);

	fclose(inputData->input);

	inputData->input = fopen(inputData->filename, "wb");
	inputData->recording = true;

	if (inputData->input)
	{
		fwrite(fileData, 1, size, inputData->input);
		Msg("[IM] Split from frame %d\n", inputData->frames);
	}
	else
	{
		Warning("CInputManager::Split: failed to create file\n");
	}
}

void CInputManager::Stop(InputData *inputData)
{
	if (!inputData->input)
	{
		Msg("[IM] Not in action\n");
		return;
	}

#pragma warning(push)
#pragma warning(disable: 26451)

	Msg("[IM] Action finished. Frames: %d, time: %.3f\n", inputData->frames, inputData->frames * g_pServerGameDLL->GetTickInterval());

#pragma warning(pop)

	fclose(inputData->input);
	free((void *)inputData->filename);

	inputData->active = false;
	inputData->recording = false;
	inputData->filename = NULL;
	inputData->input = NULL;
}

void CInputManager::ReadInput(InputData *inputData, CUserCmd *pCmd, void *pPlayer, bool bLocalClient)
{
	size_t bytes = fread(&g_inputFrameBuffer, 1, IM_FRAME_SIZE, inputData->input);

	if (bytes != IM_FRAME_SIZE)
	{
		Stop(inputData);
		return;
	}

	pCmd->viewangles.x = g_inputFrameBuffer.viewangles[0];
	pCmd->viewangles.y = g_inputFrameBuffer.viewangles[1];
	pCmd->viewangles.z = g_inputFrameBuffer.viewangles[2];

	pCmd->forwardmove = g_inputFrameBuffer.forwardmove;
	pCmd->sidemove = g_inputFrameBuffer.sidemove;

	pCmd->buttons = g_inputFrameBuffer.buttons;
	pCmd->impulse = g_inputFrameBuffer.impulse;

	pCmd->mousedx = g_inputFrameBuffer.mousedx;
	pCmd->mousedy = g_inputFrameBuffer.mousedy;

	// Selecting weapon
	if (g_inputFrameBuffer.weaponselect > 0)
	{
		if (bLocalClient)
		{
			CBaseHandle *pMyWeapons = reinterpret_cast<CBaseHandle *>(GetOffset(pPlayer, RecvPropOffsets::m_hMyWeapons));

			// Survivors can have 5 items + 1 physics object
			for (int i = 0; i < 6; ++i)
			{
				CBaseHandle hWeapon = pMyWeapons[i];
				C_BaseCombatWeapon *pWeapon = reinterpret_cast<C_BaseCombatWeapon *>(g_pClientEntityList->GetClientEntityFromHandle(hWeapon));

				if (!pWeapon)
					continue;

				int nWeaponID = GetVTableFunction<int (*)()>(pWeapon, Offsets::Functions::C_TerrorWeapon__GetWeaponID)();

				if (nWeaponID == g_inputFrameBuffer.weaponselect)
				{
					pCmd->weaponselect = hWeapon.GetEntryIndex(); // entindex
					pCmd->weaponsubtype = g_inputFrameBuffer.weaponsubtype;
					break;
				}
			}
		}
		else
		{
			CBaseHandle *pMyWeapons = reinterpret_cast<CBaseHandle *>(GetOffset(pPlayer, NetPropOffsets::m_hMyWeapons));

			for (int i = 0; i < 6; ++i)
			{
				CBaseHandle hWeapon = pMyWeapons[i];
				CBaseCombatWeapon *pWeapon = reinterpret_cast<CBaseCombatWeapon *>(HandleToBaseEntity(hWeapon));

				if (!pWeapon)
					continue;

				int nWeaponID = GetVTableFunction<int (*)()>(pWeapon, Offsets::Functions::CTerrorWeapon__GetWeaponID)();

				if (nWeaponID == g_inputFrameBuffer.weaponselect)
				{
					pCmd->weaponselect = hWeapon.GetEntryIndex();
					pCmd->weaponsubtype = g_inputFrameBuffer.weaponsubtype;
					break;
				}
			}
		}
	}
}

void CInputManager::SaveInput(InputData *inputData, CUserCmd *pCmd, bool bLocalClient)
{
	memset(&g_inputFrameBuffer, 0, sizeof(g_inputFrameBuffer));

	g_inputFrameBuffer.viewangles[0] = pCmd->viewangles.x;
	g_inputFrameBuffer.viewangles[1] = pCmd->viewangles.y;
	g_inputFrameBuffer.viewangles[2] = pCmd->viewangles.z;

	g_inputFrameBuffer.forwardmove = pCmd->forwardmove;
	g_inputFrameBuffer.sidemove = pCmd->sidemove;

	g_inputFrameBuffer.buttons = pCmd->buttons;
	g_inputFrameBuffer.impulse = pCmd->impulse;

	g_inputFrameBuffer.mousedx = pCmd->mousedx;
	g_inputFrameBuffer.mousedy = pCmd->mousedy;

	if (pCmd->weaponselect > 0)
	{
		if (bLocalClient)
		{
			C_BaseCombatWeapon *pWeapon = reinterpret_cast<C_BaseCombatWeapon *>(g_pClientEntityList->GetClientEntity(pCmd->weaponselect));

			if (pWeapon)
				g_inputFrameBuffer.weaponselect = GetVTableFunction<int (*)()>(pWeapon, Offsets::Functions::C_TerrorWeapon__GetWeaponID)();
		}
		else
		{
			edict_t *pEdict = EntIndexToEdict(pCmd->weaponselect);
			CBaseCombatWeapon *pWeapon = reinterpret_cast<CBaseCombatWeapon *>(EdictToBaseEntity(pEdict));

			if (pWeapon)
				g_inputFrameBuffer.weaponselect = GetVTableFunction<int (*)()>(pWeapon, Offsets::Functions::CTerrorWeapon__GetWeaponID)();
		}
	}

	fwrite(&g_inputFrameBuffer, IM_FRAME_SIZE, 1, inputData->input);
}