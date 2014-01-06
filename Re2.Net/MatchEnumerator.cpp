/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Match.h"
#include "MatchCollection.h"
#include "MatchEnumerator.h"


namespace Re2
{
namespace Net
{
    MatchEnumerator::MatchEnumerator(MatchCollection^ matchcoll)
    {
        _matchcoll = matchcoll;
    }
    
    bool MatchEnumerator::MoveNext()
    {
        if(_done)
            return false;

        _match = _matchcoll->GetMatch(_curindex);
        _curindex++;
        if(!_match)
        {
            _done = true;
            return false;
        }

        return true;
    }

    void MatchEnumerator::Reset()
    {
        _curindex = 0;
        _done     = false;
        _match    = nullptr;
    }

    Object^ MatchEnumerator::Current::get()
    {
        if(!_match)
            throw gcnew InvalidOperationException("EnumNotStarted");

        return _match;
    }
}
}