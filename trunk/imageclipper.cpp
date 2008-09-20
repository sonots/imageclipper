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
#ifdef _MSC_VER // MS Visual Studio
#pragma warning(disable:4996)
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "cvaux.lib")
#pragma comment(lib, "highgui.lib")
#endif

#include "cv.h"
#include "cvaux.h"
#include "highgui.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <string>
#include <vector>
using namespace std;
namespace fs = boost::filesystem;
// no header file because i'm lazy ;-)

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
vector<fs::path> get_filelist( const fs::path& dirpath, 
                              const boost::regex regex = boost::regex(".*"), 
                              fs::file_type file_type = fs::type_unknown )
{
    vector<fs::path> filelist;
    bool list_directory    = ( file_type == fs::directory_file );
    bool list_regular_file = ( file_type == fs::regular_file );
    bool list_symlink      = ( file_type == fs::symlink_file );
    bool list_other        = ( !list_directory && !list_regular_file && !list_symlink );
    bool list_all          = ( file_type == fs::type_unknown ); // just for now

    if( !fs::exists( dirpath ) || !fs::is_directory( dirpath ) )
    {
        return filelist;
    }

    fs::directory_iterator iter( dirpath ), end_iter;
    for( ; iter != end_iter; ++iter )
    {
        fs::path filename = iter->path();
        if( boost::regex_match( filename.native_file_string(), regex ) )
        {
            if( list_all )
            {
                filelist.push_back( filename );
            }
            else if( list_regular_file && fs::is_regular( filename ) )
            {
                filelist.push_back( filename );                
            }
            else if( list_directory && fs::is_directory( filename ) )
            {
                filelist.push_back( filename );
            }
            else if( list_symlink && fs::is_symlink( filename ) )
            {
                filelist.push_back( filename );
            }
            else if( list_other && fs::is_other( filename ) )
            {
                filelist.push_back( filename );
            }
        }
    }
    return filelist;
}

/**
* Convert format
*
* %i => filename
* %e => extension
* %x => x
* %y => y
* %w => width
* %h => height
* %f => frame number (for video file)
*
* @param format The format string
* @return string
* @todo refine more (use boost::any or use boost::regex)
*/
string convert_format( const string& format, const string& dirname, const string& filename, const string& extension, 
                      int x, int y, int width, int height, int frame = 0 )
{
    string ret = format;
    char tmp[2048];
    char intkeys[] = { 'x', 'y', 'w', 'h', 'f' };
    int  intvals[] = { x, y, width, height, frame };
    char strkeys[] = { 'i', 'e', 'd' };
    std::string strvals[] = { filename, extension, dirname };
    int nintkeys = 5;
    int nstrkeys = 3;
    for( int i = 0; i < nintkeys + nstrkeys; i++ )
    {
        std::string::size_type start = ret.find( "%" );
        if( start == std::string::npos ) break;
        std::string::size_type minstrpos = std::string::npos;
        std::string::size_type minintpos = std::string::npos;
        int minstrkey = INT_MAX; int minintkey = INT_MAX;
        for( int j = 0; j < nstrkeys; j++ )
        {
            std::string::size_type pos = ret.find( strkeys[j], start );
            if( pos < minstrpos )
            {
                minstrpos = pos;
                minstrkey = j;
            }
        }
        for( int j = 0; j < nintkeys; j++ )
        {
            std::string::size_type pos = ret.find( intkeys[j], start );
            if( pos < minintpos )
            {
                minintpos = pos;
                minintkey = j;
            }
        }
        if( minstrpos == std::string::npos && minintpos == std::string::npos ) break;
        if( minstrpos < minintpos )
        {
            string format_substr = ret.substr( start, minstrpos - start ) + "s";
            sprintf( tmp, format_substr.c_str(), strvals[minstrkey].c_str() );
            ret.replace( start, minstrpos - start + 1, string( tmp ) );
        }
        else
        {
            string format_substr = ret.substr( start, minintpos - start ) + "d";
            sprintf( tmp, format_substr.c_str(), intvals[minintkey] );
            ret.replace( start, minintpos - start + 1, string( tmp ) );
        }
    }
    return ret;
}

