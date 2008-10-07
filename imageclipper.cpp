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
#include "get_filelist.h"
#include "convert_format.h"
#include "cvxrectangle.h"
using namespace std;
namespace fs = boost::filesystem;

/**
* A structure for cvSetMouseCallback function
*/
/*
typedef struct ImageClipperMouse {
    const char* w_name;
    IplImage* img;
    CvRect rect;
    CvRect circle; // use x, y for center, width as radius. width == 0 means watershed is off
} ImageClipperMouse ;

inline ImageClipperMouse imageClipperMouse( const char* w_name, IplImage* img, CvRect& rect, CvRect& circle )
{
    ImageClipperMouse m = { w_name, img, rect, circle };
    return m;
}*/
const char* param_w_name;
IplImage*   param_img;
CvRect      param_rect;
CvRect      param_circle;
int         param_degree;

inline CvRect cvShowImageAndWatershed( const char* w_name, const IplImage* img, const CvRect &circle )
{
    IplImage* clone = cvCloneImage( img );
    IplImage* markers  = cvCreateImage( cvGetSize( clone ), IPL_DEPTH_32S, 1 );
    CvPoint center = cvPoint( circle.x, circle.y );
    int radius = circle.width;

    // Set watershed markers. Now, marker's shape is like circle
    // Set (1 * radius) - (3 * radius) region as ambiguous region (0), intuitively
    cvSet( markers, cvScalarAll( 1 ) );
    cvCircle( markers, center, 3 * radius, cvScalarAll( 0 ), CV_FILLED, 8, 0 );
    cvCircle( markers, center, radius, cvScalarAll( 2 ), CV_FILLED, 8, 0 );
    cvWatershed( clone, markers );

    // Draw watershed markers and rectangle surrounding watershed markers
    cvCircle( clone, center, radius, cvScalarAll (255), 2, 8, 0);

    CvPoint minpoint = cvPoint( markers->width, markers->height );
    CvPoint maxpoint = cvPoint( 0, 0 );
    for (int y = 1; y < markers->height-1; y++) { // looks outer boundary is always -1. 
        for (int x = 1; x < markers->width-1; x++) {
            int* idx = (int *) cvPtr2D (markers, y, x, NULL);
            if (*idx == -1) { // watershed marker -1
                cvSet2D (clone, y, x, cvScalarAll (255));
                if( x < minpoint.x ) minpoint.x = x;
                if( y < minpoint.y ) minpoint.y = y;
                if( x > maxpoint.x ) maxpoint.x = x;
                if( y > maxpoint.y ) maxpoint.y = y;
            }
        }
    }
    cvRectangle( clone, minpoint, maxpoint, CV_RGB(255, 255, 0), 1 );

    cvShowImage( w_name, clone );
    cvReleaseImage( &clone );

    return cvRect( minpoint.x, minpoint.y, maxpoint.x - minpoint.x, maxpoint.y - minpoint.y );
}

