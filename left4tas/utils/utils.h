// C++
// Utils

#pragma once

#define FAILED_INIT "[L4TAS] %s failed initialization\n"
#define FAILED_IFACE "[L4TAS] Failed to get %s interface\n"

#define M_SWAP(a,b,temp) ((temp)=(a),(a)=(b),(b)=(temp))

#include "../sdk.h"
#include "icliententity.h"

//-----------------------------------------------------------------------------

extern CGlobalVars *gpGlobals;

extern bool g_bFailedInit;

//-----------------------------------------------------------------------------

void *GetInterface(CreateInterfaceFn interfaceFactory, const char *pszInterfaceVersion);

const wchar_t *CStringToWideCString(const char *pszString);

//-----------------------------------------------------------------------------

inline void FailedInit(const char *object)
{
	Warning(FAILED_INIT, object);
	g_bFailedInit = true;
}

inline void FailedIFace(const char *object)
{
	Warning(FAILED_IFACE, object);
	g_bFailedInit = true;
}

inline void *GetVTableFunction(void *pBaseClass, int nIndex)
{
	const unsigned long *pVTable = *reinterpret_cast<unsigned long **>(pBaseClass);
	return (void *)(pVTable[nIndex]);
}

template <typename T>
inline T GetVTableFunction(void *pBaseClass, int nIndex)
{
	const unsigned long *pVTable = *reinterpret_cast<unsigned long **>(pBaseClass);
	return reinterpret_cast<T>(pVTable[nIndex]);
}

template <typename T>
inline T GetFunction(void *pFunction)
{
	return reinterpret_cast<T>(pFunction);
}

inline void *GetOffset(void *ptr, unsigned long offset)
{
	return (void *)((unsigned char *)ptr + offset);
}

inline void *GetFunctionAddress(void *pCallAddress /* Including CALL opcode */)
{
	unsigned long dwRelativeAddress = *reinterpret_cast<unsigned long *>(GetOffset(pCallAddress, 1));
	return (void *)(dwRelativeAddress + (unsigned long)pCallAddress + sizeof(void *) + 1);
}

inline bool IsEdictValid(const edict_t *pEdict)
{
	return pEdict && ((edict_t *)pEdict)->GetUnknown();
}

inline int EdictToEntIndex(const edict_t *pEdict)
{
	return (int)(pEdict - gpGlobals->pEdicts);
}

inline CBaseEntity *EdictToBaseEntity(edict_t *pEdict)
{
	return pEdict->GetUnknown()->GetBaseEntity();
}

inline edict_t *EntIndexToEdict(const int nIndex)
{
	if (nIndex > 0 && nIndex <= gpGlobals->maxEntities)
		return (edict_t *)(gpGlobals->pEdicts + nIndex);

	return NULL;
}

inline edict_t *BaseEntityToEdict(CBaseEntity *pEntity)
{
	IServerUnknown *pUnknown = reinterpret_cast<IServerUnknown *>(pEntity);
	IServerNetworkable *pNetworkable = pUnknown->GetNetworkable();

	if (pNetworkable)
		return pNetworkable->GetEdict();

	return NULL;
}

inline int EntIndexOfBaseEntity(CBaseEntity *pEntity)
{
	IServerUnknown *pUnknown = reinterpret_cast<IServerUnknown *>(pEntity);
	CBaseHandle handle = pUnknown->GetRefEHandle();

	if (handle.GetEntryIndex() >= MAX_EDICTS)
		return handle.ToInt() | (1 << 31);
	else
		return handle.GetEntryIndex();
}

inline int EntIndexOfBaseEntity(IClientEntity *pEntity)
{
	IClientNetworkable *pNetworkable = pEntity->GetClientNetworkable();

	if (pNetworkable)
		return GetVTableFunction<int (__thiscall *)(void *)>(pNetworkable, 8)(pNetworkable); // Fix SDK

	return -1;
}

inline CBaseEntity *HandleToBaseEntity(CBaseHandle handle)
{
	int nIndex;

	if (!handle.IsValid())
		return NULL;

	if (handle.GetEntryIndex() >= MAX_EDICTS)
		nIndex = handle.ToInt() | (1 << 31);
	else
		nIndex = handle.GetEntryIndex();

	edict_t *pEdict = EntIndexToEdict(nIndex);

	if (IsEdictValid(pEdict))
		return EdictToBaseEntity(pEdict);

	return NULL;
}