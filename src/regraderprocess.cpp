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
#include "calc.h"
#include <math.h>

namespace Igorski {

RegraderProcess::RegraderProcess( int amountOfChannels ) {
    _delayTime     = 0;
    _delayMix      = .5f;
    _delayFeedback = .1f;

    _delayBuffer  = new AudioBuffer( amountOfChannels, Calc::millisecondsToBuffer( MAX_DELAY_TIME_MS ));
    _delayIndices = new int[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i ) {
        _delayIndices[ i ] = 0;
    }
    _amountOfChannels = amountOfChannels;

    bitCrusher = new BitCrusher( 8, .5f, .5f );
    decimator  = new Decimator( 32, 0.f );
    filter     = new Filter();
    flanger    = new Flanger( amountOfChannels );
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
    _preMixBuffer  = 0;
    _postMixBuffer = 0;
}

RegraderProcess::~RegraderProcess() {
    delete[] _delayIndices;
    delete _delayBuffer;
    delete _postMixBuffer;
    delete _preMixBuffer;
    delete bitCrusher;
    delete decimator;
    delete filter;
    delete flanger;
    delete limiter;
}

/* setters */

void RegraderProcess::setDelayTime( float value )
{
    // maximum delay time (in milliseconds) is specified in MAX_DELAY_TIME_MS when using freeform scaling
    // when the delay is synced to the host, the maximum time is a single measure
    // at the current tempo and time signature

    float delayMaxInMs = ( syncDelayToHost ) ? (( 60.f / _tempo ) * _timeSigDenominator ) * 1000.f
        : MAX_DELAY_TIME_MS;

    _delayTime = Calc::millisecondsToBuffer( Calc::cap( value ) * delayMaxInMs );

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

void RegraderProcess::syncDelayTime()
{
    // duration of a full measure in samples

    int fullMeasureSamples = Calc::secondsToBuffer(( 60.f / _tempo ) * _timeSigDenominator );

    // we allow syncing to up to 32nd note resolution

    int subdivision = 32;

    _delayTime = Calc::roundTo( _delayTime, fullMeasureSamples / subdivision );
}

}
