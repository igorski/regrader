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
#include "calc.h"

namespace Igorski {

/* constructor / destructor */

Flanger::Flanger( int amountOfChannels ) {

    FLANGER_BUFFER_SIZE = ( int ) (( float ) VST::SAMPLE_RATE / 5.0f );
    SAMPLE_MULTIPLIER   = ( float ) VST::SAMPLE_RATE * 0.01f;

    _writePointer         =
    _writePointerStored   = 0;
    _feedbackPhase        = 1.f;
    _sweepSamples         = 0.f;
    _mixLeftWet           =
    _mixRightWet          =
    _mixLeftDry           =
    _mixRightDry          = 1.f;

    for ( int i = 0; i < amountOfChannels; ++i ) {

        // create delay buffers to write the flanger delay into

        float* buffer = new float[ FLANGER_BUFFER_SIZE ];
        memset( buffer, 0, FLANGER_BUFFER_SIZE * sizeof( float ));
        _buffers.push_back( buffer );

        _lastChannelSamples.push_back( 0.f );
    }

    _delayFilter = new LowPassFilter( 20.f );
    _mixFilter   = new LowPassFilter( 20.f );

    setRate( 0.1f );
    setWidth( 0.5f );
    setFeedback( 0.75f );
    setDelay( .1f  );
    setMix( 1.f );
}

Flanger::~Flanger()
{
    delete _delayFilter;
    delete _mixFilter;

    while ( _buffers.size() > 0 ) {
        delete _buffers.back();
        _buffers.pop_back();
    }
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
    float* delayBuffer = _buffers.at( c );
    int maxWriteIndex = FLANGER_BUFFER_SIZE - 1;

    float delay, mix, delaySamples, sample, w1, w2, ep;
    int ep1, ep2;

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

        sample = sampleBuffer[ i ];
        delayBuffer[ _writePointer ] = sample + _feedback * _feedbackPhase * _lastChannelSamples.at( c );
        _lastChannelSamples.at( c ) = delayBuffer[ ep1 ] * w1 + delayBuffer[ ep2 ] * w2;
        sampleBuffer[ i ] = Calc::capSample( _mixLeftDry * sample + _mixLeftWet * mix * _lastChannelSamples.at( c ));

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
}

void Flanger::store()
{
    _writePointerStored = _writePointer;
    _sweepStored = _sweep;
    _delayFilter->store();
    _mixFilter->store();
}

void Flanger::restore()
{
    _writePointer = _writePointerStored;
    _sweep = _sweepStored;
    _delayFilter->restore();
    _mixFilter->restore();
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
