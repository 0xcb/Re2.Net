/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "GroupCollection.h"


namespace Re2
{
namespace Net
{
    using namespace System;


    private ref class GroupEnumerator : public IEnumerator
    {
        typedef Capture _Capture;


        internal:

            int              _curindex;
            GroupCollection^ _rgc;

            GroupEnumerator(GroupCollection^ rgc);


        public:
            
            property _Capture^ Capture { _Capture^ get(); }

            virtual property Object^ Current { Object^ get(); }

            virtual bool MoveNext();

            virtual void Reset();
    };
}
}