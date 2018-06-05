/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - http://www.igorski.nl
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
#include "regraderprocess.h"
#include <math.h>

namespace Igorski {

RegraderProcess::RegraderProcess( int amountOfChannels ) {
    _maxTime       = ( int ) round(( Igorski::VST::SAMPLE_RATE / 1000 ) * MAX_DELAY_TIME );
    _delayTime     = 0;
    _delayMix      = .5f;
    _delayFeedback = .1f;

    _delayBuffer  = new AudioBuffer( amountOfChannels, _maxTime );
    _tempBuffer   = new AudioBuffer( amountOfChannels, _maxTime / 10 );
    _delayIndices = new int[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i ) {
        _delayIndices[ i ] = 0;
    }
    _amountOfChannels = amountOfChannels;

    bitCrusher = new BitCrusher( 8, .5f, .5f );
    decimator  = new Decimator( 32, 0.f );
    filter     = new Filter();
    flanger    = new Flanger( 0.1f, 0.5f, .75f, .1f, 1.f );
    limiter    = new Limiter( 10.f, 500.f, .6f );

    bitCrusherPostMix = false;
    decimatorPostMix  = false;
    filterPostMix     = true;
    flangerPostMix    = true;

    // these will be synced to host, see vst.cpp. here we default to 120 BPM in 4/4 time
    _tempo              = 120.0;
    _timeSigNumerator   = 4;
    _timeSigDenominator = 4;

    syncDelayToHost     = true;

    // will be lazily created in the process function
    _cloneInBuffer = 0;
}

RegraderProcess::~RegraderProcess() {
    delete[] _delayIndices;
    delete _delayBuffer;
    delete _tempBuffer;
    delete _cloneInBuffer;
    delete bitCrusher;
    delete decimator;
    delete filter;
    delete flanger;
    delete limiter;
}

/* public methods */

void RegraderProcess::process( float** inBuffer, float** outBuffer, int numInChannels, int numOutChannels,
                               int bufferSize, uint32 sampleFramesSize ) {
    float delaySample;
    int i, readIndex, delayIndex, channelDelayBufferChannel;

    // clear existing output buffer contents
    for ( i = 0; i < numOutChannels; i++ )
        memset( outBuffer[ i ], 0, sampleFramesSize );

    float dryMix = 1.0f - _delayMix;

    // effect LFO is controlled from the outside, cache properties here
    bool hasDecimatorLFO = ( decimator->getRate() > 0.f );
    bool hasFilterLFO    = ( filter->lfo->getRate() > 0.f );
    bool hasFlanger      = ( flanger->getRate() > 0.f || flanger->getWidth() > 0.f );
    float initialDecimatorLFOOffset = decimator->getAccumulator();
    float initialFilterLFOOffset    = filter->lfo->getAccumulator();
    float initialFilterCutoff       = filter->getCurrentCutoff();
    float initialFlangerSweep       = flanger->getSweep();

    // clone in buffer for pre-mix processing
    cloneInBuffer( inBuffer, numInChannels, bufferSize );

    for ( int32 c = 0; c < numInChannels; ++c )
    {
        float* channelInBuffer    = inBuffer[ c ];
        float* channelInCloneBuffer = _cloneInBuffer->getBufferForChannel( c );
        float* channelOutBuffer   = outBuffer[ c ];
        float* channelTempBuffer  = _tempBuffer->getBufferForChannel( c );
        float* channelDelayBuffer = _delayBuffer->getBufferForChannel( c );
        delayIndex = _delayIndices[ c ];

        // when processing each new channel restore to the same LFO offset to get the same movement ;)

        if ( hasDecimatorLFO && c > 0 )
            decimator->setAccumulator( initialDecimatorLFOOffset );

        if ( hasFilterLFO && c > 0 )
            filter->resetFilter( initialFilterLFOOffset, initialFilterCutoff );

        if ( hasFlanger && c > 0 )
            flanger->setSweep( initialFlangerSweep );

        // PRE MIX processing

        if ( !bitCrusherPostMix )
            bitCrusher->process( channelInCloneBuffer, bufferSize );

        if ( !decimatorPostMix )
            decimator->process( channelInCloneBuffer, bufferSize );

        if ( !filterPostMix )
            filter->process( channelInCloneBuffer, bufferSize, c );

        if ( hasFlanger && !flangerPostMix )
            flanger->process( channelInCloneBuffer, bufferSize, c );

        // DELAY processing applied onto the temp buffer

        for ( i = 0; i < bufferSize; ++i )
        {
            readIndex = delayIndex - _delayTime + 1;

            if ( readIndex < 0 ) {
                readIndex += _delayTime;
            }

            // read the previously delayed samples from the buffer
            // ( for feedback purposes ) and append the current incoming sample to it

            delaySample = channelDelayBuffer[ readIndex ];
            channelDelayBuffer[ delayIndex ] = channelInCloneBuffer[ i ] + delaySample * _delayFeedback;

            if ( ++delayIndex == _delayTime ) {
                delayIndex = 0;
            }

            // write the delay sample into the temp buffer
            channelTempBuffer[ i ] = delaySample;
        }
        // update last index for this channel
        _delayIndices[ c ] = delayIndex;

        // POST MIX processing
        // apply the post mix effect processing on the temp buffer

        if ( decimatorPostMix )
            decimator->process( channelTempBuffer, bufferSize );

        if ( filterPostMix )
            filter->process( channelTempBuffer, bufferSize, c );

        if ( bitCrusherPostMix )
            bitCrusher->process( channelTempBuffer, bufferSize );

        if ( hasFlanger && flangerPostMix )
            flanger->process( channelTempBuffer, bufferSize, c );

        // mix the input buffer into the output (dry mix)

        for ( i = 0; i < bufferSize; ++i ) {

            // wet mix (e.g. the effected delay signal)
            channelOutBuffer[ i ] = channelTempBuffer[ i ] * _delayMix;

            // dry mix (e.g. mix in the input signal)
            channelOutBuffer[ i ] += ( channelInBuffer[ i ] * dryMix );
        }
    }

    // limit the signal as it can get quite hot
    limiter->process( outBuffer, bufferSize, ( numOutChannels > 1 ));
}

/* setters */

void RegraderProcess::setDelayTime( float value )
{
    if ( _delayTime == value )
        return;

    _delayTime = ( int ) round( VST::cap( value ) * _maxTime );

    if ( syncDelayToHost )
        syncDelayTime();

    for ( int i = 0; i < _amountOfChannels; ++i ) {
        if ( _delayIndices[ i ] >= _delayTime )
            _delayIndices[ i ] = 0;
    }
}

void RegraderProcess::setDelayMix( float value )
{
    _delayMix = value;
}

void RegraderProcess::setDelayFeedback( float value )
{
    _delayFeedback = value;
}

void RegraderProcess::setTempo( double tempo, int32 timeSigNumerator, int32 timeSigDenominator )
{
    if ( _tempo == tempo && _timeSigNumerator == timeSigNumerator && _timeSigDenominator == timeSigDenominator )
        return;

    if ( syncDelayToHost ) {

        // if delay is synced to host tempo, keep delay time
        // relative to new tempo

        float currentFullMeasureDuration = ( 60.f / _tempo ) * _timeSigDenominator;
        float currentDelaySubdivision    = currentFullMeasureDuration / _delayTime;

        // calculate new delay time (note we're using passed arguments as values)

        float newFullMeasureDuration = ( 60.f / tempo ) * timeSigDenominator;
        _delayTime = newFullMeasureDuration / currentDelaySubdivision;
    }

    _timeSigNumerator   = timeSigNumerator;
    _timeSigDenominator = timeSigDenominator;
    _tempo              = tempo;
}

/* protected methods */

void RegraderProcess::cloneInBuffer( float** inBuffer, int numInChannels, int bufferSize )
{
    // if clone buffer wasn't created yet or the buffer size has changed
    // delete existing buffer and create new to match properties

    if ( _cloneInBuffer == 0 || _cloneInBuffer->bufferSize != bufferSize ) {
        delete _cloneInBuffer;
        _cloneInBuffer = new AudioBuffer( numInChannels, bufferSize );
    }

    // clone the in buffer contents

    for ( int c = 0; c < numInChannels; ++c ) {
        float* inChannelBuffer  = inBuffer[ c ];
        float* outChannelBuffer = _cloneInBuffer->getBufferForChannel( c );

        for ( int i = 0; i < bufferSize; ++i ) {
            outChannelBuffer[ i ] = inChannelBuffer[ i ];
        }
    }
}

void RegraderProcess::syncDelayTime()
{
    // duration of a full measure in buffer seconds

    float fullMeasure = ( 60.f / _tempo ) * _timeSigDenominator;

    // we allow syncing to up to 32nd note resolution

    int subdivision = 32;

    _delayTime = VST::roundTo( _delayTime, fullMeasure / subdivision );
}

}
