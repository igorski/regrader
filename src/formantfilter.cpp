/**
 * The MIT License (MIT)
 *
 * based on public source code by alex@smartelectronix.com, adapted
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
#include "formantfilter.h"
#include <float.h>
#include <cmath>

namespace Igorski {

/* constructor / destructor */

/**
 * @param aVowel {double} interpolated value within
 *                       the range of the amount specified in the coeffs Array
 */
FormantFilter::FormantFilter( double aVowel )
{
    int i = 0;
    for ( ; i < 11; i++ )
        _currentCoeffs[ i ] = 0.0;

    for ( i = 0; i < 10; i++ )
        _memory[ i ] = 0.0;

    cacheCoeffs();
    setVowel( aVowel );

    lfo    = new LFO();
    hasLFO = false;
    cacheLFO();
}

FormantFilter::~FormantFilter()
{
    delete lfo;
}

/* public methods */

double FormantFilter::getVowel()
{
    return _vowel;
}

void FormantFilter::setVowel( double aVowel )
{
    float tempRatio = _tempVowel / std::max(( double ) MIN_VOWEL(), _vowel );

    _vowel = aVowel;

    // in case filter is attached to oscillator, keep relative offset
    // of currently moving vowel in place
    _tempVowel = ( hasLFO ) ? _vowel * tempRatio : _vowel;

    cacheLFO();
    calcCoeffs();
}

void FormantFilter::setLFO( float LFORatePercentage, float LFODepth )
{
    bool wasEnabled = hasLFO;
    bool enabled    = LFORatePercentage > 0.f;

    hasLFO = enabled;

    bool hadChange = ( wasEnabled != enabled ) || _lfoDepth != LFODepth;

    if ( enabled )
        lfo->setRate(
            VST::MIN_LFO_RATE() + (
                LFORatePercentage * ( VST::MAX_LFO_RATE() - VST::MIN_LFO_RATE() )
            )
        );

    // turning LFO off
    if ( !hasLFO && wasEnabled )
        _tempVowel = _vowel;

    if ( hadChange ) {
        _lfoDepth = LFODepth;
        calcCoeffs();
        cacheLFO();
    }
}

void FormantFilter::process( float* inBuffer, int bufferSize )
{
    // if vowel is 0, don't do anything.
    if ( _vowel == 0.f )
        return;

    for ( int i = 0; i < bufferSize; ++i )
    {
        float res = ( _currentCoeffs[ 0 ]  * inBuffer[ i ] +
                      _currentCoeffs[ 1 ]  * _memory[ 0 ] +
                      _currentCoeffs[ 2 ]  * _memory[ 1 ] +
                      _currentCoeffs[ 3 ]  * _memory[ 2 ] +
                      _currentCoeffs[ 4 ]  * _memory[ 3 ] +
                      _currentCoeffs[ 5 ]  * _memory[ 4 ] +
                      _currentCoeffs[ 6 ]  * _memory[ 5 ] +
                      _currentCoeffs[ 7 ]  * _memory[ 6 ] +
                      _currentCoeffs[ 8 ]  * _memory[ 7 ] +
                      _currentCoeffs[ 9 ]  * _memory[ 8 ] +
                      _currentCoeffs[ 10 ] * _memory[ 9 ] );

        _memory[ 9 ] = _memory[ 8 ];
        _memory[ 8 ] = _memory[ 7 ];
        _memory[ 7 ] = _memory[ 6 ];
        _memory[ 6 ] = _memory[ 5 ];
        _memory[ 5 ] = _memory[ 4 ];
        _memory[ 4 ] = _memory[ 3 ];
        _memory[ 3 ] = _memory[ 2 ];
        _memory[ 2 ] = _memory[ 1 ];
        _memory[ 1 ] = _memory[ 0 ];
        _memory[ 0 ] = res;

        inBuffer[ i ] = res;

        // oscillator attached to formant filter ? travel the vowel values
        // between the minimum and maximum frequencies

        if ( hasLFO )
        {
            // multiply by .5 and add .5 to make the LFO's bipolar waveform unipolar
            float lfoValue = lfo->peek() * .5f  + .5f;
            _tempVowel = std::min( _lfoMax, _lfoMin + _lfoRange * lfoValue );
            calcCoeffs();
        }
    }
}

void FormantFilter::cacheLFO()
{
    _lfoRange = ( float ) _vowel * _lfoDepth;
    _lfoMax   = std::min(( float ) VOWEL_U, ( float ) _vowel + _lfoRange / 2.f );
    _lfoMin   = std::max(( float ) MIN_VOWEL(), ( float ) _vowel - _lfoRange / 2.f );
}

/* private methods */

// stores the vowel coefficients

