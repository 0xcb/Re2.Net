/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Group.h"
#include "Match.h"


namespace Re2
{
namespace Net
{
    using namespace System;
    using namespace System::Collections;
    using namespace System::Reflection;

    ref class Group;
    ref class Match;


    /// <summary>
    ///     Represents the set of captured groups from a single match.
    /// </summary>
    [/*Serializable,*/ DefaultMember("Item")]
    public ref class GroupCollection : public ICollection
    {
        internal:
            
            array<Group^>^ _groups;
            Match^         _match;

            GroupCollection(Match^ match);

            Group^ GetGroup(int groupNumber);


        public:

            /// <summary>
            ///     Gets the number of groups in the collection.
            /// </summary>
            /// <value>
            ///     The number of groups in the <see cref="GroupCollection"/>.
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
            ///     Gets an object that can be used to synchronize access to the collection.
            /// </summary>
            /// <value>
            ///     A copy of the <see cref="Match"/> object to synchronize.
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
            /// <returns>An object that contains all <see cref="Group"/> objects within the <c>GroupCollection</c>.</returns>
            virtual IEnumerator^ GetEnumerator();


            /*
             *  When accessing the default (Item) property with an invalid argument,
             *  GroupCollections return Group::Empty rather than throwing an exception.
             *  This is true for groupNumber as well as groupName.
             */


            /// <summary>
            ///     Gets an individual member of the collection by name.
            /// </summary>
            /// <param name="groupName">The name of a capturing group.</param>
            /// <value>
            ///     The member of the collection specified by <paramref name="groupName"/>, or an empty <see cref="Group"/>.
            /// </value>
            property Group^ default[String^] { Group^ get(String^ groupName); }


            /// <summary>
            ///     Gets an individual member of the collection by index.
            /// </summary>
            /// <param name="groupNumber">Index into the <see cref="Group"/> collection.</param>
            /// <value>
            ///     The member of the collection specified by <paramref name="groupNumber"/>, or an empty <see cref="Group"/>.
            /// </value>
            property Group^ default[int]
            {
                public:   Group^ get(int groupNumber);
                internal: void   set(int groupNumber, Group^ group);
            }
    };
}
}