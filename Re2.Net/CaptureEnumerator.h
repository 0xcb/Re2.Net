/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "CaptureCollection.h"


namespace Re2
{
namespace Net
{
    using namespace System;

    ref class Capture;
    ref class CaptureCollection;


    /*[Serializable]*/
    private ref class CaptureEnumerator : public IEnumerator
    {
        typedef Capture _Capture;


        internal:
        
            int                _curindex;
            CaptureCollection^ _rcc;

            CaptureEnumerator(CaptureCollection^ rcc);


        public:

            property _Capture^ Capture { _Capture^ get(); }

            virtual property Object^ Current { Object^ get(); }

            virtual bool MoveNext();

            virtual void Reset();
    };
}
}