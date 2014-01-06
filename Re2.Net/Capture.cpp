/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "RegexInput.h"


namespace Re2
{
namespace Net
{
    Capture::Capture(RegexInput^ input, int index, int length)
    {
        _input  = input;
        _index  = index;
        _length = length;
    }

    RegexInput^ Capture::Input::get()
    {
        return _input;
    }

    int Capture::Index::get()
    {
        return _index;
    }

    int Capture::Length::get()
    {
        return _length;
    }

    String^ Capture::Value::get()
    {
        if(_input->Bytes)
        {
            if(_input->IsUTF8)
                return _utf8Encoding->GetString(_input->Bytes, _index, _length);
            else
                return _latin1Encoding->GetString(_input->Bytes, _index, _length);
        }
        else
            return _input->Input->Substring(_index, _length);
    }

    String^ Capture::ToString()
    {
        return this->Value;
    }
}
}