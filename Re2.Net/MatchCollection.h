/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Regex.h"
#include "Match.h"


namespace Re2
{
namespace Net
{
    using namespace System;

    ref class Regex;
    ref class Match;


    /*
     *  The contents of the .NET MatchCollection class are lazily evaluated, which, like
     *  Match.NextMatch(), requires MatchCollection to be written as a wrapper around the
     *  Regex class. Re2.Net preserves this behavior so that the work of finding a match
     *  is postponed -- perhaps indefinitely -- until the code explicitly operates on it.
     */

    /// <summary>
    ///     Represents the set of successful matches found by iteratively applying a regular expression pattern to the input.
    /// </summary>
    [/*Serializable,*/ DefaultMember("Item")]
    public ref class MatchCollection : public ICollection
    {
        internal:

            int        _done;
            Match^     _match;
            ArrayList^ _matches;
        

        private:

            static int _infinite = 0x7fffffff;


        internal:
        
            MatchCollection(Match^ match);

            Match^ GetMatch(int i);


        public:
        
            /// <summary>
            ///     Copies all the elements of the collection to the given array starting at the given index.
            /// </summary>
            /// <param name="array">The array the collection is to be copied into.</param>
            /// <param name="arrayIndex">The position in the array where copying is to begin.</param>
            /// <exception cref="System::ArgumentNullException">
            ///     <paramref name="array"/> is <c>null</c>.
            /// </exception>
            /// <exception cref="System::ArgumentOutOfRangeException">
            ///     <para><paramref name="arrayIndex"/> is outside the bounds of <paramref name="array"/>.</para>
            ///     <para>- or -</para>
            ///     <para>
            ///         <paramref name="arrayIndex"/> plus <see cref="MatchCollection::Count"/> is outside the bounds of <paramref name="array"/>.
            ///     </para>
            /// </exception>
            virtual void CopyTo(Array^ array, int arrayIndex);


            /// <summary>
            ///     Provides an enumerator that iterates through the collection.
            /// </summary>
            /// <returns>An object that contains all <see cref="Match"/> objects within the MatchCollection.</returns>
            virtual IEnumerator^ GetEnumerator();


            /// <summary>
            ///     Gets the number of matches.
            /// </summary>
            /// <value>
            ///     The number of matches.
            /// </value>
            virtual property int Count { int get(); }


            /// <summary>
            ///     Gets a value that indicates whether the collection is read only.
            /// </summary>
            /// <value>
            ///     <c>true</c> in all cases.
            /// </value>
            property bool IsReadOnly { bool get(); }


            /// <summary>
            ///     Gets a value indicating whether access to the collection is synchronized (thread-safe).
            /// </summary>
            /// <value>
            ///     <c>false</c> in all cases.
            /// </value>
            virtual property bool IsSynchronized { bool get(); }


            /// <summary>
            ///     Gets an individual member of the collection.
            /// </summary>
            /// <param name="i">Index into the <see cref="Match"/> collection.</param>
            /// <value>
            ///     The captured expression at position <paramref name="i"/> in the collection.
            /// </value>
            /// <exception cref="System::ArgumentOutOfRangeException">
            ///     <paramref name="i"/> is less than 0 or greater than or equal to <see cref="Count"/>.
            /// </exception>
            virtual property Match^ default[int] { Match^ get(int i); }


            /// <summary>
            ///     Gets an object that can be used to synchronize access to the collection.
            /// </summary>
            /// <value>
            ///     An object that can be used to synchronize access to the collection. This property always returns the object itself.
            /// </value>
            virtual property Object^ SyncRoot { Object^ get(); }
    };
}
}