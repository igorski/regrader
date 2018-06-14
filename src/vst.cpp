/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - https://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "global.h"
#include "vst.h"
#include "paramids.h"
#include "calc.h"

#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/vstpresetkeys.h"

#include <stdio.h>

namespace Igorski {

//------------------------------------------------------------------------
// Regrader Implementation
//------------------------------------------------------------------------
Regrader::Regrader()
: fDelayTime( 0.125f )
, fDelayHostSync( 1.f )
, fDelayFeedback( 0.2f )
, fDelayMix( .5f )
, fBitResolution( 1.f )
, fBitResolutionChain( 1.f )
, fLFOBitResolution( .0f )
, fLFOBitResolutionDepth( .75f )
, fDecimator( 1.f )
, fLFODecimator( 0.f )
, fDecimatorChain( 0.f )
, fFilterChain( 1.f )
, fFilterCutoff( .5f )
, fFilterResonance( 1.f )
, fLFOFilter( 0.f )
, fLFOFilterDepth( 0.5f )
, fFlangerChain( 0.f )
, fFlangerRate( 0.f )
, fFlangerWidth( 0.f )
, fFlangerFeedback( 0.f )
, fFlangerDelay( 0.f )
, regraderProcess( 0 )
, outputGainOld( 0.f )
, currentProcessMode( -1 ) // -1 means not initialized
{
    // register its editor class (the same as used in vstentry.cpp)
    setControllerClass( VST::RegraderControllerUID );
}

//------------------------------------------------------------------------
Regrader::~Regrader()
{
    // free all allocated resources
    delete regraderProcess;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::initialize( FUnknown* context )
{
    //---always initialize the parent-------
    tresult result = AudioEffect::initialize( context );
    // if everything Ok, continue
    if ( result != kResultOk )
        return result;

    //---create Audio In/Out buses------
    addAudioInput ( STR16( "Stereo In" ),  SpeakerArr::kStereo );
    addAudioOutput( STR16( "Stereo Out" ), SpeakerArr::kStereo );

    //---create Event In/Out buses (1 bus with only 1 channel)------
    addEventInput( STR16( "Event In" ), 1 );

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::terminate()
{
    // nothing to do here yet...except calling our parent terminate
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::setActive (TBool state)
{
    if (state)
        sendTextMessage( "Regrader::setActive (true)" );
    else
        sendTextMessage( "Regrader::setActive (false)" );

    // reset output level meter
    outputGainOld = 0.f;

    // call our parent setActive
    return AudioEffect::setActive( state );
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::process( ProcessData& data )
{
    // In this example there are 4 steps:
    // 1) Read inputs parameters coming from host (in order to adapt our model values)
    // 2) Read inputs events coming from host (note on/off events)
    // 3) Apply the effect using the input buffer into the output buffer

    //---1) Read input parameter changes-----------
    IParameterChanges* paramChanges = data.inputParameterChanges;
    if ( paramChanges )
    {
        int32 numParamsChanged = paramChanges->getParameterCount();
        // for each parameter which are some changes in this audio block:
        for ( int32 i = 0; i < numParamsChanged; i++ )
        {
            IParamValueQueue* paramQueue = paramChanges->getParameterData( i );
            if ( paramQueue )
            {
                ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount();
                switch ( paramQueue->getParameterId())
                {
                    // we use in this example only the last point of the queue.
                    // in some wanted case for specific kind of parameter it makes sense to retrieve all points
                    // and process the whole audio block in small blocks.

                    case kDelayTimeId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayTime = ( float ) value;
                        break;

                    case kDelayHostSyncId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayHostSync = ( float ) value;
                        break;

                    case kDelayFeedbackId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayFeedback = ( float ) value;
                        break;

                    case kDelayMixId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayMix = ( float ) value;
                        break;

                    case kBitResolutionId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fBitResolution = ( float ) value;
                        break;

                    case kBitResolutionChainId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fBitResolutionChain = ( float ) value;
                        break;

                    case kLFOBitResolutionId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fLFOBitResolution = ( float ) value;
                        break;

                    case kLFOBitResolutionDepthId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fLFOBitResolutionDepth = ( float ) value;
                        break;

                    case kDecimatorId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDecimator = ( float ) value;
                        break;

                    case kDecimatorChainId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDecimatorChain = ( float ) value;
                        break;

                    case kLFODecimatorId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fLFODecimator = ( float ) value;
                        break;

                    case kFilterChainId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFilterChain = ( float ) value;
                        break;

                    case kFilterCutoffId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFilterCutoff = ( float ) value;
                        break;

                    case kFilterResonanceId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFilterResonance = ( float ) value;
                        break;

                    case kLFOFilterId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fLFOFilter = ( float ) value;
                        break;

                    case kLFOFilterDepthId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fLFOFilterDepth = ( float ) value;
                        break;

                    case kFlangerChainId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFlangerChain = ( float ) value;
                        break;

                    case kFlangerRateId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFlangerRate = ( float ) value;
                        break;

                    case kFlangerWidthId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFlangerWidth = ( float ) value;
                        break;

                    case kFlangerFeedbackId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFlangerFeedback = ( float ) value;
                        break;

                    case kFlangerDelayId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFlangerDelay = ( float ) value;
                        break;
                }
                syncModel();
            }
        }
    }

    // according to docs: processing context (optional, but most welcome)

    if ( data.processContext != nullptr ) {
        regraderProcess->setTempo(
            data.processContext->tempo, data.processContext->timeSigNumerator, data.processContext->timeSigDenominator
        );
    }

    //---2) Read input events-------------
//    IEventList* eventList = data.inputEvents;


    //-------------------------------------
    //---3) Process Audio---------------------
    //-------------------------------------

    if ( data.numInputs == 0 || data.numOutputs == 0 || regraderProcess == 0 )
    {
        // nothing to do
        return kResultOk;
    }

    int32 numInChannels  = data.inputs[ 0 ].numChannels;
    int32 numOutChannels = data.outputs[ 0 ].numChannels;

    // --- get audio buffers----------------
    uint32 sampleFramesSize = getSampleFramesSizeInBytes( processSetup, data.numSamples );
    void** in  = getChannelBuffersPointer( processSetup, data.inputs [ 0 ] );
    void** out = getChannelBuffersPointer( processSetup, data.outputs[ 0 ] );

    // process the incoming sound!

    bool isDoublePrecision = ( data.symbolicSampleSize == kSample64 );

    if ( isDoublePrecision ) {
        // 64-bit samples, e.g. Reaper64
        regraderProcess->process<double>(
            ( double** ) in, ( double** ) out, numInChannels, numOutChannels,
            data.numSamples, sampleFramesSize
        );
    }
    else {
        // 32-bit samples, e.g. Ableton Live, Bitwig Studio... (oddly enough also when 64-bit?)
        regraderProcess->process<float>(
            ( float** ) in, ( float** ) out, numInChannels, numOutChannels,
            data.numSamples, sampleFramesSize
        );
    }

    // output flags

    data.outputs[ 0 ].silenceFlags = false; // there should always be output
    float outputGain = regraderProcess->limiter->getLinearGR();

    //---4) Write output parameter changes-----------
    IParameterChanges* outParamChanges = data.outputParameterChanges;
    // a new value of VuMeter will be sent to the host
    // (the host will send it back in sync to our controller for updating our editor)
    if ( !isDoublePrecision && outParamChanges && outputGainOld != outputGain ) {
        int32 index = 0;
        IParamValueQueue* paramQueue = outParamChanges->addParameterData( kVuPPMId, index );
        if ( paramQueue )
            paramQueue->addPoint( 0, outputGain, index );
    }
    outputGainOld = outputGain;
    return kResultOk;
}

//------------------------------------------------------------------------
tresult Regrader::receiveText( const char* text )
{
    // received from Controller
    fprintf( stderr, "[Regrader] received: " );
    fprintf( stderr, "%s", text );
    fprintf( stderr, "\n" );

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::setState( IBStream* state )
{
    // called when we load a preset, the model has to be reloaded

    float savedDelayTime = 0.f;
    if ( state->read( &savedDelayTime, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedDelayHostSync = 0.f;
    if ( state->read( &savedDelayHostSync, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedDelayFeedback = 0.f;
    if ( state->read( &savedDelayFeedback, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedDelayMix = 0.f;
    if ( state->read( &savedDelayMix, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedBitResolution = 0.f;
    if ( state->read( &savedBitResolution, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedBitResolutionChain = 0.f;
    if ( state->read( &savedBitResolutionChain, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedLFOBitResolution = 0.f;
    if ( state->read( &savedLFOBitResolution, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedLFOBitResolutionDepth = 0.f;
    if ( state->read( &savedLFOBitResolutionDepth, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedDecimator = 0.f;
    if ( state->read( &savedDecimator, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedDecimatorChain = 0.f;
    if ( state->read( &savedDecimatorChain, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedLFODecimator = 1.f;
    if ( state->read( &savedLFODecimator, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFilterChain = 0.f;
    if ( state->read( &savedFilterChain, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFilterCutoff = 1.f;
    if ( state->read( &savedFilterCutoff, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFilterResonance = 1.f;
    if ( state->read( &savedFilterResonance, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedLFOFilter = 1.f;
    if ( state->read( &savedLFOFilter, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedLFOFilterDepth = 1.f;
    if ( state->read( &savedLFOFilterDepth, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFlangerChain = 0.f;
    if ( state->read( &savedFlangerChain, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFlangerRate = 1.f;
    if ( state->read( &savedFlangerRate, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFlangerWidth = 1.f;
    if ( state->read( &savedFlangerWidth, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFlangerFeedback = 1.f;
    if ( state->read( &savedFlangerFeedback, sizeof ( float )) != kResultOk )
        return kResultFalse;

    float savedFlangerDelay = 1.f;
    if ( state->read( &savedFlangerDelay, sizeof ( float )) != kResultOk )
        return kResultFalse;

#if BYTEORDER == kBigEndian
    SWAP_32( savedDelayTime )
    SWAP_32( savedDelayHostSync )
    SWAP_32( savedDelayFeedback )
    SWAP_32( savedDelayMix )
    SWAP_32( savedBitResolution )
    SWAP_32( savedBitResolutionChain )
    SWAP_32( savedLFOBitResolution )
    SWAP_32( savedLFOBitResolutionDepth )
    SWAP_32( savedDecimator )
    SWAP_32( savedDecimatorChain )
    SWAP_32( savedLFODecimator )
    SWAP_32( savedFilterChain )
    SWAP_32( savedFilterCutoff )
    SWAP_32( savedFilterResonance )
    SWAP_32( savedLFOFilter )
    SWAP_32( savedLFOFilterDepth )
    SWAP_32( savedFlangerChain )
    SWAP_32( savedFlangerRate )
    SWAP_32( savedFlangerWidth )
    SWAP_32( savedFlangerFeedback )
    SWAP_32( savedFlangerDelay )
#endif

    fDelayTime             = savedDelayTime;
    fDelayHostSync         = savedDelayHostSync;
    fDelayFeedback         = savedDelayFeedback;
    fDelayMix              = savedDelayMix;
    fBitResolution         = savedBitResolution;
    fBitResolutionChain    = savedBitResolutionChain;
    fLFOBitResolution      = savedLFOBitResolution;
    fLFOBitResolutionDepth = savedLFOBitResolutionDepth;
    fDecimator             = savedDecimator;
    fDecimatorChain        = savedDecimatorChain;
    fLFODecimator          = savedLFODecimator;
    fFilterChain           = savedFilterChain;
    fFilterCutoff          = savedFilterCutoff;
    fFilterResonance       = savedFilterResonance;
    fLFOFilter             = savedLFOFilter;
    fLFOFilterDepth        = savedLFOFilterDepth;
    fFlangerChain          = savedFlangerChain;
    fFlangerRate           = savedFlangerRate;
    fFlangerWidth          = savedFlangerWidth;
    fFlangerFeedback       = savedFlangerFeedback;
    fFlangerDelay          = savedFlangerDelay;

    syncModel();

    // Example of using the IStreamAttributes interface
    FUnknownPtr<IStreamAttributes> stream (state);
    if ( stream )
    {
        IAttributeList* list = stream->getAttributes ();
        if ( list )
        {
            // get the current type (project/Default..) of this state
            String128 string = {0};
            if ( list->getString( PresetAttributes::kStateType, string, 128 * sizeof( TChar )) == kResultTrue )
            {
                UString128 tmp( string );
                char ascii[128];
                tmp.toAscii( ascii, 128 );
                if ( !strncmp( ascii, StateType::kProject, strlen( StateType::kProject )))
                {
                    // we are in project loading context...
                }
            }

            // get the full file path of this state
            TChar fullPath[1024];
            memset( fullPath, 0, 1024 * sizeof( TChar ));
            if ( list->getString( PresetAttributes::kFilePathStringType,
                 fullPath, 1024 * sizeof( TChar )) == kResultTrue )
            {
                // here we have the full path ...
            }
        }
    }
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::getState( IBStream* state )
{
    // here we need to save the model

    float toSaveDelayTime             = fDelayTime;
    float toSavedDelayHostSync        = fDelayHostSync;
    float toSaveDelayFeedback         = fDelayFeedback;
    float toSaveDelayMix              = fDelayMix;
    float toSaveBitResolution         = fBitResolution;
    float toSaveBitResolutionChain    = fBitResolutionChain;
    float toSaveLFOBitResolution      = fLFOBitResolution;
    float toSaveLFOBitResolutionDepth = fLFOBitResolutionDepth;
    float toSaveDecimator             = fDecimator;
    float toSaveDecimatorChain        = fDecimatorChain;
    float toSaveLFODecimator          = fLFODecimator;
    float toSaveFilterChain           = fFilterChain;
    float toSaveFilterCutoff          = fFilterCutoff;
    float toSaveFilterResonance       = fFilterResonance;
    float toSaveLFOFilter             = fLFOFilter;
    float toSaveLFOFilterDepth        = fLFOFilterDepth;
    float toSaveFlangerChain          = fFlangerChain;
    float toSaveFlangerRate           = fFlangerRate;
    float toSaveFlangerWidth          = fFlangerWidth;
    float toSaveFlangerFeedback       = fFlangerFeedback;
    float toSaveFlangerDelay          = fFlangerDelay;

#if BYTEORDER == kBigEndian
    SWAP_32( toSaveDelayTime )
    SWAP_32( toSavedDelayHostSync )
    SWAP_32( toSaveDelayFeedback )
    SWAP_32( toSaveDelayMix )
    SWAP_32( toSaveBitResolution )
    SWAP_32( toSaveBitResolutionChain )
    SWAP_32( toSaveLFOBitResolution )
    SWAP_32( toSaveLFOBitResolutionDepth )
    SWAP_32( toSaveDecimator )
    SWAP_32( toSaveDecimatorChain )
    SWAP_32( toSaveLFODecimator )
    SWAP_32( toSaveFilterChain )
    SWAP_32( toSaveFilterCutoff )
    SWAP_32( toSaveFilterResonance )
    SWAP_32( toSaveLFOFilter )
    SWAP_32( toSaveLFOFilterDepth )
    SWAP_32( toSaveFlangerChain )
    SWAP_32( toSaveFlangerRate )
    SWAP_32( toSaveFlangerWidth )
    SWAP_32( toSaveFlangerFeedback )
    SWAP_32( toSaveFlangerDelay )
#endif

    state->write( &toSaveDelayTime,             sizeof( float ));
    state->write( &toSavedDelayHostSync,        sizeof( float ));
    state->write( &toSaveDelayFeedback,         sizeof( float ));
    state->write( &toSaveDelayMix,              sizeof( float ));
    state->write( &toSaveBitResolution,         sizeof( float ));
    state->write( &toSaveBitResolutionChain,    sizeof( float ));
    state->write( &toSaveLFOBitResolution,      sizeof( float ));
    state->write( &toSaveLFOBitResolutionDepth, sizeof( float ));
    state->write( &toSaveDecimator,             sizeof( float ));
    state->write( &toSaveDecimatorChain,        sizeof( float ));
    state->write( &toSaveLFODecimator,          sizeof( float ));
    state->write( &toSaveFilterChain,           sizeof( float ));
    state->write( &toSaveFilterCutoff,          sizeof( float ));
    state->write( &toSaveFilterResonance,       sizeof( float ));
    state->write( &toSaveLFOFilter,             sizeof( float ));
    state->write( &toSaveLFOFilterDepth,        sizeof( float ));
    state->write( &toSaveFlangerChain,          sizeof( float ));
    state->write( &toSaveFlangerRate,           sizeof( float ));
    state->write( &toSaveFlangerWidth,          sizeof( float ));
    state->write( &toSaveFlangerFeedback,       sizeof( float ));
    state->write( &toSaveFlangerDelay,          sizeof( float ));

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::setupProcessing( ProcessSetup& newSetup )
{
    // called before the process call, always in a disabled state (not active)

    // here we keep a trace of the processing mode (offline,...) for example.
    currentProcessMode = newSetup.processMode;

    VST::SAMPLE_RATE = newSetup.sampleRate;

    // spotted to fire multiple times...

    if ( regraderProcess != 0 )
        delete regraderProcess;

    // TODO: creating a bunch of extra channels for no apparent reason?
    regraderProcess = new RegraderProcess( 6 );
    syncModel();

    return AudioEffect::setupProcessing( newSetup );
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::setBusArrangements( SpeakerArrangement* inputs,  int32 numIns,
                                                 SpeakerArrangement* outputs, int32 numOuts )
{
    if ( numIns == 1 && numOuts == 1 )
    {
        // the host wants Mono => Mono (or 1 channel -> 1 channel)
        if ( SpeakerArr::getChannelCount( inputs[0])  == 1 &&
             SpeakerArr::getChannelCount( outputs[0]) == 1 )
        {
            AudioBus* bus = FCast<AudioBus>( audioInputs.at( 0 ));
            if ( bus )
            {
                // check if we are Mono => Mono, if not we need to recreate the buses
                if ( bus->getArrangement() != inputs[0])
                {
                    removeAudioBusses();
                    addAudioInput ( STR16( "Mono In" ),  inputs[0] );
                    addAudioOutput( STR16( "Mono Out" ), inputs[0] );
                }
                return kResultOk;
            }
        }
        // the host wants something else than Mono => Mono, in this case we are always Stereo => Stereo
        else
        {
            AudioBus* bus = FCast<AudioBus>( audioInputs.at(0));
            if ( bus )
            {
                tresult result = kResultFalse;

                // the host wants 2->2 (could be LsRs -> LsRs)
                if ( SpeakerArr::getChannelCount(inputs[0]) == 2 && SpeakerArr::getChannelCount( outputs[0]) == 2 )
                {
                    removeAudioBusses();
                    addAudioInput  ( STR16( "Stereo In"),  inputs[0] );
                    addAudioOutput ( STR16( "Stereo Out"), outputs[0]);
                    result = kResultTrue;
                }
                // the host want something different than 1->1 or 2->2 : in this case we want stereo
                else if ( bus->getArrangement() != SpeakerArr::kStereo )
                {
                    removeAudioBusses();
                    addAudioInput ( STR16( "Stereo In"),  SpeakerArr::kStereo );
                    addAudioOutput( STR16( "Stereo Out"), SpeakerArr::kStereo );
                    result = kResultFalse;
                }
                return result;
            }
        }
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::canProcessSampleSize( int32 symbolicSampleSize )
{
    if ( symbolicSampleSize == kSample32 )
        return kResultTrue;

    // we support double processing
    if ( symbolicSampleSize == kSample64 )
        return kResultTrue;

    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Regrader::notify( IMessage* message )
{
    if ( !message )
        return kInvalidArgument;

    if ( !strcmp( message->getMessageID(), "BinaryMessage" ))
    {
        const void* data;
        uint32 size;
        if ( message->getAttributes ()->getBinary( "MyData", data, size ) == kResultOk )
        {
            // we are in UI thread
            // size should be 100
            if ( size == 100 && ((char*)data)[1] == 1 ) // yeah...
            {
                fprintf( stderr, "[Regrader] received the binary message!\n" );
            }
            return kResultOk;
        }
    }

    return AudioEffect::notify( message );
}

void Regrader::syncModel()
{
    regraderProcess->syncDelayToHost = Calc::toBool( fDelayHostSync );
    regraderProcess->setDelayTime( fDelayTime );
    regraderProcess->setDelayFeedback( fDelayFeedback );
    regraderProcess->setDelayMix( fDelayMix );

    regraderProcess->bitCrusherPostMix = Calc::toBool( fBitResolutionChain );
    regraderProcess->decimatorPostMix  = Calc::toBool( fDecimatorChain );
    regraderProcess->filterPostMix     = Calc::toBool( fFilterChain );
    regraderProcess->flangerPostMix    = Calc::toBool( fFlangerChain );

    regraderProcess->bitCrusher->setAmount( fBitResolution );
    regraderProcess->bitCrusher->setLFO( fLFOBitResolution, fLFOBitResolutionDepth );
    regraderProcess->decimator->setBits( ( int )( fDecimator * 32.f ));
    regraderProcess->decimator->setRate( fLFODecimator );
    regraderProcess->filter->updateProperties( fFilterCutoff, fFilterResonance, fLFOFilter, fLFOFilterDepth );

    regraderProcess->flanger->setRate( fFlangerRate );
    regraderProcess->flanger->setWidth( fFlangerWidth );
    regraderProcess->flanger->setFeedback( fFlangerFeedback );
    regraderProcess->flanger->setDelay( fFlangerDelay );
}

}
