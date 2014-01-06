/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Group.h"
#include "GroupCollection.h"
#include "GroupEnumerator.h"
#include "Match.h"


namespace Re2
{
namespace Net
{
    GroupCollection::GroupCollection(Match^ match)
        : _match(match),
          _groups(gcnew array<Group^>(match->_groupcount))
    {
        _groups[0] = _match;
    }


    /*
     *  Regex::GroupNumberFromName() returns -1 if the name doesn't exist. GetGroup() therefore
     *  allows invalid values, and simply returns Group::Empty where normally we would expect an
     *  an exception to be thrown. (This is consistent with .NET GroupCollection behavior.)
     */
    Group^ GroupCollection::GetGroup(int groupNumber)
    {
        if(groupNumber < 0 || groupNumber >= _groups->Length)
            return Group::Empty;
        return _groups[groupNumber];
    }


    int GroupCollection::Count::get()
    {
        return _match->_groupcount;
    }


    bool GroupCollection::IsReadOnly::get()
    {
        return true;
    }


    bool GroupCollection::IsSynchronized::get()
    {
        return false;
    }


    Group^ GroupCollection::default::get(int groupNumber)
    {
        return this->GetGroup(groupNumber);
    }


    void GroupCollection::default::set(int groupNumber, Group^ group)
    {
        _groups[groupNumber] = group;
    }


    Group^ GroupCollection::default::get(String^ groupName)
    {
        if(!_match->_regex)
            return Group::Empty;
        return this->GetGroup(_match->_regex->GroupNumberFromName(groupName));
    }


    Object^ GroupCollection::SyncRoot::get()
    {
        return _match;
    }


    void GroupCollection::CopyTo(Array^ array, int arrayIndex)
    {
        if(!array)
            throw gcnew ArgumentNullException("array");

        for(int i = 0; i < this->Count; i++)
        {
            array->SetValue(this[i], arrayIndex);
            arrayIndex++;
        }
    }


    IEnumerator^ GroupCollection::GetEnumerator()
    {
        return gcnew GroupEnumerator(this);
    }
}
}