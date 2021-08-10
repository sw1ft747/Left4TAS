// C++
// Simple Trigger

#include "../offsets.h"

#include "utils.h"
#include "cdll_int.h"
#include "simple_trigger.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern IVEngineClient *g_pEngineClient;
static char s_szExecutionBuffer[48];

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CSimpleTrigger::CSimpleTrigger(const char *pszTriggerOutput, Vector &vecStart, Vector &vecEnd)
{
	m_vecStart = vecStart;
	m_vecEnd = vecEnd;

	m_customData = 0;

	memcpy(m_szTriggerOutput, pszTriggerOutput, strlen(pszTriggerOutput) + 1);
}

bool CSimpleTrigger::Think(Vector *vecPos) const
{
	if (IsPointInBox(vecPos))
	{
		sprintf(s_szExecutionBuffer, "exec %s", m_szTriggerOutput);
		GetVTableFunction<void (__stdcall *)(const char *)>(g_pEngineClient, Offsets::Functions::IVEngineClient__ExecuteClientCmd)(s_szExecutionBuffer);
		return true;
	}
	
	return false;
}

int CSimpleTrigger::GetCustomData() const
{
	return m_customData;
}

void CSimpleTrigger::SetCustomData(int data)
{
	m_customData = data;
}

bool CSimpleTrigger::IsValueInRange(float flValue, float flMin, float flMax) const
{
	return flMax > flMin ? (flValue > flMin && flValue < flMax) : (flValue > flMax && flValue < flMin);
}

bool CSimpleTrigger::IsPointInBox(Vector *vecPos) const
{
	return IsValueInRange(vecPos->x, m_vecStart.x, m_vecEnd.x) &&
		IsValueInRange(vecPos->y, m_vecStart.y, m_vecEnd.y) &&
		IsValueInRange(vecPos->z, m_vecStart.z, m_vecEnd.z);
}