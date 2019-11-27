#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__

#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/funknown.h"

using namespace Steinberg;

namespace Igorski {
namespace VST {

    static const int   ID       = 97151818;
    static const char* NAME     = "Regrader";
    static const char* VENDOR   = "igorski.nl";

    static const FUID RegraderProcessorUID( 0x9A615AD3, 0xFFF74B54, 0xA6AFDDE5, 0xD9995465 );
    static const FUID RegraderWithSideChainProcessorUID( 0x31C358F3, 0x528F457D, 0xBA31BDFB, 0x70A7A2DA );
    static const FUID RegraderControllerUID( 0xF99D622B, 0xCF48474A, 0xB7E202CD, 0x66D160D9 );

    // set upon initialization, see vst.cpp
    static float SAMPLE_RATE = 44100.f;
    static int BUFFER_SIZE   = 8192;

    static const float PI     = 3.141592653589793f;
    static const float TWO_PI = PI * 2.f;

    // maximum and minimum filter frequency ranges
    // also see regrader.uidesc to update the controls to match

    static const float FILTER_MIN_FREQ      = 30.f;
    static const float FILTER_MAX_FREQ      = 22050.f;
    static const float FILTER_MIN_RESONANCE = 0.1f;
    static const float FILTER_MAX_RESONANCE = 0.7071067811865476f; //sqrt( 2.f ) / 2.f;

    // maximum and minimum rate of oscillation in Hz
    // also see regrader.uidesc to update the controls to match

    static const float MAX_LFO_RATE() { return 10.f; }
    static const float MIN_LFO_RATE() { return .1f; }

    // sine waveform used for the oscillator
    static const float TABLE[ 128 ] = { 0, 0.0490677, 0.0980171, 0.14673, 0.19509, 0.24298, 0.290285, 0.33689, 0.382683, 0.427555, 0.471397, 0.514103, 0.55557, 0.595699, 0.634393, 0.671559, 0.707107, 0.740951, 0.77301, 0.803208, 0.83147, 0.857729, 0.881921, 0.903989, 0.92388, 0.941544, 0.95694, 0.970031, 0.980785, 0.989177, 0.995185, 0.998795, 1, 0.998795, 0.995185, 0.989177, 0.980785, 0.970031, 0.95694, 0.941544, 0.92388, 0.903989, 0.881921, 0.857729, 0.83147, 0.803208, 0.77301, 0.740951, 0.707107, 0.671559, 0.634393, 0.595699, 0.55557, 0.514103, 0.471397, 0.427555, 0.382683, 0.33689, 0.290285, 0.24298, 0.19509, 0.14673, 0.0980171, 0.0490677, 1.22465e-16, -0.0490677, -0.0980171, -0.14673, -0.19509, -0.24298, -0.290285, -0.33689, -0.382683, -0.427555, -0.471397, -0.514103, -0.55557, -0.595699, -0.634393, -0.671559, -0.707107, -0.740951, -0.77301, -0.803208, -0.83147, -0.857729, -0.881921, -0.903989, -0.92388, -0.941544, -0.95694, -0.970031, -0.980785, -0.989177, -0.995185, -0.998795, -1, -0.998795, -0.995185, -0.989177, -0.980785, -0.970031, -0.95694, -0.941544, -0.92388, -0.903989, -0.881921, -0.857729, -0.83147, -0.803208, -0.77301, -0.740951, -0.707107, -0.671559, -0.634393, -0.595699, -0.55557, -0.514103, -0.471397, -0.427555, -0.382683, -0.33689, -0.290285, -0.24298, -0.19509, -0.14673, -0.0980171, -0.0490677 };
}
}

#endif
