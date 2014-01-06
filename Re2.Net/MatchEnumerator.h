/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Match.h"
#include "MatchCollection.h"


namespace Re2
{
namespace Net
{
    using namespace System;

    ref class Match;
    ref class MatchCollection;


    /*[Serializable]*/
    private ref class MatchEnumerator : public IEnumerator
    {
        internal:
        
            int              _curindex;
            bool             _done;
            Match^           _match;
            MatchCollection^ _matchcoll;

            MatchEnumerator(MatchCollection^ matchcoll);


        public:
        
            virtual bool MoveNext();

            virtual void Reset();

            virtual property Object^ Current { Object^ get(); }
    };
}
}