/*
 *  Sine.h
 *  Sine
 *
 *  Created by Farhan on 28/05/2015.
 *  Copyright (c) 2015 Farhan Mannan. All rights reserved.
 *
 */

#include "AUInstrumentBase.h"
#include "SineVersion.h"

static const UInt32 kNumNotes = 12;

#pragma mark Sine Parameter Constants

static CFStringRef kParamName_PolyphonyMode = CFSTR ("polyphony mode");
static const int kMonophonic_PolyphonyMode = 1;
static const int kPolyphonic_PolyphonyMode = 2;
static const int kDefaultValue_PolyphonyMode = kMonophonic_PolyphonyMode;

static CFStringRef kMenuItem_Monophonic_PolyphonyMode = CFSTR ("monophonic");
static CFStringRef kMenuItem_Polyphonic_PolyphonyMode = CFSTR ("polyphonic");

enum {
    kParameter_PolyphonyMode = 1, // because kGlobalVolumeParam, 0, is defined in Sine.cp
    kNumberOfParameters = 2
};

struct SineNote : public SynthNote
{
    virtual					~SineNote() {}
    
    virtual bool			Attack(const MusicDeviceNoteParams &inParams)
    {
#if DEBUG_PRINT
        printf("SineNote::Attack %p %d\n", this, GetState());
#endif
        double sampleRate = SampleRate();
        phase = 0.;
        amp = 0.;
        maxamp = 0.4 * pow(inParams.mVelocity/127., 3.); // what the hell's going on here
        up_slope = maxamp / (0.1 * sampleRate);
        dn_slope = -maxamp / (0.9 * sampleRate);
        fast_dn_slope = -maxamp / (0.005 * sampleRate);
        return true;
    }
    virtual void			Kill(UInt32 inFrame); // voice is being stolen.
    virtual void			Release(UInt32 inFrame);
    virtual void			FastRelease(UInt32 inFrame);
    virtual Float32			Amplitude() { return amp; } // used for finding quietest note for voice stealing.
    virtual OSStatus		Render(UInt64 inAbsoluteSampleFrame, UInt32 inNumFrames, AudioBufferList** inBufferList, UInt32 inOutBusCount);
    
    double phase, amp, maxamp;
    double up_slope, dn_slope, fast_dn_slope;
};


class Sine : public AUMonotimbralInstrumentBase
{
public:
    Sine(AudioUnit inComponentInstance);
    virtual						~Sine();
				
    virtual OSStatus			Initialize();
    virtual void				Cleanup();
    virtual OSStatus			Version() { return kSineVersion; }
    
    virtual AUElement*			CreateElement(AudioUnitScope scope, AudioUnitElement element);
    
    virtual OSStatus			GetParameterInfo(AudioUnitScope					inScope,
                                                 AudioUnitParameterID			inParameterID,
                                                 AudioUnitParameterInfo &		outParameterInfo);
    
    virtual	ComponentResult GetParameterValueStrings (
                                                      AudioUnitScope			inScope,
                                                      AudioUnitParameterID	inParameterID,
                                                      CFArrayRef				*outStrings
                                                      );
    
    MidiControls*				GetControls( MusicDeviceGroupID inChannel)
    {
        SynthGroupElement *group = GetElForGroupID(inChannel);
        return (MidiControls *) group->GetMIDIControlHandler();
    }

private:
    
    SineNote mSineNotes[kNumNotes];
};

extern "C" {
    
#include <CoreFoundation/CoreFoundation.h>
    
#pragma GCC visibility push(default)
    
    /* External interface to the Sine, C-based */
    
    CFStringRef SineUUID(void);
    
#pragma GCC visibility pop
}