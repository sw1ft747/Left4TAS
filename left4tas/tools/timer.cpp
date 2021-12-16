// Timer

#include "timer.h"

#include "../sdk.h"
#include "../cvars.h"

#include "misc.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern ICvar *g_pCVar;
extern CGlobalVars *gpGlobals;

static char buffer[256];
CTimer g_Timer;

//-----------------------------------------------------------------------------
// Convert the given time in timer format (minutes:seconds,milliseconds)
//-----------------------------------------------------------------------------

static inline void ConvertToTimerFormat(float flTime)
{
	static char buff[24];

	int minutes = static_cast<int>(flTime) / 60.0;
	int seconds = static_cast<int>(flTime) % 60;
	float temp = flTime - static_cast<int>(flTime);

	sprintf(buff, "%.3f", temp);

	int milliseconds = atol(buff + 2);

	sprintf(buffer, "%s%d:%s%d,%s%d", minutes < 10 ? "0" : "",
									  minutes,
									  seconds < 10 ? "0" : "",
									  seconds,
									  milliseconds < 10 ? "00" : milliseconds < 100 ? "0" : "",
									  milliseconds);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CTimer::CTimer() : m_bTimerStarted(false),
					m_bTotalTimeProcessed(false),
					m_bPreciseTimeChanged(false),
					m_nStartTick(0),
					m_flStartTime(0.0f),
					m_flStartPreciseTime(0.0f),
					m_flSegmentTime(0.0f),
					m_flSegmentPreciseTime(0.0f)
{
}

void CTimer::Start()
{
	if (m_bTimerStarted)
		return;

	m_nStartTick = gpGlobals->tickcount;
	m_flStartTime = gpGlobals->curtime;
	m_flStartPreciseTime = static_cast<float>(Plat_FloatTime());

	m_bTimerStarted = true;
	m_bTotalTimeProcessed = m_bPreciseTimeChanged = false;

	extern ConVar *host_timescale;

	if (host_timescale->GetFloat() != 1.0f)
		m_bPreciseTimeChanged = true;
}

void CTimer::Stop()
{
	if (!m_bTimerStarted)
		return;

	m_nTotalTicks = gpGlobals->tickcount - m_nStartTick;
	m_flTotalTime = gpGlobals->curtime - m_flStartTime;
	m_flTotalPreciseTime = static_cast<float>(Plat_FloatTime()) - m_flStartPreciseTime;

	m_bTimerStarted = false;
	m_bTotalTimeProcessed = true;
}

void CTimer::Reset()
{
	m_nTotalTicks = 0;
	m_flTotalTime = m_flTotalPreciseTime = 0.0f;
	m_bTimerStarted = m_bTotalTimeProcessed = m_bPreciseTimeChanged = false;
}

bool CTimer::SetSegmentTime(float flSegmentTime, float flSegmentPreciseTime)
{
	if (flSegmentTime >= 0.0f && flSegmentPreciseTime >= 0.0f)
	{
		m_flSegmentTime = flSegmentTime;
		m_flSegmentPreciseTime = flSegmentPreciseTime;

		return true;
	}

	return false;
}

float CTimer::GetSegmentTime(bool bPreciseTime /* = false */)
{
	return bPreciseTime ? m_flSegmentPreciseTime : m_flSegmentTime;
}

void CTimer::PrintTicks() const
{
	if (m_bTimerStarted || m_bTotalTimeProcessed)
	{
		unsigned int nTicks = m_nTotalTicks;

		if (!m_bTotalTimeProcessed)
			nTicks = gpGlobals->tickcount - m_nStartTick;
		
		sprintf(buffer, "\x05> \x04Total ticks: \x03%lu", nTicks);
		ClientPrint(-1, timer_print_to_chat.GetBool() ? 3 : 2, buffer);
	}
}

void CTimer::PrintTime() const
{
	if (m_bTimerStarted || m_bTotalTimeProcessed)
	{
		char buffer_time[64];
		char buffer_preciseTime[64];

		int print_dest = timer_print_to_chat.GetBool() ? 3 : 2;
		float flTime, flTotalTime, flPreciseTime, flTotalPreciseTime;

		flTime = flTotalTime = m_flTotalTime;
		flPreciseTime = flTotalPreciseTime = m_flTotalPreciseTime;

		if (!m_bTotalTimeProcessed)
		{
			flTime = flTotalTime = gpGlobals->curtime - m_flStartTime;
			flPreciseTime = flTotalPreciseTime = static_cast<float>(Plat_FloatTime()) - m_flStartPreciseTime;
		}

		if (m_flSegmentTime > 0.0 && m_flSegmentPreciseTime > 0.0)
		{
			flTotalTime += m_flSegmentTime;

			GetTimeInTimerFormat(buffer_time, ARRAYSIZE(buffer_time), &flTotalTime, true);
			sprintf(buffer, "\x05> \x04Total time: \x03%.3f \x05%s", flTotalTime, buffer_time);

			ClientPrint(-1, print_dest, buffer);

			if (!IsPreciseTimeCorrupted())
			{
				flTotalPreciseTime += m_flSegmentPreciseTime;

				GetPreciseTimeInTimerFormat(buffer_preciseTime, ARRAYSIZE(buffer_preciseTime), &flTotalPreciseTime, true);
				sprintf(buffer, "\x04Precise total time: \x03%.3f \x05%s", flTotalPreciseTime, buffer_preciseTime);

				ClientPrint(-1, print_dest, buffer);
			}
		}

		GetTimeInTimerFormat(buffer_time, ARRAYSIZE(buffer_time), &flTime, false);
		sprintf(buffer, "\x05> \x04Segment time: \x03%.3f \x05%s", flTime, buffer_time);

		ClientPrint(-1, print_dest, buffer);

		if (!IsPreciseTimeCorrupted())
		{
			GetPreciseTimeInTimerFormat(buffer_preciseTime, ARRAYSIZE(buffer_preciseTime), &flPreciseTime, false);
			sprintf(buffer, "\x04Precise time: \x03%.3f \x05%s", flPreciseTime, buffer_preciseTime);

			ClientPrint(-1, print_dest, buffer);
		}
	}
}

float CTimer::GetTime(bool bPreciseTime /* = false */) const
{
	if (!m_bTimerStarted && !m_bTotalTimeProcessed)
		return 0.0f;

	if (bPreciseTime)
	{
		float flTime = m_flTotalPreciseTime;

		if (!m_bTotalTimeProcessed)
			flTime = static_cast<float>(Plat_FloatTime()) - m_flStartPreciseTime;

		return flTime;
	}
	else
	{
		float flTime = m_flTotalTime;

		if (!m_bTotalTimeProcessed)
			flTime = gpGlobals->curtime - m_flStartTime;

		return flTime;
	}
}

bool CTimer::GetTimeInTimerFormat(char *buff, size_t bufferSize, float *calcTime, bool bReturnTotalTime = false) const
{
	if (!calcTime)
	{
		float flTime = m_flTotalTime;

		if (!m_bTotalTimeProcessed)
		{
			if (m_bTimerStarted)
				flTime = gpGlobals->curtime - m_flStartTime;
			else
				flTime = 0.0f;
		}

		if (bReturnTotalTime)
			flTime += m_flSegmentTime;

		ConvertToTimerFormat(flTime);
	}
	else
	{
		ConvertToTimerFormat(*calcTime);
	}

	size_t length = strlen(buffer) + 1;

	if (length > bufferSize)
		return false;

	memcpy(buff, buffer, length);

	return true;
}

bool CTimer::GetPreciseTimeInTimerFormat(char *buff, size_t bufferSize, float *calcTime, bool bReturnTotalTime = false) const
{
	if (!calcTime)
	{
		float flTime = m_flTotalPreciseTime;

		if (!m_bTotalTimeProcessed)
		{
			if (m_bTimerStarted)
				flTime = static_cast<float>(Plat_FloatTime()) - m_flStartPreciseTime;
			else
				flTime = 0.0f;
		}

		if (bReturnTotalTime)
			flTime += m_flSegmentPreciseTime;

		ConvertToTimerFormat(flTime);
	}
	else
	{
		ConvertToTimerFormat(*calcTime);
	}

	size_t length = strlen(buffer) + 1;

	if (length > bufferSize)
		return false;

	memcpy(buff, buffer, length);

	return true;
}

bool CTimer::IsStarted() const
{
	return m_bTimerStarted;
}

void CTimer::OnPreciseTimeCorrupted()
{
	if (IsStarted())
		m_bPreciseTimeChanged = true;
}

bool CTimer::IsPreciseTimeCorrupted() const
{
	return m_bPreciseTimeChanged;
}