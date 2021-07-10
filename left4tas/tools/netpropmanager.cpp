// C++
// Network Properties Manager

#include "../sdk.h"

#include "netpropmanager.h"
#include "iserverentity.h"
#include "server_class.h"

SendProp::~SendProp()
{
}

CNetPropManager NetProps;

CNetPropManager::CNetPropManager() : m_pServerClassHead(NULL)
{
}

void CNetPropManager::Init(ServerClass *pServerClassHead)
{
	m_pServerClassHead = pServerClassHead;
}

void CNetPropManager::Dump()
{
	FILE *file = fopen("netprops.txt", "w");

	if (!file)
	{
		Warning("[CNetPropManager::DumpClass] Failed to create a dump file\n");
		return;
	}

	for (ServerClass *pServerClassNode = m_pServerClassHead; pServerClassNode; pServerClassNode = pServerClassNode->m_pNext)
	{
		fprintf(file, "%s:\nTable (1 Deep): %s\n", pServerClassNode->GetName(), pServerClassNode->m_pTable->GetName());
		DumpDataTableInFile(pServerClassNode->m_pTable, 2, file, 0);
		fprintf(file, "\n\n");
	}

	fclose(file);

	Msg("All props were dumped in the file 'netprops.txt'\n");
}

void CNetPropManager::DumpClass(const char *pszClassname)
{
	for (ServerClass *pServerClassNode = m_pServerClassHead; pServerClassNode; pServerClassNode = pServerClassNode->m_pNext)
	{
		if (!strcmp(pServerClassNode->GetName(), pszClassname))
		{
			FILE *file = fopen("netprops.txt", "w");

			if (!file)
			{
				Warning("[CNetPropManager::DumpClass] Failed to create a dump file\n");
				return;
			}

			fprintf(file, "%s:\nTable (1 Deep): %s\n", pServerClassNode->GetName(), pServerClassNode->m_pTable->GetName());
			DumpDataTableInFile(pServerClassNode->m_pTable, 2, file, 0);
			fclose(file);

			Msg("All props of class %s were dumped in the file 'netprops.txt'\n", pszClassname);

			return;
		}
	}

	Warning("No such class\n");
}

int CNetPropManager::GetPropOffset(const char *pszClassname, const char *pszPropName)
{
	for (ServerClass *pServerClassNode = m_pServerClassHead; pServerClassNode; pServerClassNode = pServerClassNode->m_pNext)
	{
		if (!strcmp(pServerClassNode->GetName(), pszClassname))
		{
			return CNetPropManager::GetOffset(pServerClassNode->m_pTable, pszPropName);
		}
	}

	return 0;
}

inline int CNetPropManager::GetPropOffset(IServerEntity *pEntity, const char *pszPropName)
{
	return CNetPropManager::GetOffset(pEntity->GetNetworkable()->GetServerClass()->m_pTable, pszPropName);
}

void CNetPropManager::DumpDataTableInFile(SendTable *pSendTable, int nDeep, FILE *file, int nOffset)
{
	int shift = 0;
	char tabs[16]; tabs[0] = 0;

	for (int i = 0; i < nDeep - 2; ++i)
	{
		shift += sprintf(tabs + shift, "\t");
	}

	for (int i = 0; i < pSendTable->GetNumProps(); ++i)
	{
		SendProp sendProp = pSendTable->m_pProps[i];
		SendTable *pDataTable = sendProp.GetDataTable();

		int currentOffset = sendProp.GetOffset();

		if (pDataTable)
		{
			fprintf(file, "\t%sTable (%d Deep): %s\n", tabs, nDeep, pDataTable->GetName());
			DumpDataTableInFile(pDataTable, nDeep + 1, file, currentOffset + nOffset);
		}
		else
		{
			fprintf(file, "%s-Member: %s (offset %d : %d) (type %s)\n", tabs, sendProp.GetName(), currentOffset, /* real offset: */ currentOffset + nOffset, GetPropType(sendProp.GetType()));
		}
	}
}

int CNetPropManager::GetOffset(SendTable *pSendTable, const char *pszPropName)
{
	for (int i = 0; i < pSendTable->GetNumProps(); ++i)
	{
		SendProp sendProp = pSendTable->m_pProps[i];

		if (!strcmp(sendProp.GetName(), pszPropName))
			return sendProp.GetOffset();

		SendTable *pDataTable = sendProp.GetDataTable();

		if (pDataTable && !(*pDataTable->m_pProps).GetParentArrayPropName())
		{
			int nOffset = GetOffset(pDataTable, pszPropName);

			if (nOffset)
				return sendProp.GetOffset() + nOffset;
		}
	}

	return 0;
}

const char *CNetPropManager::GetPropType(SendPropType type)
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

CON_COMMAND(dump_netprops, "Dump in the file 'netprops.txt' network properties of all classes")
{
	NetProps.Dump();
}

CON_COMMAND(dump_netprops_class, "Dump in the file 'netprops.txt' network properties of a given class")
{
	if (args.ArgC() != 2)
	{
		Msg("Usage: dump_netprops_class [classname]\n");
		return;
	}

	NetProps.DumpClass(args.Arg(1));
}