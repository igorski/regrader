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
#include "lowpassfilter.h"
#include "global.h"
#include <cmath>

namespace Igorski {

/* constructor / destructor */

LowPassFilter::LowPassFilter( float cutoff )
{
    setCutoff( cutoff );
}

LowPassFilter::~LowPassFilter()
{

}

/* public methods */

float LowPassFilter::getCutoff()
{
    return _cutoff;
}

void LowPassFilter::setCutoff( float value )
{
    _cutoff = value;

    float Q = 1.1f;
    w0 = VST::TWO_PI * _cutoff / ( float ) VST::SAMPLE_RATE;
    alpha = sin(w0) / ( 2.f * Q );
    b0 =  (1.f - cos(w0)) / 2.f;
    b1 =   1.f - cos(w0);
    b2 =  (1.f - cos(w0)) / 2.f;
    a0 =   1.f + alpha;
    a1 =  -2.0 * cos(w0);
    a2 = 1.f - alpha;
    x1 = x2 = y1 = y2 = 0;
}

void LowPassFilter::store()
{
    orgx1 = x1;
    orgx2 = x2;
    orgy1 = y1;
    orgy2 = y2;
}

void LowPassFilter::restore()
{
    x1 = orgx1;
    x2 = orgx2;
    y1 = orgy1;
    y2 = orgy2;
}

float LowPassFilter::processSingle( float sample )
{
    float sampleOut = (b0/a0) * sample + (b1/a0) * x1 + (b2/a0) * x2 - (a1/a0) * y1 - (a2/a0) * y2;

    x2 = x1;
    x1 = sample;
    y2 = y1;
    y1 = sampleOut;

    return sampleOut;
}
}
