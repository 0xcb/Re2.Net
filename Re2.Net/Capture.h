/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#include "RegexInput.h"


namespace Re2
{
namespace Net
{
    using namespace System;
    using namespace System::Text;


    /// <summary>
    ///     Represents the results from a single successful subexpression capture.
    /// </summary>
    /*[Serializable]*/
    public ref class Capture
    {
        private:

            static initonly Encoding^ _utf8Encoding   = (Encoding^)Encoding::UTF8->Clone();
            static initonly Encoding^ _latin1Encoding = (Encoding^)Encoding::GetEncoding("ISO-8859-1")->Clone();

            static Capture()
            {
                /*
                 *  All byte sequences are valid Latin-1. In fact, strictly speaking, there's no need
                 *  to even clone the Encoding here.
                 *
                 *  UTF-8, on the other hand, can fail to parse perfectly valid byte inputs. When it
                 *  does, a replacement "?" is preferable to throwing an exception
                 */
                _utf8Encoding->DecoderFallback = DecoderFallback::ReplacementFallback;
            }


        internal:

            int         _index;
            int         _length;
            RegexInput^ _input;

            Capture(RegexInput^ input, int i, int l);

            property RegexInput^ Input { RegexInput^ get(); }


        public:

            /// <summary>
            ///     Gets the position in the input where the first character of the captured subexpression is found.
            /// </summary>
            /// <value>
            ///     The zero-based position in the input where the first character of the captured subexpression is found.
            /// </value>
            property int Index { int get(); }
            
            /// <summary>
            ///     Gets the length of the captured subexpression.
            /// </summary>
            /// <value>
            ///     The length of the captured subexpression.
            /// </value>
            /// <remarks>
            ///     <c>Length</c> is given in characters, if the input is a string, or bytes, if the input is a byte array.
            /// </remarks>
            property int Length { int get(); }

            /// <summary>
            ///     Gets the captured subexpression as a string.
            /// </summary>
            /// <value>
            ///     A string representation of the subexpression captured by the <c>Match</c>.
            /// </value>
            /// <remarks>
            ///     If the input is a byte array, <c>Value</c> represents a conversion of the matched bytes to a Unicode
            ///     string. Where conversion is not possible, the QUESTION MARK character ("?", U+003F) is substituted.
            /// </remarks>
            property String^ Value { String^ get(); }

            /// <summary>
            ///     Retrieves the captured subexpression from the input as a string by calling the Value property.
            /// </summary>
            /// <value>
            ///     A string representation of the subexpression captured by the <c>Match</c>.
            /// </value>
            /// <remarks>
            ///     <c>ToString</c> is actually an internal call to the <see cref="Value"/> property.
            /// </remarks>
            virtual String^ ToString() override;
    };
}
}