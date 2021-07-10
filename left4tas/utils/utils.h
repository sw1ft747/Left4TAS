// C++
// Utils

#pragma once

#define FAILED_INIT "[L4TAS] %s failed initialization\n"
#define FAILED_IFACE "[L4TAS] Failed to get %s interface\n"

#include "../sdk.h"

extern CGlobalVars *gpGlobals;

void *GetInterface(CreateInterfaceFn interfaceFactory, const char *pszInterfaceVersion);

const wchar_t *CStringToWideCString(const char *pszString);

inline void FailedInit(const char *object)
{
	Warning(FAILED_INIT, object);
}

inline void FailedIFace(const char *object)
{
	Warning(FAILED_IFACE, object);
}

inline bool IsEdictValid(const edict_t *pEdict)
{
	if (!pEdict)
		return false;

	if (!((edict_t *)pEdict)->GetUnknown())
		return false;

	return true;
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
	IServerUnknown *pUnknown = reinterpret_cast<IServerUnknown * >(pEntity);
	CBaseHandle handle = pUnknown->GetRefEHandle();

	if (handle.GetEntryIndex() >= MAX_EDICTS)
		return handle.ToInt() | (1 << 31);
	else
		return handle.GetEntryIndex();
}

inline void *GetVTableFunction(void *pBaseClass, int nIndex)
{
	const DWORD *pVTable = *reinterpret_cast<DWORD **>(pBaseClass);
	return (void *)(pVTable[nIndex]);
}

template <typename T>
inline T GetVTableFunction(void *pBaseClass, int nIndex)
{
	const DWORD *pVTable = *reinterpret_cast<DWORD **>(pBaseClass);
	return reinterpret_cast<T>(pVTable[nIndex]);
}

template <typename T>
inline T GetFunction(void *pFunction)
{
	return reinterpret_cast<T>(pFunction);
}

inline void *GetOffset(void *ptr, DWORD offset)
{
	return (void *)((unsigned char *)ptr + offset);
}