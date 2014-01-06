/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once


namespace Re2
{
namespace Net
{
    using namespace System;
    using namespace System::Runtime::CompilerServices;


    /// <summary>
    ///     Provides enumerated values to use to set regular expression options.
    /// </summary>
    [FlagsAttribute]
    public enum class RegexOptions
    {
        /// <summary>
        ///     Specifies no options.
        /// </summary>
        None = 0,
        
        /// <summary>
        ///     Specifies case-insensitive mode. This option is equivalent to the 'i' flag.
        /// </summary>
        IgnoreCase = 1,
        
        /// <summary>
        ///     Specifies multiline mode: '^' and '$' match the start and end of any line as
        ///     well as the start and end of the input. This option is equivalent to the 'm'
        ///     flag.
        /// </summary>
        Multiline = 1 << 1,
        
        /// <summary>
        ///     Specifies that the longest match is returned, rather than the first match.
        /// </summary>
        LongestMatch = 1 << 2,

        /// <summary>
        ///     Specifies that '\n' characters are ignored, even if they appear in the regular
        ///     expression.
        /// </summary>
        IgnoreNewline = 1 << 3,

        /// <summary>
        ///     Specifies single-line mode: '.' matches '\n' in addition to other printable
        ///     characters. This option is equivalent to the 's' flag.
        /// </summary>
        Singleline = 1 << 4,
        
        /// <summary>
        ///     Specifies ungreedy mode: The greediness of repetition operators (e.g. '*' and
        ///     '*?') is reversed. This option is equivalent to the 'U' flag.
        /// </summary>
        Ungreedy = 1 << 5,
        
        /// <summary>
        ///     Specifies that the pattern is treated as a string literal rather than a regular
        ///     expression.
        /// </summary>
        Literal = 1 << 6,

        ///// <summary>
        /////     Specifies that the only valid captures are explicitly named or numbered groups of
        /////     the form (?&lt;name&gt;...). This allows unnamed parentheses to act as noncapturing
        /////     groups without the syntactic clumsiness of the expression (?:...).
        ///// </summary>
        //ExplicitCapture = 1 << 7,

        /// <summary>
        ///     Specifies that only the regular expression as a whole is captured. This significantly
        ///     increases the speed of the matching engine.
        /// </summary>
        SingleCapture = 1 << 8,

        /// <summary>
        ///     Specifies Latin-1 (ISO-8859-1) encoding. If the input is a System.String, it is
        ///     converted from Unicode, with only valid Latin-1 values accepted. This option can
        ///     be used to search for byte patterns.
        /// </summary>
        /// <remarks>
        ///     Encodings are mutually exclusive. If both the <c>Latin1</c> and <c>ASCII</c>
        ///     options are set, <c>Latin1</c> is used. If neither is set, Unicode is used.
        /// </remarks>
        Latin1 = 1 << 9,

        /// <summary>
        ///     Specifies ASCII encoding. If the input is a System.String, it is converted from
        ///     Unicode, with only valid 7-bit ASCII values accepted. This option can be used to
        ///     search for byte patterns.
        /// </summary>
        /// <remarks>
        ///     Encodings are mutually exclusive. If both the <c>Latin1</c> and <c>ASCII</c>
        ///     options are set, <c>Latin1</c> is used. If neither is set, Unicode is used.
        /// </remarks>
        ASCII = 1 << 10,
        
        /// <summary>
        ///     Specifies POSIX mode with egrep syntax.
        /// </summary>
        POSIX = 1 << 11,

        /// <summary>
        ///     Enables Perl's digit ('\d'), whitespace ('\s'), and word ('\w') character
        ///     classes, as well as their negations ('\D', '\S', and '\W'). This option is
        ///     always enabled and can only be turned off in POSIX mode.
        /// </summary>
        PerlClasses = 1 << 12,

        /// <summary>
        ///     Enables Perl's word boundary operator ('\b'), as well as its negation ('\B').
        ///     This option is always enabled and can only be turned off in POSIX mode.
        /// </summary>
        WordBoundary = 1 << 13,
        
        /// <summary>
        ///     Specifies one-line mode: '^' and '$' only match the start and end of the input.
        ///     This option is always enabled and can only be turned off in POSIX mode.
        /// </summary>
        OneLine = 1 << 14
    };

    [ExtensionAttribute]
    public ref class RegexOption abstract sealed
    {
        public:

            /// <summary>
            ///     Determines whether a <c>RegexOptions</c> instance contains all of the specified flags.
            /// </summary>
            /// <param name="flags">A <c>RegexOptions</c> instance in which to locate additional <c>RegexOptions</c> flags.</param>
            /// <param name="options">The <c>RegexOptions</c> flags to locate in the <c>RegexOptions</c> instance.</param>
            /// <returns><c>true</c> if the bit field or bit fields that are set in <paramref name="options"/> are also set in the current instance; otherwise, <c>false</c></returns>
            [ExtensionAttribute]
            static bool HasAllFlags(RegexOptions flags, RegexOptions options)
            {
                if(flags == options)
                    return true;

                /*
                 *  If flags and options aren't equal (see above) and either has a value of zero,
                 *  the equality test (see below) would falsely succeed.
                 */
                if(flags == RegexOptions::None || options == RegexOptions::None)
                    return false;

                if((flags & options) == options)
                    return true;

                return false;
            }

            /// <summary>
            ///     Determines whether a <c>RegexOptions</c> instance contains one or more of the specified flags.
            /// </summary>
            /// <param name="flags">A <c>RegexOptions</c> instance in which to locate additional <c>RegexOptions</c> flags.</param>
            /// <param name="options">The <c>RegexOptions</c> flags to locate in the <c>RegexOptions</c> instance.</param>
            /// <returns><c>true</c> if one or more of the bit fields that are set in <paramref name="options"/> are also set in the current instance; otherwise, <c>false</c></returns>
            [ExtensionAttribute]
            static bool HasAnyFlag(RegexOptions flags, RegexOptions options)
            {
                return (flags & options) > RegexOptions::None || flags == options;
            }
    };
}
}