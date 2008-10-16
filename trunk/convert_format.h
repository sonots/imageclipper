/**
* The MIT License
* 
* Copyright (c) 2008, Naotoshi Seo <sonots(at)sonots.com>
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#include <stdio.h>
#include <string>

#ifndef CONVERT_FORMAT_INCLUDED
#define CONVERT_FORMAT_INCLUDED

/**
* Convert format
*
* %i => filename
* %e => extension
* %x => x
* %y => y
* %w => width
* %h => height
* %r => rotation (int degree)
* %s => shear deformation degree (int degree)
* %f => frame number (for video file)
*
* @param format The format string
* @return string
*/
string convert_format(const string& format, const string& dirname, const string& filename, const string& extension, 
                      int x, int y, int width, int height, int frame = 0, int rotation = 0, int shear = 0)
{
    string ret = format;
    char tmp[2048];
    char intkeys[] = { 'x', 'y', 'w', 'h', 'f', 'r', 's' };
    int  intvals[] = { x, y, width, height, frame, rotation, shear };
    char strkeys[] = { 'i', 'e', 'd' };
    std::string strvals[] = { filename, extension, dirname };
    int nintkeys = 7;
    int nstrkeys = 3;
    for(int i = 0; i < nintkeys + nstrkeys; i++) {
        std::string::size_type start = ret.find("%");
        if(start == std::string::npos) break;
        std::string::size_type minstrpos = std::string::npos;
        std::string::size_type minintpos = std::string::npos;
        int minstrkey = INT_MAX; int minintkey = INT_MAX;
        for(int j = 0; j < nstrkeys; j++) {
            std::string::size_type pos = ret.find(strkeys[j], start);
            if(pos < minstrpos) {
                minstrpos = pos;
                minstrkey = j;
            }
        }
        for(int j = 0; j < nintkeys; j++) {
            std::string::size_type pos = ret.find(intkeys[j], start);
            if(pos < minintpos) {
                minintpos = pos;
                minintkey = j;
            }
        }
        if(minstrpos == std::string::npos && minintpos == std::string::npos) break;
        if(minstrpos < minintpos) {
            string format_substr = ret.substr(start, minstrpos - start) + "s";
            sprintf(tmp, format_substr.c_str(), strvals[minstrkey].c_str());
            ret.replace(start, minstrpos - start + 1, string(tmp));
        } else {
            string format_substr = ret.substr(start, minintpos - start) + "d";
            sprintf(tmp, format_substr.c_str(), intvals[minintkey]);
            ret.replace(start, minintpos - start + 1, string(tmp));
        }
    }
    return ret;
}

#endif
