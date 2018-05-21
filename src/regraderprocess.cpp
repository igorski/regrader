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
#include <algorithm>
#include <math.h>
#include "util.h" // QQQ: debug only

namespace Igorski {

RegraderProcess::RegraderProcess( int amountOfChannels ) {
    _maxTime       = ( int ) round(( Igorski::VST::SAMPLE_RATE / 1000 ) * MAX_DELAY_TIME );
    _delayTime     = 0;
    _delayMix      = .5f;
    _delayFeedback = .1f;

    _delayBuffer  = new AudioBuffer( amountOfChannels, _maxTime );
    _delayIndices = new int[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i ) {
        _delayIndices[ i ] = 0;
    }
    _amountOfChannels = amountOfChannels;
}

RegraderProcess::~RegraderProcess() {
    delete   _delayBuffer;
    delete[] _delayIndices;
}

/* public methods */

void RegraderProcess::process( float** inBuffer, float** outBuffer, int numInChannels, int numOutChannels,
                               int bufferSize, uint32 sampleFramesSize ) {
    float delaySample;
    int readIndex, delayIndex, channelDelayBufferChannel;

    // clear existing output buffer contents
    for ( int32 i = 0; i < numOutChannels; i++ )
        memset( outBuffer[ i ], 0, sampleFramesSize );

    float dryMix = 1.0f - _delayMix;

    for ( int32 c = 0; c < numInChannels; ++c )
    {
        float* channelInBuffer    = inBuffer[ c ];
        float* channelOutBuffer   = outBuffer[ c ];
        float* channelDelayBuffer = _delayBuffer->getBufferForChannel( c );
        delayIndex = _delayIndices[ c ];

        for ( int32 i = 0; i < bufferSize; ++i )
        {
            readIndex = delayIndex - _delayTime + 1;

            if ( readIndex < 0 ) {
                readIndex += _delayTime;
            }

            // read the previously delayed samples from the buffer
            // ( for feedback purposes ) and append the current sample to it

            delaySample = channelDelayBuffer[ readIndex ];
            channelDelayBuffer[ delayIndex ] = channelInBuffer[ i ] + delaySample * _delayFeedback;

            if ( ++delayIndex == _delayTime ) {
                delayIndex = 0;
            }

            // write the wet mix of the effect into the output buffer
            channelOutBuffer[ i ] = delaySample * _delayMix;
        }
        // update last index for this channel
        _delayIndices[ c ] = delayIndex;

        // apply the processing on the delay buffer
        // TODO...

        // mix the input buffer into the output (dry mix)

        for ( int i = 0; i < bufferSize; ++i )
            channelOutBuffer[ i ] += ( channelInBuffer[ i ] * dryMix );
    }
}

/* setters */

void RegraderProcess::setDelayTime( float value ) {
    _delayTime = ( int ) round( std::min( 1.f, std::max( 0.f, value )) * _maxTime );

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
