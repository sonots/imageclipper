/**
* The MIT License
* 
* Copyright (c) 2008, Naotoshi Seo <sonots(at)umd.edu>
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
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <vector>
using namespace std;
namespace fs = boost::filesystem;

#ifndef GET_FILELIST_INCLUDED
#define GET_FILELIST_INCLUDED
/**
* List Files in a directory
*
* @param dirpath Path to the target directory
* @param [regex = ".*"] Regular expression (instead of wild cards)
* @param [file_type = type_unknown] List only specified file_type. The default is for all
* @return vector<boost::filesystem::path> Vector of file list
* @requirements boost::filesystem, boost::regex, std::vector, std::string 
*
* Memo: boost::filesystem::path path.native_file_string().c_str()
*/
vector<fs::path> 
get_filelist(const fs::path& dirpath, 
             const boost::regex regex = boost::regex(".*"), 
             fs::file_type file_type = fs::type_unknown)
{
    vector<fs::path> filelist;
    bool list_directory    = (file_type == fs::directory_file);
    bool list_regular_file = (file_type == fs::regular_file);
    bool list_symlink      = (file_type == fs::symlink_file);
    bool list_other        = (!list_directory && !list_regular_file && !list_symlink);
    bool list_all          = (file_type == fs::type_unknown); // just for now

    if(!fs::exists(dirpath) || !fs::is_directory(dirpath)) {
        return filelist;
    }

    fs::directory_iterator iter(dirpath), end_iter;
    for(; iter != end_iter; ++iter) {
        fs::path filename = iter->path();
        if(boost::regex_match(filename.native_file_string(), regex)) {
            if(list_all) {
                filelist.push_back(filename);
            } else if(list_regular_file && fs::is_regular(filename)) {
                filelist.push_back(filename);                
            } else if(list_directory && fs::is_directory(filename)) {
                filelist.push_back(filename);
            } else if(list_symlink && fs::is_symlink(filename)) {
                filelist.push_back(filename);
            } else if(list_other && fs::is_other(filename)) {
                filelist.push_back(filename);
            }
        }
    }
    return filelist;
}

#endif
