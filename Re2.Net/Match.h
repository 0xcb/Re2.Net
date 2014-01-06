/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "Group.h"
#include "GroupCollection.h"
#include "Regex.h"
#include "RegexInput.h"


namespace Re2
{
namespace Net
{
    using namespace System;
    
    using System::Security::Permissions::HostProtectionAttribute;
    using System::Security::Permissions::SecurityAction;

    using re2::StringPiece;

    ref class Group;
    ref class GroupCollection;
    ref class Regex;


    /// <summary>
    ///     Represents the results from a single regular expression match.
    /// </summary>
    /*[Serializable]*/
    public ref class Match : public Group
    {
        internal:
            
            static initonly Match^ _empty = gcnew Match(nullptr, 1, RegexInput::Empty, 0, 0, 0);

            Regex^           _regex;
            RegexInput^      _input;
            GroupCollection^ _groupcoll;
            int              _groupcount;
            int              _nextpos;

            Match(Regex^ regex, int groupcount, RegexInput^ input, int begpos, int len, int nextpos);

            /*
             *  With only one capture per group and no backtracking, RE2 doesn't need the many
             *  internal methods that clutter up System.Text.RegularExpressions.Match.
             */

        public:
                
            /// <summary>
            ///     Gets the empty group. All failed matches return this empty match.
            /// </summary>
            /// <value>
            ///     An empty match.
            /// </value>
            property static Match^ Empty { Match^ get(); }


            /// <summary>
            ///     Gets a collection of the capturing groups in the regular expression.
            /// </summary>
            /// <value>
            ///     The capturing groups in the regular expression.
            /// </value>
            property virtual GroupCollection^ Groups { GroupCollection^ get(); }


            /// <summary>
            ///     Returns a new <see cref="Match"/> object with the results for the next match, starting at the position at
            ///     which the last match ended (at the character after the last matched character).
            /// </summary>
            /// <returns>The next regular expression match.</returns>
            Match^ NextMatch();


            ///// <summary>
            /////     Returns the expansion of the specified replacement pattern.
            ///// </summary>
            ///// <param name="replacement">The replacement pattern to use.</param>
            ///// <returns>The expanded version of the <paramref name="replacement"/> parameter.</returns>
            ///// <exception cref="System::ArgumentNullException">
            /////     <paramref name="replacement"/> is <c>null</c>.
            ///// </exception>
            ///// <exception cref="System::NotSupportedException">
            /////     Expansion is not allowed for this pattern.
            ///// </exception>
            //virtual String^ Result(String^ replacement);


            /// <summary>
            ///     Returns a <see cref="Match"/> instance equivalent to the one supplied that is suitable to share between multiple threads.
            /// </summary>
            /// <param name="inner">A regular expression match equivalent to the one expected.</param>
            /// <returns>A regular expression match that is suitable to share between multiple threads.</returns>
            /// <exception cref="System::ArgumentNullException">
            ///     <paramref name="inner"/> is <c>null</c>.
            /// </exception>
            [HostProtection(SecurityAction::LinkDemand, Synchronization=true)]
            static Match^ Synchronized(Match^ inner);
    };
}
}