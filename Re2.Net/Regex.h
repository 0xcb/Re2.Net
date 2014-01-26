/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  Use of Re2.Net is governed by the GNU Affero General Public
 *  License, Version 3, which can be found in the LICENSES file
 *  and at http://www.gnu.org/licenses/agpl-3.0.html.
 *
 *  If the AGPLv3 doesn't suit your needs, you can contact me at
 *  mailto:coltblackmore+re2.net@gmail.com.
 *
 *
 *  RE2 Copyright © 2003-2009 The RE2 Authors. All Rights Reserved.
 *
 *  Use of RE2 is governed by a BSD-style license, which can be found in
 *  the LICENSES file and at http://re2.googlecode.com/hg/LICENSE.
 */

/*
 *  C++/CLI Notes:
 *
 *  [+] C++/CLI can't target .NET v3.5 using msvcr100, even though it
 *      claims to in 'Project -> Properties'. VC++ 2008 (msvcr90) with the
 *      libraries extension feature pack is required to link to RE2.
 *      
 *  [+] VC++ 2008 SP1 is required to fix a linker error.
 *
 *  [+] VC++ 2008 hotfix KB976656 is required to fix a compiler error with
 *      using managed object methods.
 *
 *  [+] '$(FrameworkSdkDir)lib\x64' is added to 'Library Directories' in
 *      'Project Properties -> VC++ Directories' for the x64 Platform.
 *      Otherwise Visual Studio can't find it when compiling under msvcr90.
 *
 *  [+] A reference to System.Core.dll has to be added manually for VS2012.
 *      .NET 4.5 moved ExtensionAttribute from System.Core to mscorlib,
 *      but only the latter (mscorlib.dll) is referenced by default.
 *
 *  [+] C++/CLI is generally brittle in the sense of throwing gibberish
 *      linker and compiler errors. You may need to do some
 *      troubleshooting to get things working in your environment.
 *
 *
 *  Implementation Notes:
 *
 *  [+] The maximum memory usable by an RE2 object cannot currently be set
 *      with static methods. This may be changed in the future, but the
 *      default of 8 MB is enough to handle all but the most complex
 *      expressions. For larger value, simply use instance rather than
 *      static methods.
 */

#pragma once

/*
 *  Lib Requirements:
 *
 *  Using the Re2.Net source requires that debug and release RE2 libs
 *  for both x86 and x64 be placed in the following directories relative
 *  to the solution root:
 *
 *      x64 Debug:   .. re2\lib\x64\debug\re2.lib
 *      x64 Release: .. re2\lib\x64\release\re2.lib
 *      x86 Debug:   .. re2\lib\x86\debug\re2.lib
 *      x86 Release: .. re2\lib\x86\release\re2.lib
 *
 *  A Windows RE2 port that can be used to generate the required libs is
 *  available at http://code.google.com/p/re2win/. Note though that it
 *  hasn't been updated since 2011/11, whereas RE2 is actively maintained,
 *  with the latest release appearing in 2013/10.
 * 
 *  A better approach might be to build RE2 with CMake. See:
 *      https://groups.google.com/forum/#!topic/re2-dev/u5T0KDGuSHA.
 *
 *  Or with MinGW. See:
 *      https://groups.google.com/forum/#!topic/re2-dev/BmXX5mk7Sts.
 *
 *  Or with Cygwin. See:
 *      http://stackoverflow.com/questions/1981043
 *
 *  The latest release of RE2 itself is always available from the RE2
 *  public repository at http://code.google.com/p/re2/downloads.
 *
 *  NB: With Re2.Net finally posted, I'll be evaluating methods of
 *      compiling for Windows directly from the Unix source next.
 *      That information will appear here when I have it.
 */

/* Import the appropriate platform and configuration of re2.lib. */
#ifdef _M_AMD64
    #ifdef _DEBUG
        #pragma comment(lib, "re2\\lib\\x64\\debug\\re2.lib")
    #else
        #pragma comment(lib, "re2\\lib\\x64\\release\\re2.lib")
    #endif
#else
    #ifdef _DEBUG
        #pragma comment(lib, "re2\\lib\\x86\\debug\\re2.lib")
    #else
        #pragma comment(lib, "re2\\lib\\x86\\release\\re2.lib")
    #endif
#endif

#pragma managed(push, off)
    #include "re2\src\re2.h"
    #include "re2\src\stringpiece.h"
