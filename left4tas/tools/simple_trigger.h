// C++
// Simple Trigger

#pragma once

#include "mathlib/vector.h"

#define TRIGGER_OUTPUT_MAXLEN 32

class CSimpleTrigger
{
public:
	CSimpleTrigger(const char *pszTriggerOutput, Vector &vecStart, Vector &vecEnd);
	bool Think(Vector *vecPos) const;

	int GetCustomData() const;
	void SetCustomData(int data);

private:
	bool IsValueInRange(float flValue, float flMin, float flMax) const;
	bool IsPointInBox(Vector *vecPos) const;

private:
	Vector m_vecStart;
	Vector m_vecEnd;

	int m_customData;
	char m_szTriggerOutput[TRIGGER_OUTPUT_MAXLEN];
};