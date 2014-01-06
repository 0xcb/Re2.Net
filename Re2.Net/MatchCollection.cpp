/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Regex.h"
#include "Match.h"
#include "MatchCollection.h"
#include "MatchEnumerator.h"


namespace Re2
{
namespace Net
{
    MatchCollection::MatchCollection(Match^ match)
    {
        _match   = match;
        _matches = gcnew ArrayList();
        _done    = !match->Success;
        if(!_done)
            _matches->Add(_match);
    }

    Match^ MatchCollection::GetMatch(int i)
    {
        if(i < 0)
            return nullptr;

        if(_matches->Count > i)
            return static_cast<Match^>(_matches[i]);

        if(_done)
            return nullptr;

        do
        {
            _match = _match->NextMatch();
            if(!_match->Success)
            {
                _done = true;
                return nullptr;
            }
            _matches->Add(_match);
        }
        while(_matches->Count <= i);

        return _match;
    }

    void MatchCollection::CopyTo(Array^ array, int arrayIndex)
    {
        _matches->CopyTo(array, arrayIndex);
    }

    IEnumerator^ MatchCollection::GetEnumerator()
    {
        return gcnew MatchEnumerator(this);
    }

    int MatchCollection::Count::get()
    {
        if(!_done)
            this->GetMatch(MatchCollection::_infinite);
        return _matches->Count;
    }

    bool MatchCollection::IsReadOnly::get()
    {
        return true;
    }

    bool MatchCollection::IsSynchronized::get()
    {
        return false;
    }

    Match^ MatchCollection::default::get(int i)
    {
        Match^ match = this->GetMatch(i);
        if(!match)
            throw gcnew ArgumentOutOfRangeException("i");
        return match;
    }

    Object^ MatchCollection::SyncRoot::get()
    {
        return this;
    }
}
}