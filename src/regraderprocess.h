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
#ifndef __REGRADERPROCESS__H_INCLUDED__
#define __REGRADERPROCESS__H_INCLUDED__

#include "global.h"
#include "audiobuffer.h"
#include "bitcrusher.h"
#include "formantfilter.h"
#include "limiter.h"

using namespace Steinberg;

namespace Igorski {
class RegraderProcess {

    // max delay time (in milliseconds)
    const int MAX_DELAY_TIME = 10000;

    public:
        RegraderProcess( int amountOfChannels );
        ~RegraderProcess();

        // apply effect to incoming sampleBuffer contents
        void process( float** inBuffer, float** outBuffer, int numInChannels, int numOutChannels,
            int bufferSize, uint32 sampleFramesSize
        );

        // set delay time (in milliseconds)
        void setDelayTime( float value );
        void setDelayFeedback( float value );
        void setDelayMix( float value );

        BitCrusher* bitCrusher;
        FormantFilter* formantFilter;
        Limiter* limiter;

    private:
        AudioBuffer* _delayBuffer; // holds delay memory
        AudioBuffer* _tempBuffer;  // used during process cycle for applying effects onto delay mix

        int* _delayIndices;
        int _delayTime;
        int _maxTime;
        float _delayMix;
        float _delayFeedback;
        int _amountOfChannels;

};
}

#endif
