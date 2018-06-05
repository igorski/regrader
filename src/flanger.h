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
#ifndef __FLANGER_H_INCLUDED__
#define __FLANGER_H_INCLUDED__

#include "lowpassfilter.h"

// Adaptation of modf() by Dennis Cronin
// this macro breaks a double into integer and fractional components i and f respectively.
//
// n - input number, a double
// i - integer portion, an integer (the input number integer portion should fit)
// f - fractional portion, a double

#define MODF(n,i,f) ((i) = (int)(n), (f) = (n) - (double)(i))

/**
 * a mono/stereo Flanger effect (more channels currently not supported)
 */
namespace Igorski {
class Flanger
{
    public:

        // all arguments are in the 0 - 1 range
        Flanger( float rate, float width, float feedback, float delay, float mix );
        ~Flanger();

        float getRate();
        void setRate( float value );
        float getWidth();
        void setWidth( float value );
        float getFeedback();
        void setFeedback( float value );
        float getDelay();
        void setDelay( float value );
        float getMix();
        void setMix( float value );

        void process( float* sampleBuffer, int bufferSize, int c );

        float getSweep();
        void setSweep( float value );

        void store();
        void reset();

    protected:

        float _rate;
        float _width;
        float _feedback;
        float _delay;
        float _mix;
        float _feedbackPhase;
        float _sweepSamples;
        float _maxSweepSamples;
        int _writePointer;
        int _writePointerStored;
        float _step;
        float _sweep;
        float* _buf1;
        float* _buf2;

        LowPassFilter* _delayFilter;
        LowPassFilter* _mixFilter;

        float _lastSampleLeft;
        float _lastSampleRight;
        float _mixLeftWet;
        float _mixLeftDry;
        float _mixRightWet;
        float _mixRightDry;
        float _sweepRate;

        int FLANGER_BUFFER_SIZE;
        float SAMPLE_MULTIPLIER;

        void calculateSweep();
};
}

#endif
