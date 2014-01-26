/*
 *  Re2.Net Copyright © 2014 Colt Blackmore. All Rights Reserved.
 *
 *  See Regex.h for licensing and contact information.
 */

#pragma once

#pragma managed(push, off)
    #include "re2\src\re2.h"
    #include "re2\src\stringpiece.h"
#pragma managed(pop)

#include "Regex.h"
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
    using System::Globalization::StringInfo;
    using System::Text::Encoding;
    using System::Text::StringBuilder;

    using re2::RE2;
    using re2::StringPiece;
    using std::string;
    using std::map;

    typedef Match _Match;


    #pragma region Encoding conversion functions
    
        #pragma region Encoding functions: Do not call directly

        #pragma managed(push, off)
    
            /* Optimization of conversion logic to follow. */
            static StringPiece* stringToUTF8(int* codepoints, int length, int bufsize)
            {
                char* encoded = new char[bufsize];

                /* Inspect generated assembly and ensure all div/mod pairs use one instruction. */
                for(int i = 0, j = 0; i < length; ++i, ++j)
                {
                    if(codepoints[i] < 0x0080)
                        encoded[j] = static_cast<char>(codepoints[i]);
                    else if(codepoints[i] < 0x0800)
                    {
                        encoded[j]   = static_cast<char>(0xc0 + codepoints[i] / 0x40);
                        encoded[++j] = static_cast<char>(0x80 + codepoints[i] % 0x40);
                    }
                    else if(codepoints[i] < 0x10000)
                    {
                        encoded[j]   = static_cast<char>(0xe0 + codepoints[i] / 0x1000);
                        encoded[++j] = static_cast<char>(0x80 + (codepoints[i] % 0x1000) / 0x40);
                        encoded[++j] = static_cast<char>(0x80 + codepoints[i] % 0x40);
                    }
                    else
                    {
                        encoded[j]   = static_cast<char>(0xf0 + codepoints[i] / 0x40000);
                        encoded[++j] = static_cast<char>(0x80 + (codepoints[i] % 0x40000) / 0x1000);
                        encoded[++j] = static_cast<char>(0x80 + (codepoints[i] % 0x1000) / 0x40);
                        encoded[++j] = static_cast<char>(0x80 + codepoints[i] % 0x40);
                    }
                }

                delete[] codepoints;

                return new StringPiece(encoded, bufsize);
            }

        #pragma managed(pop)


        static StringPiece* StringToUTF8(String^ string)
        {
            int  strlength  = string->Length;
            int* codepoints = new int[strlength];
            int  bufsize    = 0;

            int c = 0;
            for(int j = 0; j < strlength; ++c, ++j)
            {
                int unit = string[j];
                if(unit < 0xd800 || unit > 0xdfff)
                    codepoints[c] = unit;
                else
                    codepoints[c] = (unit - 0xd800) * 0x400 + (string[++j] - 0xdc00) + 0x10000;

                if(codepoints[c] < 0x0080)
                    bufsize++;
                else if(codepoints[c] < 0x0800)
                    bufsize += 2;
                else if(codepoints[c] < 0x10000)
                    bufsize += 3;
                else
                    bufsize += 4;
            }
            /* String.Length is wrong if there are surrogate pairs. 'c' is the real length. */
            return stringToUTF8(codepoints, c, bufsize);
        }


        static StringPiece* StringToASCII(String^ string, String^ argument)
        {
            array<Byte>^ bytes = Encoding::ASCII->GetBytes(string);
            if(string != Encoding::ASCII->GetString(bytes))
                throw gcnew ArgumentOutOfRangeException(argument, "Specified argument was out of the range of valid ASCII values.");
            pin_ptr<Byte> pintpr = &bytes[0];
            char* copy = new char[bytes->Length];
            memcpy(copy, pintpr, bytes->Length);
            return new StringPiece(copy, bytes->Length);
        }


        /*
         *  The Latin-1 code page is available on systems with the full .NET Framework or the
         *  .NET Framework Client Profile, but it may not be available on mobile or embedded
         *  devices using the .NET Compact Framework.
         *
         *  See: http://msdn.microsoft.com/en-us/library/x5b31f9d.aspx
         */
        static StringPiece* StringToLatin1(String^ string, String^ argument)
        {
            Encoding^    Latin1 = Encoding::GetEncoding("ISO-8859-1");
            array<Byte>^ bytes  = Latin1->GetBytes(string);
            if(string != Latin1->GetString(bytes))
                throw gcnew ArgumentOutOfRangeException(argument, "Specified argument was out of the range of valid Latin-1 values.");
            pin_ptr<Byte> pintpr = &bytes[0];
            char* copy = new char[bytes->Length];
            memcpy(copy, pintpr, bytes->Length);
            return new StringPiece(copy, bytes->Length);
        }


        static String^ CharToString(std::string str, bool isUtf8)
        {
            array<Byte>^ bytes = gcnew array<Byte>(static_cast<int>(str.size()));
            Marshal::Copy((IntPtr)(char*)str.data(), bytes, 0, static_cast<int>(str.size()));
            return isUtf8 ? Encoding::UTF8->GetString(bytes)
                          : Encoding::GetEncoding("ISO-8859-1")->GetString(bytes);
        }

        #pragma endregion


        /* Call this function rather than the individual encoding functions. */
        static StringPiece* ConvertStringEncoding(String^ string, String^ source, RegexOptions options)
        {
            /* I'd love to hear a good argument for why regex supports empty patterns and inputs. */
            if(!string->Length)
                return new StringPiece(new char[0], 0);

            /* Latin1 overrides ASCII if both are set. */
            return RegexOption::HasAnyFlag(options, RegexOptions::Latin1) ? StringToLatin1(string, source) :
                   RegexOption::HasAnyFlag(options, RegexOptions::ASCII)  ? StringToASCII(string, source)  :
                                                                            StringToUTF8(string);
        }

    #pragma endregion


    #pragma region Encoding length functions

        /*
         *  Counts the number of chars in a UTF-8 sequence. This is necessary because the
         *  Index of a Capture, Group, or Match is reported in terms of the entire input,
         *  regardless of startIndex or length.
         */
        static int StrToCharPos(const char* input, int length)
        {
            length++;
            int rv = 0;
            for(int i = 0; i < length; rv++)
                /* Bits 0xxxxxxx and 11xxxxxx mark the start of a UTF-8 sequence. */
                if((input[rv] & -64) != -128)
                    i++;
            return rv - 1; /* Zero-based */
        }
        

        /*
         *  Counts the number of UTF-8 characters in a char sequence. This is necessary
         *  because the Index of a Capture, Group, or Match is reported in terms of the
         *  entire input, regardless of startIndex or length.
         */
        static int CharToStrPos(const char* input, int length)
        {
            length++;
            int rv = 0;
            for(int i = 0; i < length; i++)
                /* Bits 0xxxxxxx and 11xxxxxx mark the start of a UTF-8 sequence. */
                if((input[i] & -64) != -128)
                    rv++;
            return rv - 1; /* Zero-based */
        }

    #pragma endregion


    #pragma region Regex cache

        void Regex::Cache::Size::set(int value)
        {
            /* Remove older expressions when shrinking the cache size. */
            if(value < _size)
                while(_list.Count > value)
                {
                    Regex^ temp = _list[0];
                    _list.RemoveAt(0);
                    /* Using the full 32-bit enum value keeps Pattern + Options collision-proof. */
                    _map.Remove(temp->Pattern + static_cast<int>(temp->Options).ToString("X8"));
                }

            _size = value;
        }


        Regex^ Regex::Cache::FindOrCreate(String^ pattern, RegexOptions options)
        {
            Regex^  regex = nullptr;
            String^ key   = pattern + static_cast<int>(options).ToString("X8");

            if(_map.ContainsKey(key))
            {
                regex = _map[key];
                int i = _list.LastIndexOf(regex);
                if(i != _list.Count - 1)
                {
                    _list.RemoveAt(i);
                    _list.Add(regex);
                }
            }
            else
            {
                if(_size > 0 && _list.Count >= _size)
                {
                    Regex^ temp = _list[0];
                    _list.RemoveAt(0);
                    _map.Remove(temp->Pattern + static_cast<int>(temp->Options).ToString("X8"));
                }

                regex = gcnew Regex(pattern, options);
                if(_size > 0)
                {
                    _list.Add(regex);
                    _map[key] = regex;
                }
            }

            return regex;
        }


        int Regex::CacheSize::get()
        {
            return Cache::_size;
        }


        void Regex::CacheSize::set(int value)
        {
            if(value < 0)
                throw gcnew ArgumentOutOfRangeException("value");
            Cache::Size = value;
        }

    #pragma endregion


    #pragma region Regex properties and methods

        String^ Regex::Pattern::get()
        {
            return Regex::_pattern;
        }


        RegexOptions Regex::Options::get()
        {
            return Regex::_options;
        }


        int Regex::MaxMemory::get()
        {
            return Regex::_maxMemory;
        }


        String^ Regex::ToString()
        {
            return this->Pattern;
        }


        int Regex::GroupNumberFromName(String^ name)
        {
            if(!name)
                throw gcnew ArgumentNullException("name");

            map<string, int> map = _re2->NamedCapturingGroups();
            StringPiece*     sp  = ConvertStringEncoding(name, "name", this->Options);

            int    rv  = -1;
            string str = sp->ToString(); 

            if(map.count(str))
                rv = map[str];

            delete[] sp->data();
            delete   sp;

            return rv;
        }

    #pragma endregion


    #pragma region Regex matching methods

        #pragma region IsMatch

        bool Regex::IsMatch(String^ input, int startIndex)
        {
            if(!input)
                throw gcnew ArgumentNullException("input", "Value cannot be null.");
            if(startIndex < 0 || startIndex > (gcnew StringInfo(input))->LengthInTextElements)
                throw gcnew ArgumentOutOfRangeException("startIndex", "Start index cannot be less than 0 or greater than input length.");

            StringPiece* sp = ConvertStringEncoding(input, "input", this->Options);
            bool         rv = _re2->Match(*sp, startIndex, sp->length(), RE2::UNANCHORED, NULL, 0);
            
            delete[] sp->data();
            delete   sp;
            
            return rv;
        }


        bool Regex::IsMatch(array<Byte>^ input, int startIndex)
        {
            if(!input)
                throw gcnew ArgumentNullException("input", "Value cannot be null.");
            if(startIndex < 0 || startIndex > input->Length)
                throw gcnew ArgumentOutOfRangeException("startIndex", "Start index cannot be less than 0 or greater than input length.");

            pin_ptr<unsigned char> bytes = &input[0];
            StringPiece* sp = new StringPiece((const char*)bytes, input->Length);
            bool         rv = _re2->Match(*sp, startIndex, sp->length(), RE2::UNANCHORED, NULL, 0);
            
            delete sp;
            
            return rv;
        }


        bool Regex::IsMatch(String^ input)
        {
            return this->IsMatch(input, 0);
        }


        bool Regex::IsMatch(array<Byte>^ input)
        {
            return this->IsMatch(input, 0);
        }


        bool Regex::IsMatch(String^ input, String^ pattern, RegexOptions options)
        {
            return Cache::FindOrCreate(pattern, options)->IsMatch(input);
        }


        bool Regex::IsMatch(array<Byte>^ input, String^ pattern, RegexOptions options)
        {
            return Cache::FindOrCreate(pattern, options)->IsMatch(input);
        }


        bool Regex::IsMatch(String^ input, String^ pattern)
        {
            return Cache::FindOrCreate(pattern, RegexOptions::None)->IsMatch(input);
        }


        bool Regex::IsMatch(array<Byte>^ input, String^ pattern)
        {
            return Cache::FindOrCreate(pattern, RegexOptions::None)->IsMatch(input);
        }

        #pragma endregion


        #pragma region Match

        _Match^ Regex::_match(RegexInput^ input, int startIndex, int length, int stringStartIndex)
        {
            /*
             *  stringStartIndex tracks inputIndex for String inputs between matches to avoid recalculating
             *  in CharToStrPos(), which is prohibitively costly for large inputs.
             */

            int          groupCount = RegexOption::HasAnyFlag(this->Options, RegexOptions::SingleCapture) ? 1 : 1 + _re2->NumberOfCapturingGroups();
            StringPiece* captures   = new StringPiece[groupCount]();
            StringPiece  haystack(input->Data, input->Length);

            _Match^ rv = _Match::Empty;
            if(_re2->Match(haystack, startIndex, startIndex + length, RE2::UNANCHORED, captures, groupCount))
            {
                /* Ignore the encoding of input byte arrays. */
                bool isUtf8     = input->Bytes ? false : input->IsUTF8;
                int  charOffset = static_cast<int>(captures[0].data() - haystack.data());
                int  inputIndex = isUtf8 && charOffset ? CharToStrPos(haystack.data() + startIndex, charOffset - startIndex) + stringStartIndex : charOffset;
                int  capLength  = isUtf8 ? CharToStrPos(captures[0].data(), captures[0].length()) : captures[0].length();

                rv          = gcnew _Match(this, groupCount, input, inputIndex, capLength, charOffset + captures[0].length());
                rv->_index  = inputIndex;
                rv->_length = capLength;

                GroupCollection^ groups = rv->Groups;
                for(int i = 1; i < groupCount; i++)
                {
                    if(NULL == captures[i])
                        groups[i] = Group::Empty;
                    else
                    {
                        /*
                         *  Match tracks the char offset and String index separately in case of UTF-8 String input, but
                         *  they will be the same if the input is a Byte array, or if the Regex is ASCII or Latin-1.
                         */
                        charOffset = static_cast<int>(captures[i].data() - haystack.data());
                        inputIndex = isUtf8 && charOffset ? CharToStrPos(haystack.data() + startIndex, charOffset - startIndex) + stringStartIndex : charOffset;
                        capLength  = isUtf8 ? CharToStrPos(captures[i].data(), captures[i].length()) : captures[i].length();

                        groups[i] = gcnew Group(input, inputIndex, capLength);
                    }
                }
            }

            delete[] captures;

            return rv;
        }


        _Match^ Regex::Match(String^ input, int startIndex, int length)
        {
            int inputlength = (gcnew StringInfo(input))->LengthInTextElements;
            if(!input)
                throw gcnew ArgumentNullException("input", "Value cannot be null.");
            if(startIndex < 0 || startIndex > inputlength)
                throw gcnew ArgumentOutOfRangeException("startIndex", "Start index cannot be less than 0 or greater than input length.");
            if(length < 0 || length > inputlength)
                throw gcnew ArgumentOutOfRangeException("length", "Length cannot be less than 0 or greater than input length.");
            if(startIndex + length - 1 > inputlength)
                throw gcnew ArgumentOutOfRangeException("startIndex, length", "Start index and length combined cannot be greater than input length.");

            /* If in UTF-8 mode, convert the start and length values from String^ to char* offset. */
            bool isUtf8 = !RegexOption::HasAnyFlag(this->Options, SINGLE_BYTE_ENCODING);

            StringPiece* sp = ConvertStringEncoding(input, "input", this->Options);
            RegexInput^  ri = gcnew RegexInput(input, sp->data(), sp->length(), isUtf8);

            if(isUtf8)
            {
                if(startIndex) startIndex = StrToCharPos(sp->data(), startIndex);
                if(length)     length     = StrToCharPos(sp->data() + startIndex, length);
            }

            return this->_match(ri, startIndex, length, 0);
        }


        _Match^ Regex::Match(array<Byte>^ input, int startIndex, int length)
        {
            if(!input)
                throw gcnew ArgumentNullException("input", "Value cannot be null.");
            if(startIndex < 0 || startIndex > input->Length)
                throw gcnew ArgumentOutOfRangeException("startIndex", "Start index cannot be less than 0 or greater than input length.");
            if(length < 0 || length > input->Length)
                throw gcnew ArgumentOutOfRangeException("length", "Length cannot be less than 0 or greater than input length.");
            if(startIndex + length - 1 > input->Length)
                throw gcnew ArgumentOutOfRangeException("startIndex, length", "Start index and length combined cannot be greater than input length.");

            RegexInput^ ri = gcnew RegexInput(input, !RegexOption::HasAnyFlag(this->Options, SINGLE_BYTE_ENCODING));

            /* Unicode hijinks aren't required for byte arrays. */
            return this->_match(ri, startIndex, length, 0);
        }


        _Match^ Regex::Match(String^ input, int startIndex)
        {
            return this->Match(input, startIndex, (gcnew StringInfo(input))->LengthInTextElements - startIndex);
        }


        _Match^ Regex::Match(array<Byte>^ input, int startIndex)
        {
            return this->Match(input, startIndex, input->Length - startIndex);
        }


        _Match^ Regex::Match(String^ input)
        {
            return this->Match(input, 0, (gcnew StringInfo(input))->LengthInTextElements);
        }


        _Match^ Regex::Match(array<Byte>^ input)
        {
            return this->Match(input, 0, input->Length);
        }


        _Match^ Regex::Match(String^ input, String^ pattern, RegexOptions options)
        {
            return Cache::FindOrCreate(pattern, options)->Match(input);
        }


        _Match^ Regex::Match(array<Byte>^ input, String^ pattern, RegexOptions options)
        {
            return Cache::FindOrCreate(pattern, options)->Match(input);
        }


        _Match^ Regex::Match(String^ input, String^ pattern)
        {
            return Cache::FindOrCreate(pattern, RegexOptions::None)->Match(input);
        }


        _Match^ Regex::Match(array<Byte>^ input, String^ pattern)
        {
            return Cache::FindOrCreate(pattern, RegexOptions::None)->Match(input);
        }

        #pragma endregion


        #pragma region Matches

        MatchCollection^ Regex::Matches(String^ input, int startIndex)
        {
            return gcnew MatchCollection(this->Match(input, startIndex, (gcnew StringInfo(input))->LengthInTextElements - startIndex));
        }


        MatchCollection^ Regex::Matches(array<Byte>^ input, int startIndex)
        {
            return gcnew MatchCollection(this->Match(input, startIndex, input->Length - startIndex));
        }


        MatchCollection^ Regex::Matches(String^ input)
        {
            return gcnew MatchCollection(this->Match(input, 0, (gcnew StringInfo(input))->LengthInTextElements));
        }


        MatchCollection^ Regex::Matches(array<Byte>^ input)
        {
            return gcnew MatchCollection(this->Match(input, 0, input->Length));
        }


        MatchCollection^ Regex::Matches(String^ input, String^ pattern, RegexOptions options)
        {
            return gcnew MatchCollection(Cache::FindOrCreate(pattern, options)->Match(input, 0, (gcnew StringInfo(input))->LengthInTextElements));
        }


        MatchCollection^ Regex::Matches(array<Byte>^ input, String^ pattern, RegexOptions options)
        {
            return gcnew MatchCollection(Cache::FindOrCreate(pattern, options)->Match(input, 0, input->Length));
        }


        MatchCollection^ Regex::Matches(String^ input, String^ pattern)
        {
            return gcnew MatchCollection(Cache::FindOrCreate(pattern, RegexOptions::None)->Match(input, 0, (gcnew StringInfo(input))->LengthInTextElements));
        }


        MatchCollection^ Regex::Matches(array<Byte>^ input, String^ pattern)
        {
            return gcnew MatchCollection(Cache::FindOrCreate(pattern, RegexOptions::None)->Match(input, 0, input->Length));
        }

        #pragma endregion

    #pragma endregion


    #pragma region Regex constructors and cleanup

        Regex::Regex(String^ pattern, RegexOptions options, int maxMemory)
            : _re2(nullptr), _pattern(pattern), _options(options), _maxMemory(maxMemory)
        {
            if(!pattern)
                throw gcnew ArgumentNullException("pattern", "Value cannot be null.");
            if(options < RegexOptions::None || options > REGEX_OPTIONS_MAX)
                throw gcnew ArgumentOutOfRangeException("options", "Specified argument was outside the range of valid RegexOptions values.");
            /*
             * // maxMemory is not validated because RE2 permits maxMemory values <= 0. (See
             * // re2::Compiler::Setup() in compile.cc.) Uncomment to disallow.
             * if(maxMemory <= 0)
             *     throw gcnew ArgumentOutOfRangeException("maxMemory", "Specified argument was out of the range of valid memory values.");
             */

            // The RE2 ctor caches RE2::Options as bitwise flags. RAII can have the instance.
            RE2::Options settings;
            settings.set_max_mem(maxMemory);
            settings.set_log_errors(false);

            if(!RegexOption::HasAnyFlag(options, RegexOptions::None))
            {
                /*
                 *  re2.h labels set_utf8() a "Legacy interface" that could be removed, although
                 *  at this point that seems unlikely. If it ever happens, switch to set_encoding().
                 */
                settings.set_utf8          (!RegexOption::HasAnyFlag(options, SINGLE_BYTE_ENCODING));
                settings.set_case_sensitive(!RegexOption::HasAnyFlag(options, RegexOptions::IgnoreCase));
                settings.set_never_nl      ( RegexOption::HasAnyFlag(options, RegexOptions::IgnoreNewline));
                settings.set_longest_match ( RegexOption::HasAnyFlag(options, RegexOptions::LongestMatch));
                settings.set_literal       ( RegexOption::HasAnyFlag(options, RegexOptions::Literal));
                settings.set_posix_syntax  ( RegexOption::HasAnyFlag(options, RegexOptions::POSIX));
                settings.set_perl_classes  ( RegexOption::HasAnyFlag(options, RegexOptions::PerlClasses));
                settings.set_word_boundary ( RegexOption::HasAnyFlag(options, RegexOptions::WordBoundary));
                settings.set_one_line      ( RegexOption::HasAnyFlag(options, RegexOptions::OneLine));

                /*
                 *  never_capture is a recent addition to RE2 and will be added to Re2.Net if it allows
                 *  support for .NET's RegexOptions.ExplicitCapture flag.
                 */
                //settings.set_never_capture ( RegexOption::HasAnyFlag(options, RegexOptions::ExplicitCapture));

                /*
                 *  RE2 only accepts some options inline, so they're inserted at the front of
                 *  the pattern. This is done with the Multiline and Singleline options for backwards
                 *  compatibility with .NET's Regex class; for the Ungreedy option, it's done because the
                 *  other two are already being inserted, so hey, what's one more?
                 *
                 *  NB: The insertion doesn't bother to check whether a particular flag is already present.
                 *      Regex parsers are unanimous in permitting repeated flags, e.g. "(?mmmm)" and even
                 *      "(?m)(?m)(?m)(?m)".
                 *
                 *      Note also that flags set with RegexOptions are guaranteed to be correct and cannot
                 *      cause a parsing error. They are therefore omitted from error messages, so as not to
                 *      confuse users.
                 */
                StringBuilder^ flags = gcnew StringBuilder();
                if(RegexOption::HasAnyFlag(options, RegexOptions::Multiline))
                    flags->Append("m");
                if(RegexOption::HasAnyFlag(options, RegexOptions::Singleline))
                    flags->Append("s");
                if(RegexOption::HasAnyFlag(options, RegexOptions::Ungreedy))
                    flags->Append("U");

                if(flags->Length > 0)
                {
                    flags->Insert(0, "(?");
                    flags->Append(")");
                    pattern = pattern->Insert(0, flags->ToString());
                }
            }
                
            /*
             *  The RE2 ctor creates a local copy of the pattern, thus there is no reason to preserve it.
             *  A StringPiece doesn't take ownership of its contents, however, so the underlying data has
             *  to be freed manually.
             */
            StringPiece* regex = ConvertStringEncoding(pattern, "pattern", options);
            _re2 = new RE2(*regex, settings);
            delete[] regex->data();
            delete   regex;

            if(!_re2->ok())
                throw gcnew ArgumentException(String::Format("{0}: '{1}' in pattern '{2}'.",
                                                             _errorTable[_re2->error_code()],
                                                             CharToString(_re2->error_arg(), settings.utf8()),
                                                             Pattern));
        }


        Regex::Regex(String^ pattern, RegexOptions options)
        {
            this->Regex::Regex(pattern, options, /* #defined in re2.h */ kDefaultMaxMem);
        }


        Regex::Regex(String^ pattern)
        {
            this->Regex::Regex(pattern, RegexOptions::None, /* #defined in re2.h */ kDefaultMaxMem);
        }


        /* Initialize error code message lookup table. */
        static Regex::Regex()
        {
            _errorTable[RE2::ErrorInternal]          = "An unknown internal error has occurred";
            _errorTable[RE2::ErrorBadEscape]         = "Invalid escape sequence";
            _errorTable[RE2::ErrorBadCharClass]      = "Invalid character class";
            _errorTable[RE2::ErrorBadCharRange]      = "Invalid character class range";
            _errorTable[RE2::ErrorMissingBracket]    = "Missing bracket";
            _errorTable[RE2::ErrorMissingParen]      = "Missing parenthesis";
            _errorTable[RE2::ErrorTrailingBackslash] = "Missing escape sequence (trailing backslash)";
            _errorTable[RE2::ErrorRepeatArgument]    = "Invalid repetition operator (nothing to repeat)";
            _errorTable[RE2::ErrorRepeatSize]        = "Invalid repetition argument";
            _errorTable[RE2::ErrorRepeatOp]          = "Invalid repetition operator (repetition operators cannot be combined)";
            _errorTable[RE2::ErrorBadPerlOp]         = "Invalid Perl operator";
            _errorTable[RE2::ErrorBadUTF8]           = "Invalid UTF-8 sequence";
            _errorTable[RE2::ErrorBadNamedCapture]   = "Invalid named capture group";
            _errorTable[RE2::ErrorPatternTooLarge]   = "Pattern too large";
        }


        Regex::~Regex()
        {
            this->!Regex();
        }


        Regex::!Regex()
        {
            if(_re2)
                delete _re2;
        }

    #pragma endregion
}
}