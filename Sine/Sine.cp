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

static const UInt32 kMaxActiveNotes = 8;

const double twopi = 2.0 * 3.14159265358979;

#pragma mark Sine methods

AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, Sine)

static const AudioUnitParameterID kGlobalVolumeParam = 0;
static const CFStringRef kGlobalVolumeName = CFSTR("global volume");

Sine::Sine(AudioUnit inComponentInstance): AUMonotimbralInstrumentBase(inComponentInstance, 0, 1)
{
    CreateElements();
    Globals()->UseIndexedParameters(1);
    Globals()->SetParameter (kGlobalVolumeParam, 1.0);
}

Sine::~Sine() {}

void Sine::Cleanup()
{
#if DEBUG_PRINT
    printf("Sine::Cleanup\n");
#endif
}

OSStatus Sine::Initialize()
{
#if DEBUG_PRINT
    printf("->Sine::Initialize\n");
#endif
    AUMonotimbralInstrumentBase::Initialize();
    
    SetNotes(kNumNotes, kMaxActiveNotes, mSineNotes, sizeof(SineNote));
#if DEBUG_PRINT
    printf("<-Sine::Initialize\n");
#endif
    
    return noErr;
}

AUElement* Sine::CreateElement(	AudioUnitScope					scope,
                                   AudioUnitElement				element)
{
    switch (scope)
    {
        case kAudioUnitScope_Group :
            return new SynthGroupElement(this, element, new MidiControls);
        case kAudioUnitScope_Part :
            return new SynthPartElement(this, element);
        default :
            return AUBase::CreateElement(scope, element);
    }
}

OSStatus			Sine::GetParameterInfo(		AudioUnitScope					inScope,
                                               AudioUnitParameterID			inParameterID,
                                               AudioUnitParameterInfo &		outParameterInfo)
{
    if (inParameterID != kGlobalVolumeParam) return kAudioUnitErr_InvalidParameter;
    if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
    
    outParameterInfo.flags = SetAudioUnitParameterDisplayType (0, kAudioUnitParameterFlag_DisplaySquareRoot);
    outParameterInfo.flags += kAudioUnitParameterFlag_IsWritable;
    outParameterInfo.flags += kAudioUnitParameterFlag_IsReadable;
    
    AUBase::FillInParameterName (outParameterInfo, kGlobalVolumeName, false);
    outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
    outParameterInfo.minValue = 0;
    outParameterInfo.maxValue = 1.0;
    outParameterInfo.defaultValue = 1.0;
    return noErr;
}

#pragma mark SineNote Methods

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void			SineNote::Release(UInt32 inFrame)
{
    SynthNote::Release(inFrame);
#if DEBUG_PRINT
    printf("SineNote::Release %p %d\n", this, GetState());
#endif
}

void			SineNote::FastRelease(UInt32 inFrame) // voice is being stolen.
{
    SynthNote::Release(inFrame);
#if DEBUG_PRINT
    printf("SineNote::Release %p %d\n", this, GetState());
#endif
}

void			SineNote::Kill(UInt32 inFrame) // voice is being stolen.
{
    SynthNote::Kill(inFrame);
#if DEBUG_PRINT
    printf("SineNote::Kill %p %d\n", this, GetState());
#endif
}

OSStatus		SineNote::Render(UInt64 inAbsoluteSampleFrame, UInt32 inNumFrames, AudioBufferList** inBufferList, UInt32 inOutBusCount)
{
    float *left, *right;
    /* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
     Changes to this parameter (kGlobalVolumeParam) are not being de-zippered;
     Left as an exercise for the reader
     ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
    float globalVol = GetGlobalParameter(kGlobalVolumeParam);
    
    // SineNote only writes into the first bus regardless of what is handed to us.
    const int bus0 = 0;
    int numChans = inBufferList[bus0]->mNumberBuffers;
    if (numChans > 2) return -1;
    
    left = (float*)inBufferList[bus0]->mBuffers[0].mData;
    right = numChans == 2 ? (float*)inBufferList[bus0]->mBuffers[1].mData : 0;
    
    double sampleRate = SampleRate();
    double freq = Frequency() * (twopi/sampleRate);
    
    
#if DEBUG_PRINT_RENDER
    printf("SineNote::Render %p %d %g %g\n", this, GetState(), phase, amp);
#endif
    
    // farhan added this
    amp = maxamp;
    
    switch (GetState())
    {
        case kNoteState_Attacked :
        case kNoteState_Sostenutoed :
        case kNoteState_ReleasedButSostenutoed :
        case kNoteState_ReleasedButSustained :
        {
            for (UInt32 frame=0; frame<inNumFrames; ++frame)
            {
//                if (amp < maxamp) amp += up_slope; remove this for that clean sin sound! ayy
                float out = sin(phase) * amp * globalVol;
                phase += freq;
                if (phase > twopi) phase -= twopi;
                left[frame] += out;
                if (right) right[frame] += out;
            }
        }
            break;
            
        case kNoteState_Released :
        {
            UInt32 endFrame = 0xFFFFFFFF;
            for (UInt32 frame=0; frame<inNumFrames; ++frame)
            {
                if (amp > 0.0) amp += fast_dn_slope; // farhan changed this to be fast dn not just dn
                if (endFrame == 0xFFFFFFFF) endFrame = frame;
                float out = sin(phase) * amp * globalVol;
                phase += freq;
                left[frame] += out;
                if (right) right[frame] += out;
            }
            if (endFrame != 0xFFFFFFFF) {
#if DEBUG_PRINT
                printf("SineNote::NoteEnded  %p %d %g %g\n", this, GetState(), phase, amp);
#endif
                NoteEnded(endFrame);
            }
        }
            break;
            
        case kNoteState_FastReleased :
        {
            UInt32 endFrame = 0xFFFFFFFF;
            for (UInt32 frame=0; frame<inNumFrames; ++frame)
            {
                if (amp > 0.0) amp += fast_dn_slope;
                else if (endFrame == 0xFFFFFFFF) endFrame = frame;
                float out = sin(phase) * amp * globalVol;
                phase += freq;
                left[frame] += out;
                if (right) right[frame] += out;
            }
            if (endFrame != 0xFFFFFFFF) {
#if DEBUG_PRINT
                printf("SineNote::NoteEnded  %p %d %g %g\n", this, GetState(), phase, amp);
#endif
                NoteEnded(endFrame);
            }
        }
            break;
        default :
            break;
    }
    return noErr;
}

CFStringRef SineUUID(void)
{
    CSine* theObj = new CSine;
    return theObj->UUID();
}

CFStringRef CSine::UUID()
{
    return CFSTR("0001020304050607");
}