typedef struct MyMouseStruct {
    const char* w_name;
    IplImage* img;
    CvRect region;
} MyMouseStruct ;

inline MyMouseStruct myMouseStruct( const char* w_name, IplImage* img, CvRect& region )
{
    MyMouseStruct m = { w_name, img, region };
    return m;
}

///// Trivial Inline Functions ////
inline CvRect cvRectByPoint( const CvPoint& pt1, const CvPoint& pt2 )
{
    return cvRect( pt1.x, pt1.y, pt2.x - pt1.x, pt2.y - pt1.y );
}

inline void cvPointByRect( CvPoint& pt1, CvPoint& pt2, const CvRect& rect )
{
    pt1 = cvPoint( rect.x, rect.y );
    pt2 = cvPoint( rect.x + rect.width, rect.y + rect.height );
}
inline void cvRectangleByRect( CvArr* img, CvRect rect, CvScalar color,
                              int thickness=1, int line_type=8, int shift=0 )
{
    CvPoint pt1, pt2; cvPointByRect( pt1, pt2, rect );
    cvRectangle( img, pt1, pt2, color, thickness, line_type, shift );
}

inline void cvShowImageAndRectangle( const char* w_name, const IplImage* img, const CvRect& rect )
{
    IplImage* cloneimg = cvCloneImage( img );
    cvRectangleByRect( cloneimg, rect, CV_RGB(255, 255, 0), 1); 
    cvShowImage( w_name, cloneimg );
    cvReleaseImage( &cloneimg );
}

inline double cvPointNorm( CvPoint p1, CvPoint p2, int norm_type = CV_L2 )
{
    // support only sqrt( sum( (p1 - p2)^2 ) )
    return sqrt( pow( (double)p2.x - p1.x, 2 ) + pow( (double)p2.y - p1.y, 2 ) );
}

inline CvRect cvShowImageAndWatershed( const char* w_name, const IplImage* img, const CvPoint& center, const int radius )
{
    IplImage* cloneimg = cvCloneImage( img );
    IplImage* markers  = cvCreateImage( cvGetSize( cloneimg ), IPL_DEPTH_32S, 1 );

    // Set watershed markers. Now, marker's shape is like circle
    // Set (1 * radius) - (3 * radius) region as ambiguous region (0), intuitively
    cvSet( markers, cvScalarAll( 1 ) );
    cvCircle( markers, center, 3 * radius, cvScalarAll( 0 ), CV_FILLED, 8, 0 );
    cvCircle( markers, center, radius, cvScalarAll( 2 ), CV_FILLED, 8, 0 );
    cvWatershed( cloneimg, markers );

    // Draw watershed markers and rectangle surrounding watershed markers
    cvCircle( cloneimg, center, radius, cvScalarAll (255), 2, 8, 0);

    CvPoint minpoint = cvPoint( markers->width, markers->height );
    CvPoint maxpoint = cvPoint( 0, 0 );
    for (int y = 1; y < markers->height-1; y++) { // looks outer boundary is always -1. 
        for (int x = 1; x < markers->width-1; x++) {
            int* idx = (int *) cvPtr2D (markers, y, x, NULL);
            if (*idx == -1) { // watershed marker -1
                cvSet2D (cloneimg, y, x, cvScalarAll (255));
                if( x < minpoint.x ) minpoint.x = x;
                if( y < minpoint.y ) minpoint.y = y;
                if( x > maxpoint.x ) maxpoint.x = x;
                if( y > maxpoint.y ) maxpoint.y = y;
            }
        }
    }
    cvRectangle( cloneimg, minpoint, maxpoint, CV_RGB(255, 255, 0), 1 );
    cvShowImage( w_name, cloneimg );
    cvReleaseImage( &cloneimg );

    return cvRectByPoint( minpoint, maxpoint );
}

