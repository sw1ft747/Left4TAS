// C++
// Receive Properties Manager

#include "../sdk.h"

#include "recvpropmanager.h"
#include "icliententity.h"
#include "client_class.h"

CRecvPropManager RecvProps;

CRecvPropManager::CRecvPropManager() : m_pClientClassHead(NULL)
{
}

void CRecvPropManager::Init(ClientClass *pClientClassHead)
{
	m_pClientClassHead = pClientClassHead;
}

void CRecvPropManager::Dump()
{
	FILE *file = fopen("recvprops.txt", "w");

	if (!file)
	{
		Warning("[CRecvPropManager::DumpClass] Failed to create a dump file\n");
		return;
	}

	for (ClientClass *pClientClassNode = m_pClientClassHead; pClientClassNode; pClientClassNode = pClientClassNode->m_pNext)
	{
		fprintf(file, "%s:\nTable (1 Deep): %s\n", pClientClassNode->GetName(), pClientClassNode->m_pRecvTable->GetName());
		DumpDataTableInFile(pClientClassNode->m_pRecvTable, 2, file, 0);
		fprintf(file, "\n\n");
	}

	fclose(file);

	Msg("All props were dumped in the file 'recvprops.txt'\n");
}

void CRecvPropManager::DumpClass(const char *pszClassname)
{
	for (ClientClass *pClientClassNode = m_pClientClassHead; pClientClassNode; pClientClassNode = pClientClassNode->m_pNext)
	{
		if (!strcmp(pClientClassNode->GetName(), pszClassname))
		{
			FILE *file = fopen("recvprops.txt", "w");

			if (!file)
			{
				Warning("[CRecvPropManager::DumpClass] Failed to create a dump file\n");
				return;
			}

			fprintf(file, "%s:\nTable (1 Deep): %s\n", pClientClassNode->GetName(), pClientClassNode->m_pRecvTable->GetName());
			DumpDataTableInFile(pClientClassNode->m_pRecvTable, 2, file, 0);
			fclose(file);

			Msg("All props of class %s were dumped in the file 'recvprops.txt'\n", pszClassname);

			return;
		}
	}

	Warning("No such class\n");
}

int CRecvPropManager::GetPropOffset(const char *pszClassname, const char *pszPropName)
{
	for (ClientClass *pClientClassNode = m_pClientClassHead; pClientClassNode; pClientClassNode = pClientClassNode->m_pNext)
	{
		if (!strcmp(pClientClassNode->GetName(), pszClassname))
		{
			return CRecvPropManager::GetOffset(pClientClassNode->m_pRecvTable, pszPropName);
		}
	}

	return 0;
}

inline int CRecvPropManager::GetPropOffset(IClientEntity *pEntity, const char *pszPropName)
{
	return CRecvPropManager::GetOffset(pEntity->GetClientClass()->m_pRecvTable, pszPropName);
}

void CRecvPropManager::DumpDataTableInFile(RecvTable *pRecvTable, int nDeep, FILE *file, int nOffset)
{
	int shift = 0;
	char tabs[16]; tabs[0] = 0;

	for (int i = 0; i < nDeep - 2; ++i)
	{
		shift += sprintf(tabs + shift, "\t");
	}

	for (int i = 0; i < pRecvTable->GetNumProps(); ++i)
	{
		RecvProp recvProp = pRecvTable->m_pProps[i];
		RecvTable *pDataTable = recvProp.GetDataTable();

		int currentOffset = recvProp.GetOffset();

		if (pDataTable)
		{
			fprintf(file, "\t%sTable (%d Deep): %s\n", tabs, nDeep, pDataTable->GetName());
			DumpDataTableInFile(pDataTable, nDeep + 1, file, currentOffset + nOffset);
		}
		else
		{
			fprintf(file, "%s-Member: %s (offset %d : %d) (type %s)\n", tabs, recvProp.GetName(), currentOffset, /* real offset: */ currentOffset + nOffset, GetPropType(recvProp.GetType()));
		}
	}
}

int CRecvPropManager::GetOffset(RecvTable *pRecvTable, const char *pszPropName)
{
	for (int i = 0; i < pRecvTable->GetNumProps(); ++i)
	{
		RecvProp recvProp = pRecvTable->m_pProps[i];

		if (!strcmp(recvProp.GetName(), pszPropName))
			return recvProp.GetOffset();

		RecvTable *pDataTable = recvProp.GetDataTable();

		if (pDataTable && !(*pDataTable->m_pProps).GetParentArrayPropName())
		{
			int nOffset = GetOffset(pDataTable, pszPropName);

			if (nOffset)
				return recvProp.GetOffset() + nOffset;
		}
	}

	return 0;
}

const char *CRecvPropManager::GetPropType(SendPropType type)
{
	switch (type)
	{
	case DPT_Int:
		return "integer";

	case DPT_Float:
		return "float";

	case DPT_Vector:
		return "vector";

	case DPT_VectorXY:
		return "vector2D";

	case DPT_String:
		return "string";

	case DPT_Array:
		return "array";

	default:
		return "unknown";
	}
}

CON_COMMAND(dump_recvprops, "Dump in the file 'recvprops.txt' receive properties of all classes")
{
	RecvProps.Dump();
}

CON_COMMAND(dump_recvprops_class, "Dump in the file 'recvprops.txt' receive properties of a given class")
{
	if (args.ArgC() != 2)
	{
		Msg("Usage: dump_recvprops_class [classname]\n");
		return;
	}

	RecvProps.DumpClass(args.Arg(1));
}