void FormantFilter::cacheCoeffs()
{
    // vowel "A"

    _coeffs[ VOWEL_A ][ 0 ]  = 8.11044e-06;
    _coeffs[ VOWEL_A ][ 1 ]  = 8.943665402;
    _coeffs[ VOWEL_A ][ 2 ]  = -36.83889529;
    _coeffs[ VOWEL_A ][ 3 ]  = 92.01697887;
    _coeffs[ VOWEL_A ][ 4 ]  = -154.337906;
    _coeffs[ VOWEL_A ][ 5 ]  = 181.6233289;
    _coeffs[ VOWEL_A ][ 6 ]  = -151.8651235;
    _coeffs[ VOWEL_A ][ 7 ]  = 89.09614114;
    _coeffs[ VOWEL_A ][ 8 ]  = -35.10298511;
    _coeffs[ VOWEL_A ][ 9 ]  = 8.388101016;
    _coeffs[ VOWEL_A ][ 10 ] = -0.923313471;

    // vowel "E"

    _coeffs[ VOWEL_E ][ 0 ]  = 4.36215e-06;
    _coeffs[ VOWEL_E ][ 1 ]  = 8.90438318;
    _coeffs[ VOWEL_E ][ 2 ]  = -36.55179099;
    _coeffs[ VOWEL_E ][ 3 ]  = 91.05750846;
    _coeffs[ VOWEL_E ][ 4 ]  = -152.422234;
    _coeffs[ VOWEL_E ][ 5 ]  = 179.1170248;
    _coeffs[ VOWEL_E ][ 6 ]  = -149.6496211;
    _coeffs[ VOWEL_E ][ 7 ]  = 87.78352223;
    _coeffs[ VOWEL_E ][ 8 ]  = -34.60687431;
    _coeffs[ VOWEL_E ][ 9 ]  = 8.282228154;
    _coeffs[ VOWEL_E ][ 10 ] = -0.914150747;

    // vowel "I"

    _coeffs[ VOWEL_I ][ 0 ]  = 3.33819e-06;
    _coeffs[ VOWEL_I ][ 1 ]  = 8.893102966;
    _coeffs[ VOWEL_I ][ 2 ]  = -36.49532826;
    _coeffs[ VOWEL_I ][ 3 ]  = 90.96543286;
    _coeffs[ VOWEL_I ][ 4 ]  = -152.4545478;
    _coeffs[ VOWEL_I ][ 5 ]  = 179.4835618;
    _coeffs[ VOWEL_I ][ 6 ]  = -150.315433;
    _coeffs[ VOWEL_I ][ 7 ]  = 88.43409371;
    _coeffs[ VOWEL_I ][ 8 ]  = -34.98612086;
    _coeffs[ VOWEL_I ][ 9 ]  = 8.407803364;
    _coeffs[ VOWEL_I ][ 10 ] = -0.932568035;

    // vowel "O"

    _coeffs[ VOWEL_O ][ 0 ]  = 1.13572e-06;
    _coeffs[ VOWEL_O ][ 1 ]  = 8.994734087;
    _coeffs[ VOWEL_O ][ 2 ]  = -37.2084849;
    _coeffs[ VOWEL_O ][ 3 ]  = 93.22900521;
    _coeffs[ VOWEL_O ][ 4 ]  = -156.6929844;
    _coeffs[ VOWEL_O ][ 5 ]  = 184.596544;
    _coeffs[ VOWEL_O ][ 6 ]  = -154.3755513;
    _coeffs[ VOWEL_O ][ 7 ]  = 90.49663749;
    _coeffs[ VOWEL_O ][ 8 ]  = -35.58964535;
    _coeffs[ VOWEL_O ][ 9 ]  = 8.478996281;
    _coeffs[ VOWEL_O ][ 10 ] = -0.929252233;

    // vowel "U"

    _coeffs[ VOWEL_U ][ 0 ]  = 4.09431e-07;
    _coeffs[ VOWEL_U ][ 1 ]  = 8.997322763;
    _coeffs[ VOWEL_U ][ 2 ]  = -37.20218544;
    _coeffs[ VOWEL_U ][ 3 ]  = 93.11385476;
    _coeffs[ VOWEL_U ][ 4 ]  = -156.2530937;
    _coeffs[ VOWEL_U ][ 5 ]  = 183.7080141;
    _coeffs[ VOWEL_U ][ 6 ]  = -153.2631681;
    _coeffs[ VOWEL_U ][ 7 ]  = 89.59539726;
    _coeffs[ VOWEL_U ][ 8 ]  = -35.12454591;
    _coeffs[ VOWEL_U ][ 9 ]  = 8.338655623;
    _coeffs[ VOWEL_U ][ 10 ] = -0.910251753;
}

void FormantFilter::calcCoeffs()
{
    double interpolationThreshold = 0.f;//.7;

    // Linearly interpolate the values when below threshold
    // TODO : this is weak ;) keep calculated coeffecient values within double range

    if ( _tempVowel < interpolationThreshold )
    {
        int roundVowel  = ( int )( _tempVowel * 4.0 );
        double fracpart = _tempVowel - roundVowel;

        for ( int i = 0; i < 11; i++ )
        {
            _currentCoeffs[ i ] = _coeffs[ roundVowel ][ i ] * ( 1.0 - fracpart ) +
                                ( _coeffs[ roundVowel + ( roundVowel < 4 )][ i ] * fracpart );
        }
    }
    else
    {
        // interpolation beyond this point is quite nasty... keep in exact range

        int min      = ( int ) round( _tempVowel );
        int max      = min < ( 4 /* _coeffs.length - 1 */ ) ? min + 1 : min;
        double delta = std::abs( min - _tempVowel );

        int l = 11;

        for ( int i = 0; i < l; ++i )
        {
            double minCoeff = _coeffs[ min ][ i ];
            double maxCoeff = _coeffs[ max ][ i ];

            _currentCoeffs[ i ] = delta < .5 ? minCoeff : maxCoeff;
        }
    }
}

}