/**
* cvSetMouseCallback function
*/
void on_mouse( int event, int x, int y, int flags, void* arg )
{
    MyMouseStruct* param           = (MyMouseStruct*) arg;
    static CvPoint point0          = cvPoint( 0, 0 );
    static bool move_rect          = false;
    static bool resize_rect_left   = false;
    static bool resize_rect_right  = false;
    static bool resize_rect_top    = false;
    static bool resize_rect_bottom = false;
    static CvRect circle           = cvRect( 0, 0, 0, 0 ); // use as center.x, center,y, radius
    static bool showing_watershed  = false;
    static bool move_watershed     = false;
    static bool resize_watershed   = false;

    if( !param->img )
        return;

    if( x >= 32768 ) x -= 65536; // change left outsite to negative
    if( y >= 32768 ) y -= 65536; // change top outside to negative

    if( event == CV_EVENT_MBUTTONDOWN || ( event == CV_EVENT_LBUTTONDOWN && flags & CV_EVENT_FLAG_SHIFTKEY ) ) // draw watershed
    {
        point0   = cvPoint( x, y );
        circle.x = x;
        circle.y = y;
        showing_watershed = true;
    }
    else if( event == CV_EVENT_LBUTTONDOWN ) // draw rect
    {
        point0 = cvPoint( x, y );
        showing_watershed = false;
    }
    else if( event == CV_EVENT_RBUTTONDOWN ) // move
    {
        point0 = cvPoint( x, y );

        if( showing_watershed )
        {
            CvPoint center = cvPoint( circle.x, circle.y );
            int radius = (int) cvPointNorm( center, point0 );
            if( circle.width - 1 <= radius && radius <= circle.width )
            {
                resize_watershed = true;
            }
            else if( radius <= circle.width )
            {
                move_watershed = true;
            }
        }
        if( !resize_watershed && !move_watershed )
        {
            showing_watershed = false;
            if( ( param->region.x < x && x < param->region.x + param->region.width ) && 
                ( param->region.y < y && y < param->region.y + param->region.height ) )
            {
                move_rect = true;
            }
            if( x <= param->region.x )
            {
                resize_rect_left = true; 
            }
            else if( x >= param->region.x + param->region.width )
            {
                resize_rect_right = true;
            }
            if( y <= param->region.y )
            {
                resize_rect_top = true; 
            }
            else if( y >= param->region.y + param->region.height )
            {
                resize_rect_bottom = true;
            }
        }
    }
    else if( event == CV_EVENT_MOUSEMOVE )
    {
        if( (flags & CV_EVENT_FLAG_MBUTTON) || ( flags & CV_EVENT_FLAG_LBUTTON && flags & CV_EVENT_FLAG_SHIFTKEY ) ) // draw watershed
        {
            CvPoint center = cvPoint( circle.x, circle.y );
            circle.width = (int) cvPointNorm( center, cvPoint( x, y ) );

            param->region = cvShowImageAndWatershed( param->w_name, param->img, center, circle.width );
        }
        else if( flags & CV_EVENT_FLAG_LBUTTON ) // draw rect
        {
            param->region.x = min( point0.x, x );
            param->region.y = min( point0.y, y );
            param->region.width =  abs( point0.x - x );
            param->region.height = abs( point0.y - y );

            cvShowImageAndRectangle( param->w_name, param->img, param->region );
        }
        else if( flags & CV_EVENT_FLAG_RBUTTON ) // move
        {
            if( showing_watershed )
            {
                if( move_watershed )
                {
                    CvPoint move = cvPoint( x - point0.x, y - point0.y );
                    circle.x += move.x;
                    circle.y += move.y;

                    param->region = cvShowImageAndWatershed( param->w_name, param->img, cvPoint( circle.x, circle.y ), circle.width );

                    point0 = cvPoint( x, y );
                }
                else if( resize_watershed )
                {
                    CvPoint center = cvPoint( circle.x, circle.y );
                    circle.width = (int) cvPointNorm( center, cvPoint( x, y ) );

                    param->region = cvShowImageAndWatershed( param->w_name, param->img, center, circle.width );
                }
            }
            else
            {
                if( move_rect )
                {
                    CvPoint move = cvPoint( x - point0.x, y - point0.y );
                    param->region.x += move.x;
                    param->region.y += move.y;
                }
                if( resize_rect_left )
                {
                    int move_x = x - point0.x;
                    param->region.x += move_x;
                    param->region.width -= move_x;
                }
                else if( resize_rect_right )
                {
                    int move_x = x - point0.x;
                    param->region.width += move_x;
                }
                if( resize_rect_top )
                {
                    int move_y = y - point0.y;
                    param->region.y += move_y;
                    param->region.height -= move_y;
                }
                else if( resize_rect_bottom )
                {
                    int move_y = y - point0.y;
                    param->region.height += move_y;
                }

                // assure width is positive
                if( param->region.width <= 0 )
                {
                    param->region.x += param->region.width;
                    param->region.width *= -1;
                    bool tmp = resize_rect_right;
                    resize_rect_right = resize_rect_left;
                    resize_rect_left  = tmp;
                }
                // assure height is positive
                if( param->region.height <= 0 )
                {
                    param->region.y += param->region.height;
                    param->region.height *= -1;
                    bool tmp = resize_rect_top;
                    resize_rect_top    = resize_rect_bottom;
                    resize_rect_bottom = tmp;
                }

                cvShowImageAndRectangle( param->w_name, param->img, param->region );
                point0 = cvPoint( x, y );
            }
        }
    }
    else if( event == CV_EVENT_RBUTTONUP )
    {
        move_rect          = false;
        resize_rect_left   = false;
        resize_rect_right  = false;
        resize_rect_top    = false;
        resize_rect_bottom = false;
        move_watershed     = false;
        resize_watershed   = false;
    }
}


