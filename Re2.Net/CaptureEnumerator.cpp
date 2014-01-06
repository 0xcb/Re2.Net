/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "CaptureCollection.h"
#include "CaptureEnumerator.h"


namespace Re2
{
namespace Net
{
    typedef Capture _Capture;


    CaptureEnumerator::CaptureEnumerator(CaptureCollection^ rcc)
    {
        _rcc      = rcc;
        _curindex = -1;
    }


    _Capture^ CaptureEnumerator::Capture::get()
    {
        if(_curindex < 0 || _curindex >= _rcc->_capcount)
            throw gcnew InvalidOperationException("EnumNotStarted");

        return _rcc[_curindex];
    }


    Object^ CaptureEnumerator::Current::get()
    {
        return this->Capture;
    }


    bool CaptureEnumerator::MoveNext()
    {
        if(_curindex >= _rcc->_capcount)
            return false;

        return ++_curindex < _rcc->_capcount;
    }


    void CaptureEnumerator::Reset()
    {
        _curindex = -1;
    }
}
}