/*
 *  Sine.h
 *  Sine
 *
 *  Created by Farhan on 28/05/2015.
 *  Copyright (c) 2015 Farhan Mannan. All rights reserved.
 *
 */

extern "C" {

#include <CoreFoundation/CoreFoundation.h>
#include "AUInstrumentBase.h"
#include "SineVersion.h"

#pragma GCC visibility push(default)

/* External interface to the Sine, C-based */

CFStringRef SineUUID(void);

#pragma GCC visibility pop
}

class SinSynth : public AUMonotimbralInstrumentBase
{
public:
    SinSynth(AudioUnit inComponentInstance);
    virtual						~SinSynth();
				
    virtual OSStatus			Initialize();
    virtual void				Cleanup();
    virtual OSStatus			Version() { return kSinSynthVersion; }
    
    virtual AUElement*			CreateElement(AudioUnitScope scope, AudioUnitElement element);
    
    virtual OSStatus			GetParameterInfo(AudioUnitScope					inScope,
                                                 AudioUnitParameterID			inParameterID,
                                                 AudioUnitParameterInfo &		outParameterInfo);
    
    MidiControls*				GetControls( MusicDeviceGroupID inChannel)
    {
        SynthGroupElement *group = GetElForGroupID(inChannel);
        return (MidiControls *) group->GetMIDIControlHandler();
    }
    
private:
    
    TestNote mTestNotes[kNumNotes];
};