/**
* Print out usage
*/
void usage( const char* com, const fs::path &reference, const char* imgout_format,
           const char* vidout_format )
{
    cerr << "ImageClipper - image clipping helper tool." << endl;
    cerr << "Command Usage: " << fs::path( com ).leaf();
    cerr << " [option]... [reference]" << endl;
    cerr << "  <reference = " << reference << ">" << endl;
    cerr << "    <reference> would be a directory or an image or a video filename." << endl;
    cerr << "    For a directory, image files in the directory will be read sequentially." << endl;
    cerr << "    For an image (a file having supported image type extensions listed below), " << endl;
    cerr << "    it starts to read a directory from the specified image file. " << endl;
    cerr << "    For a video (a file except images), the video is read." << endl;
    cerr << endl;
    cerr << "  Options" << endl;
    cerr << "    -f <output_format = imgout_format or vidout_format>" << endl;
    cerr << "        Determine the output file path format." << endl;
    cerr << "        This is a syntax sugar for -i and -v. " << endl;
    cerr << "        Format Expression)" << endl;
    cerr << "            %d - dirname of the original" << endl;
    cerr << "            %i - filename of the original without extension" << endl;
    cerr << "            %e - filename extension of the original" << endl;
    cerr << "            %x - upper-left x coord" << endl;
    cerr << "            %y - upper-left y coord" << endl;
    cerr << "            %w - width" << endl;
    cerr << "            %h - height" << endl;
    cerr << "            %f - frame number (for video)" << endl;
    cerr << "        Example) ./$i_%04x_%04y_%04w_%04h.%e" << endl;
    cerr << "            Store into software directory and use image type of the original." << endl;
    cerr << "    -i <imgout_format = " << imgout_format << ">" << endl;
    cerr << "        Determine the output file path format for image inputs." << endl;
    cerr << "    -v <vidout_format = " << vidout_format << ">" << endl;
    cerr << "        Determine the output file path format for a video input." << endl;
    cerr << "    -h" << endl;
    cerr << "    --help" << endl;
    cerr << "        Show this help" << endl;
    cerr << "    -s" << endl;
    cerr << "    --show" << endl;
    cerr << "        Show clipped image" << endl;
    cerr << endl;
    cerr << "  Supported Image Types" << endl;
    cerr << "      bmp|dib|jpeg|jpg|jpe|png|pbm|pgm|ppm|sr|ras|tiff|exr|jp2" << endl;
    cerr << endl;
    cerr << "Application Usage:" << endl;
    cerr << "    Select a region by dragging the left mouse button." << endl;
    cerr << "    Move the region by dragging the right mouse button." << endl;
    cerr << "    Use following keys to clip the selected region:" << endl;
    cerr << "    s        : Save the selected region as an image." << endl;
    cerr << "    f        : Forward. Show next image." << endl;
    cerr << "    SPACE    : Save and Forward." << endl;
    cerr << "    b        : Backward. Not available for video file (now)." << endl;
    cerr << "    q or ESC : Quit. " << endl;
}