/**
* cvSetMouseCallback function
*/
void on_mouse( int event, int x, int y, int flags, void* arg )
{
    //ImageClipperMouse* param       = (ImageClipperMouse*) arg;
    static CvPoint point0          = cvPoint( 0, 0 );
    static bool move_rect          = false;
    static bool resize_rect_left   = false;
    static bool resize_rect_right  = false;
    static bool resize_rect_top    = false;
    static bool resize_rect_bottom = false;
    static bool move_watershed     = false;
    static bool resize_watershed   = false;

    if( !param_img )
        return;

    if( x >= 32768 ) x -= 65536; // change left outsite to negative
    if( y >= 32768 ) y -= 65536; // change top outside to negative

    // MBUTTON or LBUTTON + SHIFT is to draw wathershed
    if( event == CV_EVENT_MBUTTONDOWN || 
        ( event == CV_EVENT_LBUTTONDOWN && flags & CV_EVENT_FLAG_SHIFTKEY ) ) // initialization
    {
        param_circle.x = x;
        param_circle.y = y;
        cvShowImage( param_w_name, param_img );
    }
    else if( event == CV_EVENT_MOUSEMOVE && flags & CV_EVENT_FLAG_MBUTTON ||
        ( event == CV_EVENT_MOUSEMOVE && flags & CV_EVENT_FLAG_LBUTTON && flags & CV_EVENT_FLAG_SHIFTKEY ) )
    {
        param_circle.width = (int) cvPointNorm( cvPoint( param_circle.x, param_circle.y ), cvPoint( x, y ) );
        param_rect = cvShowImageAndWatershed( param_w_name, param_img, param_circle );
    }

    // LBUTTON is to draw rectangle
    else if( event == CV_EVENT_LBUTTONDOWN ) // initialization
    {
        point0 = cvPoint( x, y );
        param_circle.width = 0; // disable watershed
        cvShowImage( param_w_name, param_img );
    }
    else if( event == CV_EVENT_MOUSEMOVE && flags & CV_EVENT_FLAG_LBUTTON )
    {
        param_rect.x = min( point0.x, x );
        param_rect.y = min( point0.y, y );
        param_rect.width =  abs( point0.x - x );
        param_rect.height = abs( point0.y - y );

        cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
    }

    // RBUTTON to move rentangle or watershed marker
    else if( event == CV_EVENT_RBUTTONDOWN )
    {
        point0 = cvPoint( x, y );

        if( param_circle.width != 0 )
        {
            CvPoint center = cvPoint( param_circle.x, param_circle.y );
            int radius = (int) cvPointNorm( center, point0 );
            if( param_circle.width - 1 <= radius && radius <= param_circle.width )
            {
                resize_watershed = true;
            }
            else if( radius <= param_circle.width )
            {
                move_watershed = true;
            }
        }
        if( !resize_watershed && !move_watershed )
        {
            param_circle.width = 0;
            if( ( param_rect.x < x && x < param_rect.x + param_rect.width ) && 
                ( param_rect.y < y && y < param_rect.y + param_rect.height ) )
            {
                move_rect = true;
            }
            if( x <= param_rect.x )
            {
                resize_rect_left = true; 
            }
            else if( x >= param_rect.x + param_rect.width )
            {
                resize_rect_right = true;
            }
            if( y <= param_rect.y )
            {
                resize_rect_top = true; 
            }
            else if( y >= param_rect.y + param_rect.height )
            {
                resize_rect_bottom = true;
            }
        }
    }
    else if( event == CV_EVENT_MOUSEMOVE && flags & CV_EVENT_FLAG_RBUTTON && param_circle.width != 0 ) // Move or resize for watershed
    {
        if( move_watershed )
        {
            CvPoint move = cvPoint( x - point0.x, y - point0.y );
            param_circle.x += move.x;
            param_circle.y += move.y;

            param_rect = cvShowImageAndWatershed( param_w_name, param_img, param_circle );

            point0 = cvPoint( x, y );
        }
        else if( resize_watershed )
        {
            param_circle.width = (int) cvPointNorm( cvPoint( param_circle.x, param_circle.y ), cvPoint( x, y ) );
            param_rect = cvShowImageAndWatershed( param_w_name, param_img, param_circle );
        }
    }
    else if( event == CV_EVENT_MOUSEMOVE && flags & CV_EVENT_FLAG_RBUTTON ) // Move or resize for rectangle
    {
        if( move_rect )
        {
            CvPoint move = cvPoint( x - point0.x, y - point0.y );
            param_rect.x += move.x;
            param_rect.y += move.y;
        }
        if( resize_rect_left )
        {
            int move_x = x - point0.x;
            param_rect.x += move_x;
            param_rect.width -= move_x;
        }
        else if( resize_rect_right )
        {
            int move_x = x - point0.x;
            param_rect.width += move_x;
        }
        if( resize_rect_top )
        {
            int move_y = y - point0.y;
            param_rect.y += move_y;
            param_rect.height -= move_y;
        }
        else if( resize_rect_bottom )
        {
            int move_y = y - point0.y;
            param_rect.height += move_y;
        }

        // assure width is positive
        if( param_rect.width <= 0 )
        {
            param_rect.x += param_rect.width;
            param_rect.width *= -1;
            bool tmp = resize_rect_right;
            resize_rect_right = resize_rect_left;
            resize_rect_left  = tmp;
        }
        // assure height is positive
        if( param_rect.height <= 0 )
        {
            param_rect.y += param_rect.height;
            param_rect.height *= -1;
            bool tmp = resize_rect_top;
            resize_rect_top    = resize_rect_bottom;
            resize_rect_bottom = tmp;
        }

        cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
        point0 = cvPoint( x, y );
    }

    // common finalization
    else if( event == CV_EVENT_LBUTTONUP || event == CV_EVENT_MBUTTONUP || event == CV_EVENT_RBUTTONUP )
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
           const char* vidout_format, const CvRect &initial_rect )
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
    cerr << "    -o <output_format = imgout_format or vidout_format>" << endl;
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
    cerr << "    -r <initial_rect = " << initial_rect.x << " " << initial_rect.y << " "
        << initial_rect.width << " " << initial_rect.height << ">" << endl;
    cerr << "        Determine the initial rectnagle (left_x top_y width height)." << endl;
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
    cerr << "    s (save)        : Save the selected region as an image." << endl;
    cerr << "    f (forward)     : Forward. Show next image." << endl;
    cerr << "    SPACE           : Save and Forward." << endl;
    cerr << "    b (backward)    : Backward. Not available for video file (now)." << endl;
    cerr << "    q (quit) or ESC : Quit. " << endl;
    cerr << "    h (left) j (down) k (up) l (right): Move rectangle." << endl;
    cerr << "    y (left) u (down) i (up) o (right): Resize rectangle." << endl;
    cerr << "    e (expand) E (shrink)             : Resize rectangle keeping ratio." << endl;
    cerr << "    r (rotate clockwise) R (inverse)  : Rotate rectangle." << endl;
}

