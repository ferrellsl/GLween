// Volume.cpp: the screensaver's own sound volume (see Volume.h).
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdlib.h>
#include <string.h>

// PlaySound()/SND_* are declared here.
#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include "Volume.h"
#include "lighting_wav_data.h"

static const char	*REG_KEY = "Software\\GLween";
static const char	*REG_VALUE = "Volume";
static const int	DEFAULT_VOLUME = 50;			// Percent -- Full Volume Is Startlingly Loud

//////////////////////////////////////////////////////////////////////
// The Saved Setting
//////////////////////////////////////////////////////////////////////

static int ClampPercent(int percent)
{
	if (percent < 0)
		return 0;
	if (percent > 100)
		return 100;
	return percent;
}

int LoadVolumePercent(void)
{
	int percent = DEFAULT_VOLUME;
	HKEY key;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &key) == ERROR_SUCCESS)
	{
		DWORD value = 0, size = sizeof(value), type = 0;
		if (RegQueryValueExA(key, REG_VALUE, NULL, &type, (LPBYTE)&value, &size) == ERROR_SUCCESS
			&& type == REG_DWORD)
			percent = (int)value;
		RegCloseKey(key);
	}
	return ClampPercent(percent);
}

void SaveVolumePercent(int percent)
{
	HKEY key;
	if (RegCreateKeyExA(HKEY_CURRENT_USER, REG_KEY, 0, NULL, 0, KEY_WRITE, NULL, &key, NULL) == ERROR_SUCCESS)
	{
		DWORD value = (DWORD)ClampPercent(percent);
		RegSetValueExA(key, REG_VALUE, 0, REG_DWORD, (const BYTE *)&value, sizeof(value));
		RegCloseKey(key);
	}
}

//////////////////////////////////////////////////////////////////////
// A Copy Of The Sound, Scaled To The Volume
//
// PlaySound() has no volume of its own, and the wave-out volume calls
// change this program's entry in the Windows volume mixer, which Windows
// then remembers. Scaling the samples needs neither.
//////////////////////////////////////////////////////////////////////

static unsigned char	*g_scaled = NULL;			// The Scaled Copy
static int				g_scaledPercent = -1;		// The Volume It Was Made For

static DWORD ReadLE(const unsigned char *p, int bytes)
{
	DWORD value = 0;
	for (int i = bytes - 1; i >= 0; i--)
		value = (value << 8) | p[i];
	return value;
}

static void WriteLE(unsigned char *p, DWORD value, int bytes)
{
	for (int i = 0; i < bytes; i++)
		p[i] = (unsigned char)(value >> (8 * i));
}

// The copy is always 16-bit. The sound itself is 8-bit, and an 8-bit
// sample has only 127 steps each side of silence: turned down to a tenth
// there are a dozen left, and the sound breaks up into crackle. Sixteen
// bits leave thousands.
static const unsigned char *ScaledSound(int percent)
{
	const unsigned char *wav = LIGHTING_WAV_DATA;
	const size_t size = sizeof(LIGHTING_WAV_DATA);
	if (g_scaled && g_scaledPercent == percent)
		return g_scaled;

	// Walk the RIFF chunks: "fmt " describes the samples, "data" holds them.
	DWORD channels = 2, rate = 22050, bitsPerSample = 8;
	size_t dataStart = 0, dataLength = 0;
	size_t pos = 12;												// Past "RIFF" <size> "WAVE"
	while (pos + 8 <= size)
	{
		size_t length = ReadLE(wav + pos + 4, 4);
		size_t body = pos + 8;
		if (memcmp(wav + pos, "fmt ", 4) == 0 && length >= 16 && body + 16 <= size)
		{
			channels = ReadLE(wav + body + 2, 2);
			rate = ReadLE(wav + body + 4, 4);
			bitsPerSample = ReadLE(wav + body + 14, 2);
		}
		else if (memcmp(wav + pos, "data", 4) == 0)
		{
			dataStart = body;
			dataLength = (body + length > size) ? size - body : length;
			break;
		}
		pos = body + length + (length & 1);							// Chunks Are Padded To An Even Length
	}
	if (dataLength == 0 || (bitsPerSample != 8 && bitsPerSample != 16))
		return wav;													// Not A Sound This Can Scale -- Play It As It Is

	const size_t samples = (bitsPerSample == 8) ? dataLength : dataLength / 2;
	const size_t headerSize = 44;
	const size_t outSize = headerSize + samples * 2;

	PlaySound(NULL, NULL, 0);										// Nothing May Still Be Playing From The Old Copy
	if (!g_scaled)
		g_scaled = (unsigned char *)malloc(outSize);
	if (!g_scaled)
		return wav;													// Out Of Memory -- Play It Unscaled
	g_scaledPercent = percent;

	unsigned char *h = g_scaled;									// A Plain 16-Bit PCM Header
	memcpy(h, "RIFF", 4);			WriteLE(h + 4, (DWORD)(outSize - 8), 4);
	memcpy(h + 8, "WAVEfmt ", 8);	WriteLE(h + 16, 16, 4);
	WriteLE(h + 20, 1, 2);											// PCM
	WriteLE(h + 22, channels, 2);
	WriteLE(h + 24, rate, 4);
	WriteLE(h + 28, rate * channels * 2, 4);						// Bytes Per Second
	WriteLE(h + 32, channels * 2, 2);								// Bytes Per Frame
	WriteLE(h + 34, 16, 2);											// Bits Per Sample
	memcpy(h + 36, "data", 4);		WriteLE(h + 40, (DWORD)(samples * 2), 4);

	// Loudness is heard roughly as the square of the amplitude, so the
	// slider's halfway point should sound about half as loud.
	const double gain = (percent / 100.0) * (percent / 100.0);
	const unsigned char *in = wav + dataStart;
	unsigned char *out = g_scaled + headerSize;
	for (size_t i = 0; i < samples; i++)
	{
		int sample = (bitsPerSample == 8)
			? (in[i] - 128) * 256									// 8-Bit Is Unsigned, Silence Is 128
			: (short)ReadLE(in + 2 * i, 2);
		double scaled = sample * gain;
		sample = (int)(scaled < 0 ? scaled - 0.5 : scaled + 0.5);	// Rounded, Not Cut Off
		WriteLE(out + 2 * i, (DWORD)(sample & 0xFFFF), 2);
	}
	return g_scaled;
}

void PlayLightingSound(int percent)
{
	static int savedPercent = -1;									// Read From The Registry Once Per Run
	if (percent < 0)
	{
		if (savedPercent < 0)
			savedPercent = LoadVolumePercent();
		percent = savedPercent;
	}
	percent = ClampPercent(percent);

	if (percent == 0)
	{
		PlaySound(NULL, NULL, 0);									// Silent
		return;
	}
	// SND_MEMORY: play directly from memory instead of a file on disk.
	const unsigned char *sound = (percent == 100) ? LIGHTING_WAV_DATA : ScaledSound(percent);
	PlaySound((LPCSTR)sound, NULL, SND_MEMORY|SND_ASYNC);
}