int main( int argc, char *argv[] )
{
    //// Initialization
    fs::path reference( "." );
    bool show = false;
    const char* imgout_format = "%d/imageclipper/%i.%e_%04x_%04y_%04w_%04h.png";
    const char* vidout_format = "%d/imageclipper/%i.%e_%04f_%04x_%04y_%04w_%04h.png";
    const char* output_format = NULL;
    boost::regex imagetypes( ".*\\.(bmp|dib|jpeg|jpg|jpe|png|pbm|pgm|ppm|sr|ras|tiff|exr|jp2)$", 
        boost::regex_constants::icase );
    const char* w_name = "<S> Save <F> Forward <SPACE> s and f <B> Backward <ESC> Exit";

    //// Arguments
    for( int i = 1; i < argc; i++ )
    {
        if( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "--help" ) )
        {
            usage( argv[0], reference, imgout_format, vidout_format );
            return 0;
        } 
        else if( !strcmp( argv[i], "-f" ) || !strcmp( argv[i], "--output_format" ) )
        {
            output_format = argv[++i];
        }
        else if( !strcmp( argv[i], "-i" ) || !strcmp( argv[i], "--imgout_format" ) )
        {
            imgout_format = argv[++i];
        }
        else if( !strcmp( argv[i], "-v" ) || !strcmp( argv[i], "--vidout_format" ) )
        {
            vidout_format = argv[++i];
        }
        else if( !strcmp( argv[i], "-s" ) || !strcmp( argv[i], "--show" ) )
        {
            show = true;
        }
        else
        {
            reference = fs::path( argv[i] );
        }
    }

    //// Initial argument check
    bool is_directory = fs::is_directory( reference );
    bool is_image = boost::regex_match( reference.native_file_string(), imagetypes );
    bool is_video = !is_directory & !is_image;
    if( output_format == NULL )
    {
        output_format = is_video ? vidout_format : imgout_format;
    }

    vector<fs::path> filelist; // for image
    vector<fs::path>::iterator filename; // for image
    CvCapture* cap; // for video
    IplImage *img;
    if( is_directory || is_image )
    {
        cerr << "Now reading a directory..... ";
        if( is_directory )
        {
            filelist = get_filelist( reference, imagetypes, fs::regular_file );
            if( filelist.empty() )
            {
                cerr << "No image file exist under a directory " << reference.native_file_string() << endl << endl;
                usage( argv[0], reference, imgout_format, vidout_format );
                exit(1);
            }
            filename = filelist.begin();
        }
        else
        {
            if( !fs::exists( reference ) )
            {
                cerr << "The image file " << reference.native_file_string() << " does not exist." << endl << endl;
                usage( argv[0], reference, imgout_format, vidout_format );
                exit(1);
            }
            filelist = get_filelist( reference.branch_path(), imagetypes, fs::regular_file );
            // step up till specified file
            for( filename = filelist.begin(); filename != filelist.end(); filename++ )
            {
                if( filename->native_file_string() == reference.native_file_string() ) break;
            }
        }
        cerr << "Done!" << endl;
        img = cvLoadImage( filename->native_file_string().c_str() );
    }
    else if( is_video )
    {
        if ( !fs::exists( reference ) )
        {
            cerr << "The file " << reference.native_file_string() << " does not exist or is not readable." << endl << endl;
            usage( argv[0], reference, imgout_format, vidout_format );
            exit(1);
        }
        cap = cvCaptureFromFile( reference.native_file_string().c_str() );
        img = cvQueryFrame( cap );
        if( img == NULL )
        {
            cerr << "The file " << reference.native_file_string() << " was assumed as a video, but not loadable." << endl << endl;
            usage( argv[0], reference, imgout_format, vidout_format );
            exit(1);
        }
#if defined(WIN32) || defined(WIN64)
        img->origin = 0;
        cvFlip( img );
#endif
    }
    else
    {
        cerr << "The directory " << reference.native_file_string() << " does not exist." << endl << endl;
        usage( argv[0], reference, imgout_format, vidout_format );
        exit(1);
    }

    //// User interface
    MyMouseStruct* param = &myMouseStruct( w_name, img, cvRect(0,0,0,0) );

    cvNamedWindow( param->w_name, CV_WINDOW_AUTOSIZE );
    cvShowImage( param->w_name, param->img );
    cvSetMouseCallback( param->w_name, on_mouse, (void *)param );
    if( show ) cvNamedWindow( "Cropped", CV_WINDOW_AUTOSIZE );

    int frame = 1; // for video
    while( true ) // key callback
    {
        int key = cvWaitKey( 0 );
        if( key == 's' || key == 32 ) // SPACE
        {
            CvRect region = param->region;
            if( region.x < 0 )
            {
                region.width += region.x;
                region.x = 0; // += region.x
            }
            if( region.y < 0 )
            {
                region.height += region.y;
                region.y = 0;
            }
            region.width = min( param->img->width - region.x, region.width );
            region.height = min( param->img->height - region.y, region.height );
            if( region.width > 1 || region.height > 1 )
            {
                fs::path path = is_video ? reference : *filename;
                string extension = string( fs::extension( path ), 1 );
                string stem = fs::basename( path );
                string dirname = path.branch_path().native_file_string();
                string output_filename = convert_format( output_format, dirname, stem, extension, 
                    region.x, region.y, region.width, region.height, frame );
                fs::path output_path = fs::path( output_filename );

                fs::create_directories( output_path.branch_path() );
                if( !boost::regex_match( fs::extension( output_path ), imagetypes ) )
                {
                    cerr << "The image type " << fs::extension( output_path ) << " is not supported." << endl;
                    exit(1);
                }
                IplImage* crop = cvCreateImage( cvSize( region.width, region.height ), img->depth, img->nChannels );
                cvSetImageROI( img, region );
                cvCopy( img, crop );
                cvResetImageROI( img );
                cvSaveImage( output_path.native_file_string().c_str(), crop );
                cout << output_path.native_file_string() << endl;
                if( show ) cvShowImage( "Cropped", crop );
                cvReleaseImage( &crop );
            }
        }
        if( key == 'f' || key == 32 ) // SPACE
        {
            if( is_video )
            {
                IplImage* tmpimg;
                if( tmpimg = cvQueryFrame( cap ) )
                {
                    param->img = tmpimg;
#if defined(WIN32) || defined(WIN64)
                    param->img->origin = 0;
                    cvFlip( param->img );
#endif
                    frame++;
                    cvShowImageAndRectangle( param->w_name, param->img, param->region );
                }
            }
            else
            {
                if( filename + 1 != filelist.end() )
                {
                    cvReleaseImage( &param->img );
                    filename++;
                    param->img = cvLoadImage( filename->native_file_string().c_str() );
                    cvShowImageAndRectangle( param->w_name, param->img, param->region );
                }
            }
        }
        else if( key == 'b' )
        {
            if( !is_video )
            {
                if( filename != filelist.begin() ) 
                {
                    cvReleaseImage( &param->img );
                    filename--;
                    param->img = cvLoadImage( filename->native_file_string().c_str() );
                    cvShowImageAndRectangle( param->w_name, param->img, param->region );
                }
            }
        }
        else if( key == 'q' || key == 27 ) // ESC
        {
            break;
        }
    }
    cvDestroyWindow( param->w_name );
    if( show ) cvDestroyWindow( "Cropped" );
}