#pragma managed(pop)

#include "RegexOptions.h"
#include "RegexInput.h"
#include "Match.h"
#include "MatchCollection.h"


namespace Re2
{
namespace Net
{
    using namespace System;

    using System::Collections::Generic::Dictionary;
    using System::Collections::Generic::List;

    using re2::RE2;
    using re2::StringPiece;

    ref class Match;
    ref class MatchCollection;

    /*
     *  The compiler is unable to distinguish between types and members
     *  that share a name. Aliasing is used throughout the project to
     *  resolve this ambiguity.
     */
    typedef Match _Match;


    /// <summary>
    ///     Represents an immutable regular expression.
    /// </summary>
    public ref class Regex sealed
	{
        #pragma region Private members

        private:

            /*
             *  _re2 : The internal RE2 object. Don't call MemerwiseClone() on Regex instances.
             *         (Note that the class itself is sealed to prevent users from doing this.)
             */
            const RE2* _re2;


            /*
             *  REGEX_OPTIONS_MAX    : The upper bound on valid RegexOptions input. The lower bound is
             *                         always zero, represented by RegexOptions::None.
             *
             *  SINGLE_BYTE_ENCODING : RegexOptions set to either of the available single-byte encodings.
             */
            static initonly RegexOptions REGEX_OPTIONS_MAX    = RegexOptions(1 << (Enum::GetNames(RegexOptions::typeid)->Length - 2));
            static initonly RegexOptions SINGLE_BYTE_ENCODING = RegexOptions::Latin1 | RegexOptions::ASCII;

        #pragma endregion


        #pragma region Regex cache

        private:

            /*
             *  A cache of expressions used in calls to the static matching methods.
             *  Mirrors the cache in System.Text.RegularExpressions.Regex, which is
             *  a silly optimization in all but the most extraordinary of cases.
             *
             *  See: http://msdn.microsoft.com/en-us/library/system.text.regularexpressions.regex.cachesize.aspx
             */
            ref class Cache abstract sealed
            {
                private:

                    static Dictionary<String^, Regex^> _map;
                    static List<Regex^>                _list;


                internal:

                    static int _size = 15;

                    static property int Size { void set(int value); }

                    /*
                     *  Returns a cached regex if one is available, otherwise creates a new
                     *  regex and adds it to the cache.
                     *
                     *  NB: Regex options are immutable, so when caching and retrieving a
                     *      regex, both the pattern and the options are taken into account.
                     */
                    static Regex^ FindOrCreate(String^ pattern, RegexOptions options);
            };


        public:

            /// <summary>
            ///     Gets or sets the maximum number of entries in the current static cache of compiled regular expressions.
            /// </summary>
            /// <value>
            ///     The maximum number of entries in the static cache.
            /// </value>
            /// <exception cref="System::ArgumentOutOfRangeException">
            ///     <paramref name="value"/> is less than zero.
            /// </exception>
            static property int CacheSize
            {
                int  get();
                void set(int value);
            }

        #pragma endregion
            

        #pragma region Regex properties and methods

        private:

            initonly int          _maxMemory;
            initonly String^      _pattern;
            initonly RegexOptions _options;

        public:
            
            /// <summary>
            ///     Gets the regular expression pattern that was passed into the <c>Regex</c> constructor.
            /// </summary>
            /// <value>
            ///     The regular expression pattern that was passed into the <c>Regex</c> constructor.
            /// </value>
            property String^ Pattern { String^ get(); }
            

            /// <summary>
            ///     Gets the bitwise set of <see cref="RegexOptions"/> that were passed into the <see cref="Regex"/> constructor.
            /// </summary>
            /// <value>
            ///     One or more members of the <see cref="RegexOptions"/> enumeration that represent options that were passed to the <see cref="Regex"/> constructor.
            /// </value>
            property RegexOptions Options { RegexOptions get(); }
            

            /// <summary>
            ///     Gets the maximum memory that can be used by the current instance.
            /// </summary>
            /// <value>
            ///     The maximum memory, in bytes, that the underlying <c>Regex</c> automata can collectively consume.
            /// </value>
            property int MaxMemory { int get(); }


            /// <summary>
            ///     Returns the regular expression pattern that was passed into the <c>Regex</c> constructor.
            /// </summary>
            /// <returns>The pattern parameter that was passed into the <c>Regex</c> constructor.</returns>
            virtual String^ ToString() override;


            /// <summary>
            ///     Returns the group number that corresponds to the specified group name.
            /// </summary>
            /// <param name="name">The group name to convert to the corresponding group number.</param>
            /// <exception cref="System::ArgumentNullException">
            ///     <paramref name="name"/> is <c>null</c>.
            /// </exception>
            /// <exception cref="System::ArgumentOutOfRangeException">
            ///     <para><paramref name="name"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
            ///     <para>- or -</para>
            ///     <para><paramref name="name"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
            /// </exception>
            int GroupNumberFromName(String^ name);

        #pragma endregion


        #pragma region Regex matching methods

            #pragma region IsMatch

            public:

                /// <summary>
                ///     Indicates whether the regular expression specified in the <c>Regex</c> constructor finds a match in the specified
                ///     input string, beginning at the specified starting index in the string.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="startIndex">The input index at which to start the search.</param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                bool IsMatch(String^ input, int startIndex);


                /// <summary>
                ///     Indicates whether the regular expression specified in the <c>Regex</c> constructor finds a match in the specified
                ///     input byte array, beginning at the specified starting index.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="startIndex">The input index at which to start the search.</param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.
                /// </exception>
                bool IsMatch(array<Byte>^ input, int startIndex);


                /// <summary>
                ///     Indicates whether the regular expression specified in the <c>Regex</c> constructor finds a match in the specified
                ///     input string.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="input"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                bool IsMatch(String^ input);


                /// <summary>
                ///     Indicates whether the regular expression specified in the <see cref="Regex"/> constructor finds a match in the specified
                ///     input byte array.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                bool IsMatch(array<Byte>^ input);


                /// <summary>
                ///     Indicates whether the specified regular expression finds a match in the specified input string,
                ///     using the specified matching options.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <param name="options">A bitwise combination of the enumeration values that modify the regular expression.</param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="options"/> is not a valid <c>RegexOptions</c> value.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                static bool IsMatch(String^ input, String^ pattern, RegexOptions options);


                /// <summary>
                ///     Indicates whether the specified regular expression finds a match in the specified input byte array,
                ///     using the specified matching options.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <param name="options">A bitwise combination of the enumeration values that modify the regular expression.</param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <paramref name="options"/> is not a valid <c>RegexOptions</c> value.
                ///     <para>- or -</para>
                ///     <para><paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                static bool IsMatch(array<Byte>^ input, String^ pattern, RegexOptions options);


                /// <summary>
                ///     Indicates whether the specified regular expression finds a match in the specified input string.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                static bool IsMatch(String^ input, String^ pattern);
                /* ArgumentOutOfRangeExceptions for encoding can't be thrown if no RegexOptions are provided. */


                /// <summary>
                ///     Indicates whether the specified regular expression finds a match in the specified input byte array.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <returns><c>true</c> if the regular expression finds a match; otherwise, <c>false</c>.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                static bool IsMatch(array<Byte>^ input, String^ pattern);
                /* ArgumentOutOfRangeExceptions for encoding can't be thrown if no RegexOptions are provided. */

            #pragma endregion


            #pragma region Match

            internal:
                
                _Match^ _match(RegexInput^ input, int startIndex, int length, int stringStartIndex);


            public:

                /// <summary>
                ///     Searches the input string for the first occurrence of a regular expression, beginning at the specified starting
                ///     position and searching only the specified number of characters.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="startIndex">The input index at which to start the search.</param>
                /// <param name="length">The number of characters in the substring to include in the search.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="length"/> is less than zero or greater than the length of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><c><paramref name="startIndex"/> + <paramref name="length"/> – 1</c> identifies a position that is outside the range of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                _Match^ Match(String^ input, int startIndex, int length);
                

                /// <summary>
                ///     Searches the input byte array for the first occurrence of a regular expression, beginning at the specified starting
                ///     position and searching only the specified number of bytes.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="startIndex">The input index at which to start the search.</param>
                /// <param name="length">The number of bytes in the input to include in the search.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="length"/> is less than zero or greater than the length of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><c><paramref name="startIndex"/> + <paramref name="length"/> – 1</c> identifies a position that is outside the range of <paramref name="input"/>.</para>
                /// </exception>
                _Match^ Match(array<Byte>^ input, int startIndex, int length);
                

                /// <summary>
                ///     Searches the input string for the first occurrence of a regular expression, beginning at the specified starting
                ///     position in the string.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="startIndex">The zero-based input index at which to start the search.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                _Match^ Match(String^ input, int startIndex);
                

                /// <summary>
                ///     Searches the input byte array for the first occurrence of a regular expression, beginning at the specified starting
                ///     position.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="startIndex">The input index at which to start the search.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.
                /// </exception>
                _Match^ Match(array<Byte>^ input, int startIndex);


                /// <summary>
                ///     Searches the input string for the first occurrence of a regular expression.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="input"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                _Match^ Match(String^ input);


                /// <summary>
                ///     Searches the input byte array for the first occurrence of a regular expression.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                _Match^ Match(array<Byte>^ input);


                /// <summary>
                ///     Searches the input string for the first occurrence of the specified regular expression, using the specified matching options.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <param name="options">A bitwise combination of the enumeration values that specify options for matching.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="options"/> is not a valid <c>RegexOptions</c> value.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                static _Match^ Match(String^ input, String^ pattern, RegexOptions options);


                /// <summary>
                ///     Searches the input byte array for the first occurrence of the specified regular expression, using the specified matching options.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <param name="options">A bitwise combination of the enumeration values that specify options for matching.</param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <paramref name="options"/> is not a valid <c>RegexOptions</c> value.
                ///     <para>- or -</para>
                ///     <para><paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                static _Match^ Match(array<Byte>^ input, String^ pattern, RegexOptions options);


                /// <summary>
                ///     Searches the input string for the first occurrence of the specified regular expression.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                static _Match^ Match(String^ input, String^ pattern);
                /* ArgumentOutOfRangeExceptions for encoding can't be thrown if no RegexOptions are provided. */


                /// <summary>
                ///     Searches the input byte array for the first occurrence of the specified regular expression.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <returns>An object that contains information about the match.</returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                static _Match^ Match(array<Byte>^ input, String^ pattern);
                /* ArgumentOutOfRangeExceptions for encoding can't be thrown if no RegexOptions are provided. */

            #pragma endregion


            #pragma region Matches

                /// <summary>
                ///     Searches the specified input string for all occurrences of a regular expression, beginning at the specified
                ///     starting position in the string.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="startIndex">The input index at which to start the search.</param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                MatchCollection^ Matches(String^ input, int startIndex);


                /// <summary>
                ///     Searches the specified input byte array for all occurrences of a regular expression, beginning at the specified
                ///     starting position.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="startIndex">The input index at which to start the search.</param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <paramref name="startIndex"/> is less than zero or greater than the length of <paramref name="input"/>.
                /// </exception>
                MatchCollection^ Matches(array<Byte>^ input, int startIndex);


                /// <summary>
                ///     Searches the specified input string for all occurrences of a regular expression.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="input"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                MatchCollection^ Matches(String^ input);


                /// <summary>
                ///     Searches the specified input byte array for all occurrences of a regular expression.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> is <c>null</c>.
                /// </exception>
                MatchCollection^ Matches(array<Byte>^ input);


                /// <summary>
                ///     Searches the specified input string for all occurrences of the specified regular expression, using the
                ///     specified matching options.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <param name="options">A bitwise combination of the enumeration values that specify options for matching.</param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="options"/> is not a valid <c>RegexOptions</c> value.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="input"/> or <paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                static MatchCollection^ Matches(String^ input, String^ pattern, RegexOptions options);


                /// <summary>
                ///     Searches the specified input byte array for all occurrences of the specified regular expression, using the
                ///     specified matching options.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <param name="options">A bitwise combination of the enumeration values that specify options for matching.</param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                /// <exception cref="System::ArgumentOutOfRangeException">
                ///     <para><paramref name="options"/> is not a valid <c>RegexOptions</c> value.</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
                ///     <para>- or -</para>
                ///     <para><paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
                /// </exception>
                static MatchCollection^ Matches(array<Byte>^ input, String^ pattern, RegexOptions options);


                /// <summary>
                ///     Searches the specified input string for all occurrences of the specified regular expression.
                /// </summary>
                /// <param name="input">The string to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                static MatchCollection^ Matches(String^ input, String^ pattern);
                /* ArgumentOutOfRangeExceptions for encoding can't be thrown if no RegexOptions are provided. */


                /// <summary>
                ///     Searches the specified input byte array for all occurrences of the specified regular expression.
                /// </summary>
                /// <param name="input">The byte array to search for a match.</param>
                /// <param name="pattern">
                ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
                ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
                /// </param>
                /// <returns>
                ///     A collection of the <see cref="Re2::Net::Match"/> objects found by the search. If no matches are found, the method
                ///     returns an empty collection object.
                /// </returns>
                /// <exception cref="System::ArgumentException">
                ///     A regular expression parsing error occurred.
                /// </exception>
                /// <exception cref="System::ArgumentNullException">
                ///     <paramref name="input"/> or <paramref name="pattern"/> is <c>null</c>.
                /// </exception>
                static MatchCollection^ Matches(array<Byte>^ input, String^ pattern);
                /* ArgumentOutOfRangeExceptions for encoding can't be thrown if no RegexOptions are provided. */

            #pragma endregion

        #pragma endregion


        #pragma region Constructors and cleanup

        public:

            /// <summary>
            ///     Initializes a new instance of the <c>Regex</c> class for the specified regular expression, with options that
            ///     modify the pattern and a value that specifies the maximum amount of memory usable by the <c>Regex</c>.
            /// </summary>
            /// <param name="pattern">
            ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">http://code.google.com/p/re2/wiki/Syntax</a>
            ///     for the list of regular expression syntax accepted by Re2.Net.
            /// </param>
            /// <param name="options">A bitwise combination of the enumeration values that modify the regular expression.</param>
            /// <param name="maxMemory">The maximum amount of memory usable by the compiled <c>Regex</c>, in bytes. The default is 8 megabytes.</param>
            /// <exception cref="System::ArgumentException">
            ///     A regular expression parsing error occurred.
            /// </exception>
            /// <exception cref="System::ArgumentNullException">
            ///     <para><paramref name="pattern"/> is <c>null</c>.</para>
            ///     <para>- or -</para>
            ///     <para><paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
            ///     <para>- or -</para>
            ///     <para><paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
            /// </exception>
            /// <exception cref="System::ArgumentOutOfRangeException">
            ///     <paramref name="options"/> is not a valid <c>RegexOptions</c> value.
            /// </exception>
            Regex(String^ pattern, RegexOptions options, int maxMemory);


            /// <summary>
            ///     Initializes a new instance of the <c>Regex</c> class for the specified regular expression, with options that modify the pattern.
            /// </summary>
            /// <param name="pattern">
            ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">http://code.google.com/p/re2/wiki/Syntax</a>
            ///     for the list of regular expression syntax accepted by Re2.Net.
            /// </param>
            /// <param name="options">A bitwise combination of the enumeration values that modify the regular expression.</param>
            /// <exception cref="System::ArgumentException">
            ///     A regular expression parsing error occurred.
            /// </exception>
            /// <exception cref="System::ArgumentNullException">
            ///     <para><paramref name="pattern"/> is <c>null</c>.</para>
            ///     <para>- or -</para>
            ///     <para><paramref name="pattern"/> is not a valid Latin-1 string (flag <c>RegexOptions.Latin1</c> is set).</para>
            ///     <para>- or -</para>
            ///     <para><paramref name="pattern"/> is not a valid ASCII string (flag <c>RegexOptions.ASCII</c> is set).</para>
            /// </exception>
            /// <exception cref="System::ArgumentOutOfRangeException">
            ///     <paramref name="options"/> is not a valid <c>RegexOptions</c> value.
            /// </exception>
            Regex(String^ pattern, RegexOptions options);


            /// <summary>
            ///     Initializes a new instance of the <c>Regex</c> class for the specified regular expression.
            /// </summary>
            /// <param name="pattern">
            ///     The regular expression pattern to match. See <a href="http://code.google.com/p/re2/wiki/Syntax">
            ///     http://code.google.com/p/re2/wiki/Syntax</a> for the list of regular expression syntax accepted by Re2.Net.
            /// </param>
            /// <exception cref="System::ArgumentException">
            ///     A regular expression parsing error occurred.
            /// </exception>
            /// <exception cref="System::ArgumentNullException">
            ///     <paramref name="pattern"/> is <c>null</c>.
            /// </exception>
            Regex(String^ pattern);


            ~Regex();


        private:

            static initonly array<String^>^ _errorTable = gcnew array<String^>(RE2::ErrorPatternTooLarge + 1);

            static Regex();


        protected:

            !Regex();

        #pragma endregion
	};
}
}