// Volume.h: the screensaver's own sound volume.
//
// The volume is a percentage (0 - 100) chosen in the configuration
// dialog and kept in the registry. It is applied by scaling the sound's
// samples before they are played, so it is separate from the Windows
// volume and from the volume of every other program.
//////////////////////////////////////////////////////////////////////

#pragma once

int		LoadVolumePercent(void);					// The Saved Setting (Or The Default)
void	SaveVolumePercent(int percent);				// Remember A New Setting

// Plays the thunder sound. Pass a percentage to play it at that volume
// (the configuration dialog's Test button), or nothing to play it at
// the saved setting.
void	PlayLightingSound(int percent = -1);
