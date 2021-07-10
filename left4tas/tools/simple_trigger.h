// C++
// Simple Trigger

#pragma once

#include "mathlib/vector.h"

#define TRIGGER_OUTPUT_MAXLEN 32

class CSimpleTrigger
{
public:
	CSimpleTrigger(const char *pszTriggerOutput, Vector &vecStart, Vector &vecEnd);
	bool Think(Vector *vecPos);

private:
	bool IsValueInRange(float flValue, float flMin, float flMax);
	bool IsPointInBox(Vector *vecPos);

private:
	Vector m_vecStart;
	Vector m_vecEnd;

	char m_szTriggerOutput[TRIGGER_OUTPUT_MAXLEN];
};