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

    bitCrusher    = new BitCrusher( 8, .5f, .5f );
    formantFilter = new FormantFilter( 0.f );
    limiter       = new Limiter( 10.f, 500.f, .6f );
}

RegraderProcess::~RegraderProcess() {
    delete[] _delayIndices;
    delete _delayBuffer;
    delete _tempBuffer;
    delete bitCrusher;
    delete formantFilter;
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

    // formant filter LFO properties
    float initialFormantLFOOffset = formantFilter->hasLFO ? formantFilter->lfo->getAccumulator() : 0.f;
    float orgFormantVowel         = formantFilter->getVowel();

    for ( int32 c = 0; c < numInChannels; ++c )
    {
        float* channelInBuffer    = inBuffer[ c ];
        float* channelOutBuffer   = outBuffer[ c ];
        float* channelTempBuffer  = _tempBuffer->getBufferForChannel( c );
        float* channelDelayBuffer = _delayBuffer->getBufferForChannel( c );
        delayIndex = _delayIndices[ c ];

        for ( i = 0; i < bufferSize; ++i )
        {
            readIndex = delayIndex - _delayTime + 1;

            if ( readIndex < 0 ) {
                readIndex += _delayTime;
            }

            // read the previously delayed samples from the buffer
            // ( for feedback purposes ) and append the current incoming sample to it

            delaySample = channelDelayBuffer[ readIndex ];
            channelDelayBuffer[ delayIndex ] = channelInBuffer[ i ] + delaySample * _delayFeedback;

            if ( ++delayIndex == _delayTime ) {
                delayIndex = 0;
            }

            // write the delay sample into the temp buffer
            channelTempBuffer[ i ] = delaySample;
        }
        // update last index for this channel
        _delayIndices[ c ] = delayIndex;

        // apply the effect processing on the temp buffer

        // when processing each new channel restore to the same LFO offset to get the same movement ;)

        if ( formantFilter->hasLFO && c > 0 )
        {
            formantFilter->lfo->setAccumulator( initialFormantLFOOffset );
            formantFilter->setVowel( orgFormantVowel );
        }

        bitCrusher->process( channelTempBuffer, bufferSize );
        formantFilter->process( channelTempBuffer, bufferSize );

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

void RegraderProcess::setDelayTime( float value ) {
    _delayTime = ( int ) round( VST::cap( value ) * _maxTime );

    for ( int i = 0; i < _amountOfChannels; ++i ) {
        if ( _delayIndices[ i ] >= _delayTime )
            _delayIndices[ i ] = 0;
    }
}

void RegraderProcess::setDelayMix( float value ) {
    _delayMix = value;
}

void RegraderProcess::setDelayFeedback( float value ) {
    _delayFeedback = value;
}

}
