/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Group.h"
#include "GroupCollection.h"
#include "Match.h"
#include "Regex.h"
#include "RegexInput.h"


namespace Re2
{
namespace Net
{
    Match::Match(Regex^ regex, int groupcount, RegexInput^ input, int begpos, int len, int nextpos)
        : Group(input, begpos, len)
    {
        _regex      = regex;
        _groupcount = groupcount;
        _input      = input;
        _nextpos    = nextpos;
    }

    Match^ Match::Empty::get()
    {
        return Match::_empty;
    }

    GroupCollection^ Match::Groups::get()
    {
        if(!_groupcoll)
            _groupcoll = gcnew GroupCollection(this);

        return _groupcoll;
    }

    Match^ Match::NextMatch()
    {
        if(!_regex)
            return this;

        /* Explicitly advance the input start if the match is an empty string. */
        int start = _length ? _nextpos : _nextpos + 1;
        int end   = this->Input->Length;

        /* 
         *  In .NET's Regex class matches are still attempted (and an empty match
         *  can be successful) immediately after the last character of the input.
         *  Thus start must be greater than end, and not simply equal to it.
         */
        if(start > end)
            return Match::Empty;

        return _regex->_match(this->Input, start, end - start, _index + (_length ? _length : 1));
    }

    //String^ Match::Result(String^ replacement)
    //{
    //    if(!replacement)
    //        throw gcnew ArgumentNullException("replacement");
    //    if(!_regex)
    //        throw gcnew NotSupportedException("NoResultOnFailed");

    //    //
    //    // Add RE2's replacement style.
    //    return replacement;
    //    //
    //}

    Match^ Match::Synchronized(Match^ inner)
    {
        if(!inner)
            throw gcnew ArgumentNullException("inner");

        int length = inner->_groupcount;
        for(int i = 0; i < length; i++)
        {
            Group^ group = inner->Groups[i];
            Group::Synchronized(group);
        }

        return inner;
    }
}
}