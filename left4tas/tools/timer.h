// Timer

#pragma once

class CTimer
{
public:
	CTimer();

	void Start();
	void Stop();
	void Reset();

	void OnPreciseTimeCorrupted();

	bool SetSegmentTime(float flSegmentTime, float flSegmentPreciseTime);
	float GetSegmentTime(bool bPreciseTime = false);

	void PrintTicks() const;
	void PrintTime() const;

	float GetTime(bool bPreciseTime = false) const;
	bool GetTimeInTimerFormat(char *buff, size_t bufferSize, float *calcTime, bool bReturnTotalTime) const;
	bool GetPreciseTimeInTimerFormat(char *buff, size_t bufferSize, float *calcTime, bool bReturnTotalTime) const;

	bool IsStarted() const;

protected:
	bool IsPreciseTimeCorrupted() const;

private:
	union
	{
		float m_flStartTime;
		float m_flTotalTime;
	};

	union
	{
		float m_flStartPreciseTime;
		float m_flTotalPreciseTime;
	};

	union
	{
		unsigned int m_nStartTick;
		unsigned int m_nTotalTicks;
	};

	float m_flSegmentTime;
	float m_flSegmentPreciseTime;

	bool m_bTimerStarted;
	bool m_bTotalTimeProcessed;
	bool m_bPreciseTimeChanged;
};

extern CTimer g_Timer;