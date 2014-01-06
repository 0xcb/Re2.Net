/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "CaptureCollection.h"
#include "CaptureEnumerator.h"
#include "Group.h"


namespace Re2
{
namespace Net
{
    CaptureCollection::CaptureCollection(Group^ group)
    {
        _group    = group;
        _capcount = this->_group->_capcount;
    }


    Capture^ CaptureCollection::GetCapture(int i)
    {
        /* In RE2 groups only have one capture, which is the group itself. */
        if(i >= _capcount || i < 0)
            throw gcnew ArgumentOutOfRangeException("i");
        return _group;
    }


    int CaptureCollection::Count::get()
    {
        return _capcount;
    }


    bool CaptureCollection::IsReadOnly::get()
    {
        return true;
    }


    bool CaptureCollection::IsSynchronized::get()
    {
        return false;
    }


    Capture^ CaptureCollection::default::get(int i)
    {
        return this->GetCapture(i);
    }


    Object^ CaptureCollection::SyncRoot::get()
    {
        return _group;
    }


    void CaptureCollection::CopyTo(Array^ array, int arrayIndex)
    {
        if(!array)
            throw gcnew ArgumentNullException("array");

        for(int i = 0; i < this->Count; i++)
            array->SetValue(this[i], arrayIndex++);
    }


    IEnumerator^ CaptureCollection::GetEnumerator()
    {
        return gcnew CaptureEnumerator(this);
    }
}
}