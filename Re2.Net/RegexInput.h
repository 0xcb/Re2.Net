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
    using namespace System::Runtime::InteropServices;


    /*
     *  Each Match object includes references to the source Regex and the input
     *  data, and can be used to generate additional Matches by repeatedly
     *  applying the Regex to the input using Match::NextMatch(). In unmanaged
     *  code this leads to issues with object lifetime and ownership, which
     *  RegexInput solves by wrapping the unmanaged char* in a managed class.
     *  shared_ptr<> would also work for char* input, but not for array<Byte>^
     *  input, which Re2.Net also supports. And the semantics of a managed class
     *  will be more familiar to .NET programmers anyway.
     */
    private ref class RegexInput
    {
        private:
            
            initonly String^      _input;
            initonly array<Byte>^ _bytes;
            initonly const char*  _data;
            initonly int          _length;
            initonly GCHandle^    _handle;
            initonly bool         _isUtf8;


        internal:

            /*
             *  If the RegexInput is created from an unmanaged char array, it assumes
             *  ownership of the array and deletes it in the dtor.
             *
             *  If the RegexInput is created from a managed Byte array, the array is
             *  pinned in memory via GCHandle, which the dtor frees.
             *
             *  NB: The point of accepting Byte arrays is that no additional processing
             *      (e.g. UTF-16 -> UTF-8 conversion) or duplication of data is necessary.
             *      It seems natural that Strings, which are immutable by their nature,
             *      are copied, while Byte arrays, which can be edited in-place, are not.
             *      Still I'm open to the idea of modifying the Regex::Match() methods to
             *      create copies of Byte arrays, which obviates the need for GCHandle.
             */

            static RegexInput^ Empty = gcnew RegexInput(String::Empty, nullptr, 0, false);

            RegexInput(String^ input, const char* data, int length, bool isUtf8)
                : _input(input),
                  _data(data),
                  _length(length),
                  _isUtf8(isUtf8),
                  _bytes(nullptr),
                  _handle(nullptr)
            {
            }
                    
            RegexInput(array<Byte>^ bytes, bool isUtf8)
            {
                _bytes  = bytes;
                _handle = GCHandle::Alloc(bytes, GCHandleType::Pinned);
                _length = bytes->Length;
                _data   = (const char*)_handle->AddrOfPinnedObject().ToPointer();
                _isUtf8 = isUtf8;
                _input  = String::Empty;
            }
            
            property String^ Input
            {
                String^ get() { return _input; }
            }

            property array<Byte>^ Bytes
            {
                array<Byte>^ get() { return _bytes; }
            }

            property const char* Data
            {
                const char* get() { return _data; }
            }

            property int Length
            {
                int get() { return _length; }
            }

            property bool IsUTF8
            {
                bool get() { return _isUtf8; }
            }

            ~RegexInput()
            {
                this->!RegexInput();
            }
        
    
        protected:
            
            !RegexInput()
            {
                if(_handle)
                    _handle->Free();
                else
                    delete[] _data;
            }
    };
}
}