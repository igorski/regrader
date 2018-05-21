/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - https://www.igorski.nl
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
#include "../global.h"
#include "controller.h"
#include "uimessagecontroller.h"
#include "../paramids.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"

#include "base/source/fstring.h"

#include "vstgui/uidescription/delegationcontroller.h"

#include <stdio.h>
#include <math.h>

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
// RegraderController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::initialize( FUnknown* context )
{
    tresult result = EditControllerEx1::initialize( context );

    if ( result != kResultOk )
        return result;

    //--- Create Units-------------
    UnitInfo unitInfo;
    Unit* unit;

    // create root only if you want to use the programListId
    /*	unitInfo.id = kRootUnitId;	// always for Root Unit
    unitInfo.parentUnitId = kNoParentUnitId;	// always for Root Unit
    Steinberg::UString (unitInfo.name, USTRINGSIZE (unitInfo.name)).assign (USTRING ("Root"));
    unitInfo.programListId = kNoProgramListId;

    unit = new Unit (unitInfo);
    addUnitInfo (unit);*/

    // create a unit1
    unitInfo.id = 1;
    unitInfo.parentUnitId = kRootUnitId;    // attached to the root unit

    Steinberg::UString( unitInfo.name, USTRINGSIZE( unitInfo.name )).assign( USTRING( "ADSR" ));

    unitInfo.programListId = kNoProgramListId;

    unit = new Unit( unitInfo );
    addUnit( unit );
    int32 unitId = 1;

    // Delay controls

    RangeParameter* delayTimeParam = new RangeParameter(
        USTRING( "Delay time" ), kDelayTimeId, USTRING( "seconds" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( delayTimeParam );

    RangeParameter* delayFeedbackParam = new RangeParameter(
        USTRING( "Delay feedback" ), kDelayFeedbackId, USTRING( "0 - 1" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
     );
    parameters.addParameter( delayFeedbackParam );

    RangeParameter* delayMixParam = new RangeParameter(
        USTRING( "Delay mix" ), kDelayMixId, USTRING( "0 - 1" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( delayMixParam );

    // bit crusher controls

    RangeParameter* bitParam = new RangeParameter(
        USTRING( "Bit resolution" ), kBitResolutionId, USTRING( "0 - 16" ),
        16, 0, 16,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( bitParam );

    RangeParameter* bitLfoRateParam = new RangeParameter(
        USTRING( "Bit LFO rate" ), kLFOBitResolutionId, USTRING( "Hz" ),
        Igorski::VST::MIN_LFO_RATE(), Igorski::VST::MAX_LFO_RATE(), Igorski::VST::MIN_LFO_RATE(),
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( bitLfoRateParam );

    RangeParameter* bitLfoDepthParam = new RangeParameter(
        USTRING( "Bit LFO depth" ), kLFOBitResolutionDepthId, USTRING( "%" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( bitLfoDepthParam );

    // formant filter controls

    RangeParameter* formantParam = new RangeParameter(
        USTRING( "Formant" ), kFormantId, USTRING( "0 - 6" ),
        0, 0, 6,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( formantParam );

    RangeParameter* formantLfoRateParam = new RangeParameter(
        USTRING( "Formant LFO rate" ), kLFOFormantId, USTRING( "Hz" ),
        Igorski::VST::MIN_LFO_RATE(), Igorski::VST::MAX_LFO_RATE(), Igorski::VST::MIN_LFO_RATE(),
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( formantLfoRateParam );

    RangeParameter* formantLfoDepthParam = new RangeParameter(
        USTRING( "Formant LFO depth" ), kLFOFormantDepthId, USTRING( "%" ),
        0.f, 1.f, 0.f,
        0, ParameterInfo::kCanAutomate, unitId
    );
    parameters.addParameter( formantLfoDepthParam );

    // initialization

    String str( "REGRADER" );
    str.copyTo16( defaultMessageText, 0, 127 );

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::terminate()
{
    return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::setComponentState( IBStream* state )
{
    // we receive the current state of the component (processor part)
    if ( state )
    {
        float savedDelayTime = 1.f;
        if ( state->read( &savedDelayTime, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedDelayFeedback = 1.f;
        if ( state->read( &savedDelayFeedback, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedDelayMix = 1.f;
        if ( state->read( &savedDelayMix, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedBitResolution = 1.f;
        if ( state->read( &savedBitResolution, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedLFOBitResolution = Igorski::VST::MIN_LFO_RATE();
        if ( state->read( &savedLFOBitResolution, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedLFOBitResolutionDepth = 1.f;
        if ( state->read( &savedLFOBitResolutionDepth, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedFormant = 0.f;
        if ( state->read( &savedFormant, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedLFOFormant = Igorski::VST::MIN_LFO_RATE();
        if ( state->read( &savedLFOFormant, sizeof( float )) != kResultOk )
            return kResultFalse;

        float savedLFOFormantDepth = 1.f;
        if ( state->read( &savedLFOFormantDepth, sizeof( float )) != kResultOk )
            return kResultFalse;


#if BYTEORDER == kBigEndian
    SWAP_32( savedDelayTime )
    SWAP_32( savedDelayFeedback )
    SWAP_32( savedDelayMix )
    SWAP_32( savedBitResolution )
    SWAP_32( savedLFOBitResolution )
    SWAP_32( savedLFOBitResolutionDepth )
    SWAP_32( savedFormant )
    SWAP_32( savedLFOFormant )
    SWAP_32( savedLFOFormantDepth )
#endif
        setParamNormalized( kDelayTimeId,             savedDelayTime );
        setParamNormalized( kDelayFeedbackId,         savedDelayFeedback );
        setParamNormalized( kDelayMixId,              savedDelayMix );
        setParamNormalized( kBitResolutionId,         savedBitResolution );
        setParamNormalized( kLFOBitResolutionId,      savedLFOBitResolution );
        setParamNormalized( kLFOBitResolutionDepthId, savedLFOBitResolutionDepth );
        setParamNormalized( kFormantId,               savedFormant );
        setParamNormalized( kLFOFormantId,            savedLFOFormant );
        setParamNormalized( kLFOFormantDepthId,       savedLFOFormantDepth );

        state->seek( sizeof ( float ), IBStream::kIBSeekCur );
    }
    return kResultOk;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API RegraderController::createView( const char* name )
{
    // create the visual editor
    if ( name && strcmp( name, "editor" ) == 0 )
    {
        VST3Editor* view = new VST3Editor( this, "view", "regrader.uidesc" );
        return view;
    }
    return 0;
}

//------------------------------------------------------------------------
IController* RegraderController::createSubController( UTF8StringPtr name,
                                                    const IUIDescription* /*description*/,
                                                    VST3Editor* /*editor*/ )
{
    if ( UTF8StringView( name ) == "MessageController" )
    {
        UIMessageController* controller = new UIMessageController( this );
        addUIMessageController( controller );
        return controller;
    }
    return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::setState( IBStream* state )
{
    tresult result = kResultFalse;

    int8 byteOrder;
    if (( result = state->read( &byteOrder, sizeof( int8 ))) != kResultTrue )
        return result;

    if (( result = state->read( defaultMessageText, 128 * sizeof( TChar ))) != kResultTrue )
        return result;

    // if the byteorder doesn't match, byte swap the text array ...
    if ( byteOrder != BYTEORDER )
    {
        for ( int32 i = 0; i < 128; i++ )
            SWAP_16( defaultMessageText[ i ])
    }

    // update our editors
    for ( UIMessageControllerList::iterator it = uiMessageControllers.begin (), end = uiMessageControllers.end (); it != end; ++it )
        ( *it )->setMessageText( defaultMessageText );

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::getState( IBStream* state )
{
    // here we can save UI settings for example

    // as we save a Unicode string, we must know the byteorder when setState is called
    int8 byteOrder = BYTEORDER;
    if ( state->write( &byteOrder, sizeof( int8 )) == kResultTrue )
    {
        return state->write( defaultMessageText, 128 * sizeof( TChar ));
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
tresult RegraderController::receiveText( const char* text )
{
    // received from Component
    if ( text )
    {
        fprintf( stderr, "[RegraderController] received: " );
        fprintf( stderr, "%s", text );
        fprintf( stderr, "\n" );
    }
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::setParamNormalized( ParamID tag, ParamValue value )
{
    // called from host to update our parameters state
    tresult result = EditControllerEx1::setParamNormalized( tag, value );
    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::getParamStringByValue( ParamID tag, ParamValue valueNormalized, String128 string )
{
    switch ( tag )
    {
        // these controls are floating point values in 0 - 1 range, we can
        // simply read the normalized value which is in the same range

        case kDelayTimeId:
        case kDelayFeedbackId:
        case kDelayMixId:
        case kBitResolutionId:
        case kLFOBitResolutionDepthId:
        case kFormantId:
        case kLFOFormantDepthId:
        {
            char text[32];
            sprintf( text, "%.2f", ( float ) valueNormalized );
            Steinberg::UString( string, 128 ).fromAscii( text );

            return kResultTrue;
        }

        // LFO settings are also floating point but in a custom range
        // request the plain value from the normalized value

        case kLFOBitResolutionId:
        case kLFOFormantId:
        {
            char text[32];
            if (( tag == kLFOBitResolutionId || tag == kLFOFormantId ) && valueNormalized == 0 )
                sprintf( text, "%s", "Off" );
            else
                sprintf( text, "%.2f", normalizedParamToPlain( tag, valueNormalized ));
            Steinberg::UString( string, 128 ).fromAscii( text );

            return kResultTrue;
        }

        // everything else
        default:
            return EditControllerEx1::getParamStringByValue( tag, valueNormalized, string );
    }
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::getParamValueByString( ParamID tag, TChar* string, ParamValue& valueNormalized )
{
    /* example, but better to use a custom Parameter as seen in RangeParameter
    switch (tag)
    {
        case kAttackId:
        {
            Steinberg::UString wrapper ((TChar*)string, -1); // don't know buffer size here!
            double tmp = 0.0;
            if (wrapper.scanFloat (tmp))
            {
                valueNormalized = expf (logf (10.f) * (float)tmp / 20.f);
                return kResultTrue;
            }
            return kResultFalse;
        }
    }*/
    return EditControllerEx1::getParamValueByString( tag, string, valueNormalized );
}

//------------------------------------------------------------------------
void RegraderController::addUIMessageController( UIMessageController* controller )
{
    uiMessageControllers.push_back( controller );
}

//------------------------------------------------------------------------
void RegraderController::removeUIMessageController( UIMessageController* controller )
{
    UIMessageControllerList::const_iterator it = std::find(
        uiMessageControllers.begin(), uiMessageControllers.end (), controller
    );
    if ( it != uiMessageControllers.end())
        uiMessageControllers.erase( it );
}

//------------------------------------------------------------------------
void RegraderController::setDefaultMessageText( String128 text )
{
    String tmp( text );
    tmp.copyTo16( defaultMessageText, 0, 127 );
}

//------------------------------------------------------------------------
TChar* RegraderController::getDefaultMessageText()
{
    return defaultMessageText;
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::queryInterface( const char* iid, void** obj )
{
    QUERY_INTERFACE( iid, obj, IMidiMapping::iid, IMidiMapping );
    return EditControllerEx1::queryInterface( iid, obj );
}

//------------------------------------------------------------------------
tresult PLUGIN_API RegraderController::getMidiControllerAssignment( int32 busIndex, int16 /*midiChannel*/,
    CtrlNumber midiControllerNumber, ParamID& tag )
{
    // we support for the Gain parameter all MIDI Channel but only first bus (there is only one!)
/*
    if ( busIndex == 0 && midiControllerNumber == kCtrlVolume )
    {
        tag = kDelayTimeId;
        return kResultTrue;
    }
*/
    return kResultFalse;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
