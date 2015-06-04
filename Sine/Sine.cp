/*
 *  Sine.cp
 *  Sine
 *
 *  Created by Farhan on 28/05/2015.
 *  Copyright (c) 2015 Farhan Mannan. All rights reserved.
 *
 */

#include "Sine.h"
#include "SinePriv.h"

CFStringRef SineUUID(void)
{
	CSine* theObj = new CSine;
	return theObj->UUID();
}

CFStringRef CSine::UUID()
{
	return CFSTR("0001020304050607");
}

#pragma mark Sine methods

AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, Sine)

Sine::Sine(AudioUnit inComponentInstance): AUMonotimbralInstrumentBase(inComponentInstance, 0, 1)
{
    CreateElements();
    Globals()->UseIndexedParameters(1);
    Globals()->SetParameter (kGlobalVolumeParam, 1.0);
}

Sine::~Sine() {}