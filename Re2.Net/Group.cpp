/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "CaptureCollection.h"
#include "Group.h"
#include "RegexInput.h"


namespace Re2
{
namespace Net
{
    Group::Group(RegexInput^ input, int index, int length)
        : Capture(input, index, length),
          _capcount(RegexInput::Empty == input ? 0 : 1)
    { }

    Group^ Group::Empty::get()
    {
        return Group::_emptygroup;
    }

    CaptureCollection^ Group::Captures::get()
    {
        if(!_capcoll)
            _capcoll = gcnew CaptureCollection(this);

        return _capcoll;
    }

    Group^ Group::Synchronized(Group^ inner)
    {
        if(!inner)
            throw gcnew ArgumentNullException("inner");

        return inner;
    }

    bool Group::Success::get()
    {
        return RegexInput::Empty != _input;
    }
}
}