int main( int argc, char *argv[] )
{
    //// Initialization
    fs::path reference( "." );
    bool show                 = false;
    const char* imgout_format = "%d/imageclipper/%i.%e_%04r_%04x_%04y_%04w_%04h.png";
    const char* vidout_format = "%d/imageclipper/%i.%e_%04f_%04r_%04x_%04y_%04w_%04h.png";
    const char* output_format = NULL;
    boost::regex imagetypes( ".*\\.(bmp|dib|jpeg|jpg|jpe|png|pbm|pgm|ppm|sr|ras|tiff|exr|jp2)$", 
        boost::regex_constants::icase );
    const char* w_name        = "<S> Save <F> Forward <SPACE> s and f <B> Backward <ESC> Exit";
    CvRect initial_rect       = cvRect( 0, 0, 0, 0 );
    int frame                 = 1;

    //// Arguments
    for( int i = 1; i < argc; i++ )
    {
        if( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "--help" ) )
        {
            usage( argv[0], reference, imgout_format, vidout_format, initial_rect );
            return 0;
        } 
        else if( !strcmp( argv[i], "-o" ) || !strcmp( argv[i], "--output_format" ) )
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
        else if( !strcmp( argv[i], "-r" ) || !strcmp( argv[i], "--initial_rect" ) )
        {
            initial_rect.x      = atoi( argv[++i] );
            initial_rect.y      = atoi( argv[++i] );
            initial_rect.width  = atoi( argv[++i] );
            initial_rect.height = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-f" ) || !strcmp( argv[i], "--frame" ) )
        {
            frame = atoi( argv[++i] );
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
                usage( argv[0], reference, imgout_format, vidout_format, initial_rect );
                exit(1);
            }
            filename = filelist.begin();
        }
        else
        {
            if( !fs::exists( reference ) )
            {
                cerr << "The image file " << reference.native_file_string() << " does not exist." << endl << endl;
                usage( argv[0], reference, imgout_format, vidout_format, initial_rect );
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
        cerr << "Now showing " << filename->native_file_string() << endl;
        img = cvLoadImage( filename->native_file_string().c_str() );
    }
    else if( is_video )
    {
        if ( !fs::exists( reference ) )
        {
            cerr << "The file " << reference.native_file_string() << " does not exist or is not readable." << endl << endl;
            usage( argv[0], reference, imgout_format, vidout_format, initial_rect );
            exit(1);
        }
        cerr << "Now reading a video..... ";
        cap = cvCaptureFromFile( reference.native_file_string().c_str() );
        for( int i = 0; i < frame; i++ )
        {
            img = cvQueryFrame( cap );
            if( img == NULL )
            {
                cerr << "The file " << reference.native_file_string() << " was assumed as a video, but not loadable." << endl << endl;
                usage( argv[0], reference, imgout_format, vidout_format, initial_rect );
                exit(1);
            }
        }
        cerr << "Done!" << endl;
        cerr << "Now showing the frame " << frame << endl;
#if defined(WIN32) || defined(WIN64)
        img->origin = 0;
        cvFlip( img );
#endif
    }
    else
    {
        cerr << "The directory " << reference.native_file_string() << " does not exist." << endl << endl;
        usage( argv[0], reference, imgout_format, vidout_format, initial_rect );
        exit(1);
    }

    //// Mouse and Key callback
    //ImageClipperMouse* param = &imageClipperMouse( w_name, img, initial_rect, cvRect(0,0,0,0) );
    param_w_name = w_name;
    param_img = img;
    param_rect = initial_rect;
    param_circle = cvRect(0,0,0,0);
    param_degree = 0;
    cvNamedWindow( param_w_name, CV_WINDOW_AUTOSIZE );
    if( param_rect.width != 0 && param_rect.height != 0 )
        cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
    else
        cvShowImage( param_w_name, param_img );
    //cvSetMouseCallback( param_w_name, on_mouse, (void *)param );
    cvSetMouseCallback( param_w_name, on_mouse );
    if( show ) cvNamedWindow( "Cropped", CV_WINDOW_AUTOSIZE );

    while( true ) // key callback
    {
        int key = cvWaitKey( 0 );
        // Save
        if( key == 's' || key == 32 ) // 32 is SPACE
        {
            // If the rectangle runs off outside image, pick only inside regions
            CvRect rect = param_rect;
            if( rect.x < 0 )
            {
                rect.width += rect.x;
                rect.x = 0; // += rect.x
            }
            if( rect.y < 0 )
            {
                rect.height += rect.y;
                rect.y = 0;
            }
            rect.width = min( param_img->width - rect.x, rect.width );
            rect.height = min( param_img->height - rect.y, rect.height );

            // save into image
            if( rect.width > 1 || rect.height > 1 )
            {
                fs::path path = is_video ? reference : *filename;
                string extension = string( fs::extension( path ), 1 );
                string stem = fs::basename( path );
                string dirname = path.branch_path().native_file_string();
                string output_filename = convert_format( output_format, dirname, stem, extension, 
                    rect.x, rect.y, rect.width, rect.height, frame, param_degree );
                fs::path output_path = fs::path( output_filename );

                fs::create_directories( output_path.branch_path() );
                if( !boost::regex_match( fs::extension( output_path ), imagetypes ) )
                {
                    cerr << "The image type " << fs::extension( output_path ) << " is not supported." << endl;
                    exit(1);
                }
                IplImage* crop = cvCreateImage( cvSize( rect.width, rect.height ), param_img->depth, param_img->nChannels );
                cvCropImageRotatedROI( param_img, crop, rect, param_degree );
                cvSaveImage( output_path.native_file_string().c_str(), crop );
                cout << output_path.native_file_string() << endl;
                if( show ) cvShowImage( "Cropped", crop );
                cvReleaseImage( &crop );
            }
        }
        // Forward
        if( key == 'f' || key == 32 ) // 32 is SPACE
        {
            if( is_video )
            {
                IplImage* tmpimg;
                if( tmpimg = cvQueryFrame( cap ) )
                {
                    param_img = tmpimg;
#if defined(WIN32) || defined(WIN64)
                    param_img->origin = 0;
                    cvFlip( param_img );
#endif
                    frame++;
                    cout << reference.native_file_string() << " " <<  frame << endl;
                }
            }
            else
            {
                if( filename + 1 != filelist.end() )
                {
                    cvReleaseImage( &param_img );
                    filename++;
                    param_img = cvLoadImage( filename->native_file_string().c_str() );
                    cout << filename->native_file_string() << endl;
                }
            }
            if( param_img )
            {
                if( param_circle.width != 0 )
                {
                    param_rect = cvShowImageAndWatershed( param_w_name, param_img, param_circle );
                }
                else if( param_rect.width != 0 )
                {
                    cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
                }
                else
                {
                    cvShowImage( param_w_name, param_img );
                }
            }
        }
        // Backward
        else if( key == 'b' )
        {
            if( !is_video )
            {
                if( filename != filelist.begin() ) 
                {
                    cvReleaseImage( &param_img );
                    filename--;
                    param_img = cvLoadImage( filename->native_file_string().c_str() );
                    cout << filename->native_file_string() << endl;
                }
                if( param_img )
                {
                    if( param_circle.width != 0 )
                    {
                        param_rect = cvShowImageAndWatershed( param_w_name, param_img, param_circle );
                    }
                    else if( param_rect.width != 0 )
                    {
                        cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
                    }
                    else
                    {
                        cvShowImage( param_w_name, param_img );
                    }
                }
            }
        }
        // Exit
        else if( key == 'q' || key == 27 ) // 27 is ESC
        {
            break;
        }
        // Rectangle Movement (Vi like hotkeys)
        else if( key == 'h' ) // Left
        {
            param_rect.x = max( 0, param_rect.x - 1 );
            if( param_rect.width != 0 )
            {
                cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
            }
        }
        else if( key == 'j' ) // Down
        {
            param_rect.y = min( param_img->height - param_rect.height, param_rect.y + 1);
            if( param_rect.width != 0 )
            {
                cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
            }
        }
        else if( key == 'k' ) // Up
        {
            param_rect.y = max( 0, param_rect.y - 1 );
            if( param_rect.width != 0 )
            {
                cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
            }
        }
        else if( key == 'l' ) // Right
        {
            param_rect.x = min( param_img->width - param_rect.width, param_rect.x + 1);
            if( param_rect.width != 0 )
            {
                cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
            }
        }
        else if( key == 'y' ) // Shrink width
        {
            //param_rect.x = min( param_img->width, param_rect.x + 1 );
            //param_rect.width = max( 0, param_rect.width - 2 );
            param_rect.width = max( 0, param_rect.width - 1 );
            cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
        }
        else if( key == 'u' ) // Expand height
        {
            //param_rect.y = max( 0, param_rect.y - 1 );
            //param_rect.height += 2;
            param_rect.height += 1;
            cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
        }
        else if( key == 'i' ) // Shrink height
        {
            //param_rect.y = min( param_img->height, param_rect.y + 1 );
            //param_rect.height = max( 0, param_rect.height - 2 );
            param_rect.height = max( 0, param_rect.height - 1 );
            cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
        }
        else if( key == 'o' ) // Expand width
        {
            //param_rect.x = max( 0, param_rect.x - 1 );
            //param_rect.width += 2;
            param_rect.width += 1;
            cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
        }
        else if( key == 'r' ) // Rotate
        {
            param_degree += 1;
            param_degree = (param_degree >= 360) ? param_degree - 360 : param_degree;
            cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
        }
        else if( key == 'R' ) // Inverse Rotate
        {
            param_degree -= 1;
            param_degree = (param_degree < 0) ? 360 + param_degree : param_degree;
            cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
        }
        if( key == 'e' || key == 'E' ) // Expansion and Shrink so that ratio does not change
        {
            if( param_rect.height != 0 && param_rect.width != 0 ) 
            {
                int gcd, a = param_rect.width, b = param_rect.height;
                while( 1 )
                {
                    a = a % b;
                    if( a == 0 ) { gcd = b; break; }
                    b = b % a;
                    if( b == 0 ) { gcd = a; break; }
                }
                int ratio_width = param_rect.width / gcd;
                int ratio_height = param_rect.height / gcd;
                if( key == 'e' ) gcd += 1;
                else if( key == 'E' ) gcd -= 1;
                if( gcd > 0 )
                {
                    cout << ratio_width << ":" << ratio_height << " * " << gcd << endl;
                    param_rect.width = ratio_width * gcd;
                    param_rect.height = ratio_height * gcd; 
                    cvShowImageAndRectangle( param_w_name, param_img, param_rect, param_degree );
                }
            }
        }
    }
    cvDestroyWindow( param_w_name );
    if( show ) cvDestroyWindow( "Cropped" );
}

