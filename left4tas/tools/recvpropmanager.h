// C++
// Receive Properties Manager

#pragma once

#include "dt_recv.h"

class ClientClass;
class IClientEntity;

class CRecvPropManager
{
public: // would implement read/write methods but not needed 
	CRecvPropManager();

	void Init(ClientClass *pClientClassHead);

	void Dump();
	void DumpClass(const char *pszClassname);

	int GetPropOffset(const char *pszDataTable, const char *pszPropName);
	static inline int GetPropOffset(IClientEntity *pEntity, const char *pszPropName);

private:
	static void DumpDataTableInFile(RecvTable *pRecvTable, int nDeep, FILE *file, int nOffset);
	static int GetOffset(RecvTable *pRecvTable, const char *pszPropName);
	static const char *GetPropType(SendPropType type);

	ClientClass *m_pClientClassHead;
};

extern CRecvPropManager RecvProps;