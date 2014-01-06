/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "GroupCollection.h"
#include "GroupEnumerator.h"


namespace Re2
{
namespace Net
{
    typedef Capture _Capture;


    GroupEnumerator::GroupEnumerator(GroupCollection^ rgc)
    {
        _rgc      = rgc;
        _curindex = -1;
    }

    _Capture^ GroupEnumerator::Capture::get()
    {
        if(_curindex < 0 || _curindex >= _rgc->Count)
            throw gcnew InvalidOperationException("EnumNotStarted");
        return _rgc[_curindex];
    }

    Object^ GroupEnumerator::Current::get()
    {
        return this->Capture;
    }

    bool GroupEnumerator::MoveNext()
    {
        int count = _rgc->Count;

        if(_curindex >= count)
            return false;

        return ++_curindex < count;
    }

    void GroupEnumerator::Reset()
    {
        _curindex = -1;
    }
}
}