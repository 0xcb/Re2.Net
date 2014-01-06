/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "Group.h"


namespace Re2
{
namespace Net
{
    using namespace System;
    using namespace System::Collections;
    using namespace System::Reflection;

    ref class Capture;
    ref class Group;


    /// <summary>
    ///     Represents the set of captures made by a single capturing group.
    /// </summary>
    [/*Serializable,*/ DefaultMember("Item")]
    public ref class CaptureCollection : public ICollection
    {
        internal:
            
            int    _capcount;
            Group^ _group;

            CaptureCollection(Group^ group);

            Capture^ GetCapture(int i);


        public:

            /// <summary>
            ///     Gets the number of subexpressions captured by the group.
            /// </summary>
            /// <value>
            ///     The number of items in the <see cref="CaptureCollection"/>.
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
            ///     Gets a value that indicates whether access to the collection is synchronized (thread-safe).
            /// </summary>
            /// <value>
            ///     <c>false</c> in all cases.
            /// </value>
            virtual property bool IsSynchronized { bool get(); }


            /// <summary>
            ///     Gets an individual member of the collection.
            /// </summary>
            /// <param name="i">Index into the <see cref="Capture"/> collection.</param>
            /// <value>
            ///     The captured subexpression at position <paramref name="i"/> in the collection.
            /// </value>
            /// <exception cref="System::ArgumentOutOfRangeException">
            ///     <paramref name="i"/> is less than 0 or greater than or equal to <see cref="Count"/>.
            /// </exception>
            property Capture^ default[int] { Capture^ get(int i); }


            /// <summary>
            ///     Gets an object that can be used to synchronize access to the collection.
            /// </summary>
            /// <value>
            ///     A copy of the <see cref="Group"/> object to synchronize.
            /// </value>
            virtual property Object^ SyncRoot { Object^ get(); }


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
            ///         <paramref name="arrayIndex"/> plus <see cref="CaptureCollection::Count"/> is outside the bounds of <paramref name="array"/>.
            ///     </para>
            /// </exception>
            virtual void CopyTo(Array^ array, int arrayIndex);


            /// <summary>
            ///     Provides an enumerator that iterates through the collection.
            /// </summary>
            /// <returns>An object that contains all <see cref="Capture"/> objects within the <c>CaptureCollection</c>.</returns>
            virtual IEnumerator^ GetEnumerator();
    };
}
}