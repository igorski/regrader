/**
 * The MIT License (MIT)
 *
 * based on public source code by Dennis Cronin, adapted
 * by Igor Zinken - http://www.igorski.nl
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
#include "flanger.h"
#include "global.h"

namespace Igorski {

/* constructor / destructor */

Flanger::Flanger( float rate, float width, float feedback, float delay, float mix ) {

    FLANGER_BUFFER_SIZE = ( int ) (( float ) VST::SAMPLE_RATE / 5.0f );
    SAMPLE_MULTIPLIER   = ( float ) VST::SAMPLE_RATE * 0.01f;

    _writePointer    = 0;
    _feedbackPhase   =
    _lastSampleLeft  =
    _sweepSamples    =
    _lastSampleRight = 0.0f;
    _mixLeftWet      =
    _mixRightWet     =
    _mixLeftDry      =
    _mixRightDry     = 1.0f;

    _buf1 = new float[ FLANGER_BUFFER_SIZE ];
    _buf2 = new float[ FLANGER_BUFFER_SIZE ];

    // fill buffers with silence
    memset( _buf1, 0, FLANGER_BUFFER_SIZE * sizeof( float ));
    memset( _buf2, 0, FLANGER_BUFFER_SIZE * sizeof( float ));

    _delayFilter = new LowPassFilter( 20.0f );
    _mixFilter   = new LowPassFilter( 20.0f );

    setRate( rate );
    setWidth( width );
    setFeedback( feedback );
    setDelay( delay );
    setMix( mix );
}

Flanger::~Flanger()
{
    delete _buf1;
    delete _buf2;
    delete _delayFilter;
    delete _mixFilter;
}

/* public methods */

float Flanger::getRate()
{
    return _rate;
}

void Flanger::setRate( float value )
{
    _rate = value;

    // map into param onto 0.05Hz - 10hz with log curve
    _sweepRate  = pow( 10.0, ( float ) _rate );
    _sweepRate -= 1.f;
    _sweepRate *= 1.05556f;
    _sweepRate += 0.05f;

    calculateSweep();
}

float Flanger::getWidth()
{
    return _width;
}

void Flanger::setWidth( float value )
{
    _width = value;

    if ( value <= 0.05f )
        _sweepSamples = 0.f;
    else
        _sweepSamples = value * SAMPLE_MULTIPLIER;

    calculateSweep();
}

float Flanger::getDelay()
{
    return _delay;
}

void Flanger::setDelay( float value )
{
    _delay = value;
}

float Flanger::getFeedback()
{
    return _feedback;
}

void Flanger::setFeedback( float value )
{
    _feedback = value;
}

float Flanger::getMix()
{
    return _mix;
}

void Flanger::setMix( float value )
{
    _mix = value;
}

void Flanger::process( float* sampleBuffer, int bufferSize, int c )
{
    // Flanger is currently stereo effect only
    if ( c > 1 )
        return;

    int maxWriteIndex   = FLANGER_BUFFER_SIZE - 1;
    bool isRightChannel = ( c > 0 );

    float delay, mix, delaySamples, left, right, w1, w2, ep;
    int ep1, ep2;

    if ( !isRightChannel ) {
        store();
    }

    for ( int i = 0; i < bufferSize; i++ )
    {
        // filter delay and mix output

        delay = _delayFilter->processSingle( _delay );
        mix   = _mixFilter->processSingle( _mix );

        if ( ++_writePointer > maxWriteIndex )
            _writePointer = 0;

        // delay 0.0-1.0 maps to 0.02ms to 10ms (always have at least 1 sample of delay)
        delaySamples = ( delay * SAMPLE_MULTIPLIER ) + 1.f;
        delaySamples += _sweep;

        // build the two emptying pointers and do linear interpolation
        ep = ( float ) _writePointer - delaySamples;

        if ( ep < 0.0 )
            ep += ( float ) FLANGER_BUFFER_SIZE;

        MODF( ep, ep1, w2 );
        w1 = 1.0 - w2;

        if ( ++ep1 > maxWriteIndex )
            ep1 = 0;

        ep2 = ep1 + 1;

        if ( ep2 > maxWriteIndex )
            ep2 = 0;

        // process input channels and write output back into the buffer

        if ( isRightChannel ) {

            right = sampleBuffer[ i ];
            _buf2[ _writePointer ] = right + _feedback * _feedbackPhase * _lastSampleRight;
            _lastSampleRight = _buf2[ ep1 ] * w1 + _buf2[ ep2 ] * w2;
            sampleBuffer[ i ] = VST::capSample( _mixRightDry * right + _mixRightWet * mix * _lastSampleRight );
        }
        else {
            left = sampleBuffer[ i ];
            _buf1[ _writePointer ] = left + _feedback * _feedbackPhase * _lastSampleLeft;
            _lastSampleLeft = _buf1[ ep1 ] * w1 + _buf1[ ep2 ] * w2;
            sampleBuffer[ i ] = VST::capSample( _mixLeftDry * left + _mixLeftWet * mix * _lastSampleLeft );
        }

        // process sweep

        if ( _step != 0.0 )
        {
            _sweep += _step;

            if ( _sweep <= 0.0 )
            {
                _sweep = 0.0;
                _step = -_step;
            }
            else if ( _sweep >= _maxSweepSamples)
                _step = -_step;
        }
    }

    if ( !isRightChannel ) {
        reset();
    }
}

void Flanger::store()
{
    _writePointerStored = _writePointer;
    _delayFilter->store();
     _mixFilter->store();
}

void Flanger::reset()
{
    _writePointer = _writePointerStored;
    _delayFilter->reset();
    _mixFilter->reset();
}

float Flanger::getSweep()
{
    return _sweep;
}

void Flanger::setSweep( float value )
{
    _sweep = value;
}

/* protected methods */

void Flanger::calculateSweep()
{
    // translate sweep rate to samples per second
    _step = ( float ) ( _sweepSamples * 2.f * _sweepRate ) / ( float ) VST::SAMPLE_RATE;
    _maxSweepSamples = _sweepSamples;
    _sweep = 0.f;
}
}
