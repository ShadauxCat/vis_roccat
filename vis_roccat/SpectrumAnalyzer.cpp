/*
Roccat Talk Visualizer Plugin for Winamp
Copyright (c) 2014 Jaedyn Kitt Draper

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "SpectrumAnalyzer.h"
#include "vis.h"
#include <ROCCAT_Talk.h>
#include <stdio.h>
#include <cmath>

CROCCAT_Talk talker;

int columns[23][6] =
{
	{ 96, 77, 60, 38, 16, -1},
	{ 97, 78, 61, 39, 17, 0},
	{ 98, 79, 62, 40, 18, -1},
	{ 99, 80, 63, 41, 19, 1},
	{ 100, 81, 64, 42, 20, 2},
	{ -1, 82, 65, 43, 21, 3},
	{ -1, 83, 66, 44, 22, 4},
	{ -1, 84, 67, 45, 23, -1},
	{ -1, 85, 68, 46, 24, 5},
	{ -1, 86, 69, 47, 25, 6},
	{ -1, 87, 70, 48, 26, 7},
	{ 101,88, 71, 49, 27, 8},
	{ 102,89, 72, 50, 28, 9},
	{ 103,90, 73, 51, 29, 10},
	{ -1, -1, -1, -1, -1, 11},
	{ 104, -1, 52, -1, 30, 12},
	{ 105, -1, -1, 53, 31, 13},
	{ 106, 91, -1, 54, 32, 14},
	{ 107, -1, -1, 55, 33, 15},
	{ 108, 92, 74, 56, 34, -1},
	{ -1, 93, 75, 57, 35, -1},
	{ 109, 94, 76, 58, 36, -1},
	{ 95, -1, 59, -1, 37, -1}
};

/*static*/ void SpectrumAnalyzer::Config(struct winampVisModule* mod)
{
	MessageBox( mod->hwndParent, "Roccat Talk Visualizer\nby Jaedyn K Draper\nhttp://www.github.net/ShadauxCat/vis_roccat", "Roccat Talk Visualizer", MB_OK);
}

/*static*/ int SpectrumAnalyzer::Init(struct winampVisModule* /*mod*/)
{
	if(!talker.init_ryos_talk())
	{
		return 1;
	}
	if(!talker.set_ryos_kb_SDKmode(TRUE))
	{
		return 1;
	}
	return 0;
}

/*static*/ int SpectrumAnalyzer::Render(struct winampVisModule* mod)
{
	BYTE frameData[110];
	memset(frameData, 0, 110);

	for( int talkSlot = 0; talkSlot < 23; ++talkSlot)
	{
		BYTE total = 0;
		int spectrumDataSlot = 0 + (talkSlot * 13);
		int spectrumDataEnd = spectrumDataSlot + 13;
		for( ; spectrumDataSlot < spectrumDataEnd; ++spectrumDataSlot )
		{
			BYTE value = mod->spectrumData[0][spectrumDataSlot];
			if(value > total)
			{
				total = value;
			}
		}
		total *= 13;

		int numSlotsActive = int(double(total)/255.0 * 6.0);
		if(numSlotsActive > 6)
		{
			numSlotsActive = 6;
		}
		for(int row = 0; row < numSlotsActive; ++row)
		{
			int slotToActivate = columns[talkSlot][row];
			if(slotToActivate >= 0)
			{
				frameData[slotToActivate] = 1;
			}
			if(slotToActivate == 11)
			{
				frameData[12] = 1;
			}
		}
	}
	
	BYTE red = 0;
	BYTE green = 0;
	BYTE blue = 0;
	
	int last=mod->waveformData[0][0];
	double total=0;
	for(int waveformSlot = 1; waveformSlot < 576; ++waveformSlot)
	{
		total += abs(last - mod->waveformData[0][waveformSlot]);
		last = mod->waveformData[0][waveformSlot];
	}
	total /= 288;
	if(total > 127)
	{
		total = 127;
	}
	double ratio = double(total)/127.0;
	//The whole ratio just doesn't have enough variation in it for the set of lights the Roccat devices are capable of.
	//Cut it in half since most things seem to stay under 0.5 anyway.
	ratio /= 0.5;
	if(ratio > 1.0)
	{
		ratio = 1.0;
	}

	if(ratio <= 0.2)
	{
		ratio *= 5;
		blue = 255;
		red = BYTE(255 - (255 * ratio));
	}
	else if(ratio <= 0.4)
	{
		ratio -= 0.2;
		ratio *= 5;
		blue = 255;
		green = BYTE(255 * ratio);
	}
	else if(ratio <= 0.6)
	{
		ratio -= 0.4;
		ratio *= 5;
		green = 255;
		blue = BYTE(255 - (255 * ratio));
	}
	else if(ratio <= 0.8)
	{
		ratio -= 0.6;
		ratio *= 5;
		green = 255;
		red = BYTE(255 * ratio);
	}
	else
	{
		ratio -= 0.8;
		ratio *= 5;
		red = 255;
		green = BYTE(255 - (255 * ratio));
	}

	talker.Set_LED_RGB(TALKFX_ZONE_EVENT, TALKFX_EFFECT_ON, TALKFX_SPEED_FAST, red, green, blue);

	talker.Set_all_LEDS(frameData);
	return 0;
}

/*static*/ void SpectrumAnalyzer::Quit(struct winampVisModule* /*mod*/)
{
	talker.RestoreLEDRGB();
	talker.set_ryos_kb_SDKmode(FALSE);
}

winampVisModule SpectrumAnalyzer::RoccatMod =
{
	"Roccat Talk Visualizer",
	NULL,
	NULL,
	0, //Sample rate
	0, //Channel count
	2, // Latency
	67, // Delay between calls
	2, //spectrum analysis channels
	2, //waveform analysis channels
	{0,}, // spectrumData
	{0,}, // waveformData
	&SpectrumAnalyzer::Config,
	&SpectrumAnalyzer::Init,
	&SpectrumAnalyzer::Render,
	&SpectrumAnalyzer::Quit,
};

