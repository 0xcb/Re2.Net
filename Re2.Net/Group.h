/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Capture.h"
#include "CaptureCollection.h"
#include "RegexInput.h"


namespace Re2
{
namespace Net
{
    using namespace System;
    
    using System::Security::Permissions::HostProtectionAttribute;
    using System::Security::Permissions::SecurityAction;

    ref class Capture;
    ref class CaptureCollection;


    /*
     *  RE2 only retains the final capture for each group, so Groups are fundamentally a
     *  thin wrapper around Captures; their only purpose is compatibility with the .NET
     *  Regex class.
     */

    /// <summary>
    ///     Represents the results from a single capturing group.
    /// </summary>
    /*[Serializable]*/
    public ref class Group : public Capture
    {
        internal:

            CaptureCollection^     _capcoll;
            initonly int           _capcount;
            static initonly Group^ _emptygroup = gcnew Group(RegexInput::Empty, 0, 0);

            static property Group^ Empty { Group^ get(); }

            Group(RegexInput^ input, int index, int length);

            
        public:

            /*
             *  The HostProtectionAttribute doesn't affect managed code, only unmanaged code that hosts
             *  the CLR and implements host protection, e.g. SQL Server. It is kept here for consistency
             *  with System.Text.RegularExpression.Group.
             */

            /// <summary>
            ///     Returns a <c>Group</c> object equivalent to the one supplied that is safe to share between multiple threads.
            /// </summary>
            /// <param name="inner">The input <see cref="Group"/> object.</param>
            /// <returns>A regular expression <c>Group</c> object.</returns>
            /// <exception cref="System::ArgumentNullException">
            ///     <paramref name="inner"/> is <c>null</c>.
            /// </exception>
            [HostProtection(SecurityAction::LinkDemand, Synchronization=true)]
            static Group^ Synchronized(Group^ inner);

            /// <summary>
            ///     Gets a collection of all the captures matched by the capturing group, in innermost-leftmost-first order.
            ///     The collection may have zero or more items.
            /// </summary>
            /// <value>
            ///     The collection of subexpressions matched by the group.
            /// </value>
            property CaptureCollection^ Captures { CaptureCollection^ get(); }

            /// <summary>
            ///     Gets a value indicating whether the match is successful.
            /// </summary>
            /// <value>
            ///     <c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.
            /// </value>
            property bool Success { bool get(); }
    };
}
}