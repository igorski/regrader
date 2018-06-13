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
namespace Igorski
{
template <typename SampleType>
void RegraderProcess::process( SampleType** inBuffer, SampleType** outBuffer, int numInChannels, int numOutChannels,
                               int bufferSize, uint32 sampleFramesSize ) {

    SampleType inSample;
    float delaySample;
    int i, readIndex, delayIndex, channelDelayBufferChannel;

    SampleType dryMix = 1.f - _delayMix;
    int maxReadIndex  = std::min( _delayTime, _delayBuffer->bufferSize );

    // prepare the mix buffers and clone the incoming buffer contents into the pre-mix buffer

    prepareMixBuffers( inBuffer, numInChannels, bufferSize );

    // only apply flanger if it has a positive rate or width

    bool hasFlanger = ( flanger->getRate() > 0.f || flanger->getWidth() > 0.f );

    for ( int32 c = 0; c < numInChannels; ++c )
    {
        SampleType* channelInBuffer  = inBuffer[ c ];
        SampleType* channelOutBuffer = outBuffer[ c ];
        float* channelPreMixBuffer   = _preMixBuffer->getBufferForChannel( c );
        float* channelDelayBuffer    = _delayBuffer->getBufferForChannel( c );
        float* channelPostMixBuffer  = _postMixBuffer->getBufferForChannel( c );

        delayIndex = _delayIndices[ c ];

        // when processing the first channel, store the current effects properties
        // so each subsequent channel is processed using the same processor variables

        if ( c == 0 ) {
            decimator->store();
            filter->store();
            flanger->store();
        }

        // PRE MIX processing

        if ( !bitCrusherPostMix )
            bitCrusher->process( channelPreMixBuffer, bufferSize );

        if ( !decimatorPostMix )
            decimator->process( channelPreMixBuffer, bufferSize );

        if ( !filterPostMix )
            filter->process( channelPreMixBuffer, bufferSize, c );

        if ( hasFlanger && !flangerPostMix )
            flanger->process( channelPreMixBuffer, bufferSize, c );

        // DELAY processing applied onto the temp buffer

        for ( i = 0; i < bufferSize; ++i )
        {
            readIndex = delayIndex - _delayTime + 1;

            if ( readIndex < 0 ) {
                readIndex += _delayTime;
            }

            // read the previously delayed samples from the buffer
            // ( for feedback purposes ) and append the processed pre mix buffer sample to it

            delaySample = channelDelayBuffer[ readIndex ];
            channelDelayBuffer[ delayIndex ] = channelPreMixBuffer[ i ] + delaySample * _delayFeedback;

            if ( ++delayIndex >= maxReadIndex ) {
                delayIndex = 0;
            }

            // write the delay sample into the post mix buffer
            channelPostMixBuffer[ i ] = delaySample;
        }

        // update last delay index for this channel

        _delayIndices[ c ] = delayIndex;

        // POST MIX processing
        // apply the post mix effect processing

        if ( decimatorPostMix )
            decimator->process( channelPostMixBuffer, bufferSize );

        if ( bitCrusherPostMix )
            bitCrusher->process( channelPostMixBuffer, bufferSize );

        if ( filterPostMix )
            filter->process( channelPostMixBuffer, bufferSize, c );

        if ( hasFlanger && flangerPostMix )
            flanger->process( channelPostMixBuffer, bufferSize, c );

        // mix the input and processed post mix buffers into the output buffer

        for ( i = 0; i < bufferSize; ++i ) {

            // before writing to the out buffer we take a snapshot of the current in sample
            // value as VST2 in Ableton Live supplies the same buffer for in and out!
            inSample = channelInBuffer[ i ];

            // wet mix (e.g. the effected delay signal)
            channelOutBuffer[ i ] = ( SampleType ) channelPostMixBuffer[ i ] * _delayMix;

            // dry mix (e.g. mix in the input signal)
            channelOutBuffer[ i ] += ( inSample * dryMix );
        }

        // prepare effects for the next channel

        if ( c < ( numInChannels - 1 )) {
            decimator->restore();
            filter->restore();
            flanger->restore();
        }
    }

    // limit the signal as it can get quite hot
    limiter->process<SampleType>( outBuffer, bufferSize, numOutChannels );
}

template <typename SampleType>
void RegraderProcess::prepareMixBuffers( SampleType** inBuffer, int numInChannels, int bufferSize )
{
    // if the pre mix buffer wasn't created yet or the buffer size has changed
    // delete existing buffer and create new one to match properties

    if ( _preMixBuffer == 0 || _preMixBuffer->bufferSize != bufferSize ) {
        delete _preMixBuffer;
        _preMixBuffer = new AudioBuffer( numInChannels, bufferSize );
    }

    // clone the in buffer contents

    for ( int c = 0; c < numInChannels; ++c ) {
        SampleType* inChannelBuffer = ( SampleType* ) inBuffer[ c ];
        float* outChannelBuffer     = ( float* ) _preMixBuffer->getBufferForChannel( c );

        for ( int i = 0; i < bufferSize; ++i ) {
            outChannelBuffer[ i ] = ( float ) inChannelBuffer[ i ];
        }
    }

    // if the post mix buffer wasn't created yet or the buffer size has changed
    // delete existing buffer and create new one to match properties

    if ( _postMixBuffer == 0 || _postMixBuffer->bufferSize != bufferSize ) {
        delete _postMixBuffer;
        _postMixBuffer = new AudioBuffer( numInChannels, bufferSize );
    }
}

}
