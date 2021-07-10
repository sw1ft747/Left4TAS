// C++
// Network Properties Manager

#pragma once

#include "dt_send.h"

class ServerClass;
class IServerEntity;

class CNetPropManager
{
public:
	CNetPropManager();

	void Init(ServerClass *pServerClassHead);

	void Dump();
	void DumpClass(const char *pszClassname);

	int GetPropOffset(const char *pszDataTable, const char *pszPropName);
	static inline int GetPropOffset(IServerEntity *pEntity, const char *pszPropName);

private:
	static void DumpDataTableInFile(SendTable *pRecvTable, int nDeep, FILE *file, int nOffset);
	static int GetOffset(SendTable *pRecvTable, const char *pszPropName);
	static const char *GetPropType(SendPropType type);

	ServerClass *m_pServerClassHead;
};

extern CNetPropManager NetProps;