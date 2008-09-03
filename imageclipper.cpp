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
* @param const boost::filesystem::path &dirpath 
*     path to the target directory
* @param [const boost::regex regex = boost::regex(".*")] 
*     regular expression (instead of wild cards)
* @param [boost::filesystem::file_type file_type = boost::filesystem::type_unknown] 
*     list only specified file_type. The default is for all
* @return vector<boost::filesystem::path> 
*     vector of file list
* @requirements 
*     boost::filesystem, boost::regex, std::vector, std::string 
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
* @param const string& format 
*     the format string
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

typedef struct callback {
    const char* w_name;
    IplImage* img;
    bool lbuttondown;
    bool rbuttondown;
    CvPoint point0;
    CvPoint point1;
    CvRect region;
} MouseStruct ;

inline void cvRectangleByRect( CvArr* img, CvRect rect, CvScalar color,
                              int thickness=1, int line_type=8, int shift=0 )
{
    CvPoint pt1 = cvPoint( rect.x, rect.y );
    CvPoint pt2 = cvPoint( rect.x + rect.width, rect.y + rect.height );
    cvRectangle( img, pt1, pt2, color, thickness, line_type, shift );
}

inline void cvShowImageAndRectangle( const char* w_name, const IplImage* img, const CvRect& rect )
{
    IplImage* cloneimg = cvCloneImage( img );
    cvRectangleByRect( cloneimg, rect, CV_RGB(255, 255, 0), 1); 
    cvShowImage( w_name, cloneimg );
    cvReleaseImage( &cloneimg );
}

/**
* cvSetMouseCallback function
*/
void on_mouse( int event, int x, int y, int flags, void* arg )
{
    MouseStruct* param = (MouseStruct*) arg;

    if( !param->img )
        return;

#if defined(WIN32) || defined(WIN64) // possible to go outside image only on windows
    if( x >= 32768 ) x -= 65536; // change to negative values outside image
    if( y >= 32768 ) y -= 65536;
#endif

    if( event == CV_EVENT_LBUTTONDOWN ) 
    {
        param->lbuttondown = true;
        param->point0 = cvPoint( x, y );
    }
    else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON) ) 
    {
        param->region.x = min( param->point0.x, x );
        param->region.y = min( param->point0.y, y );
        param->region.width =  abs( param->point0.x - x );
        param->region.height = abs( param->point0.y - y );

        cvShowImageAndRectangle( param->w_name, param->img, param->region );
    }
    else if (event == CV_EVENT_LBUTTONUP)
    {
        param->lbuttondown = false;
    }
    else if( event == CV_EVENT_RBUTTONDOWN )
    {
        param->rbuttondown = true;
        param->point0 = cvPoint( x, y );
    }
    else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_RBUTTON) )
    {
        param->point1 = cvPoint( x, y );
        int move_x = x - param->point0.x;
        int move_y = y - param->point0.y;
        param->point0.x = x;
        param->point0.y = y;
        param->region.x += move_x;
        param->region.y += move_y;

        cvShowImageAndRectangle( param->w_name, param->img, param->region );
    }
    else if( event == CV_EVENT_RBUTTONUP )
    {
        param->rbuttondown = false;
    }
}


/**
* Print out usage
*/
void usage( const char* com, const fs::path &reference, const char* imgout_format,
           const char* aviout_format )
{
    cerr << "ImageClipper - image clipping helper tool." << endl;
    cerr << "Command Usage: " << fs::path( com ).leaf();
    cerr << " [option]... [reference]" << endl;
    cerr << "  <reference = " << reference << ">" << endl;
    cerr << "    <reference> would be a directory or an image or a video filename." << endl;
    cerr << "    For a directory, image files in the directory will be read sequentially." << endl;
    cerr << "    For an image, it is started to read directory from this image file." << endl;
    cerr << "    For a video, frames in the video are read sequentially." << endl;
    cerr << endl;
    cerr << "  Options" << endl;
    cerr << "    -o <output_format = " << imgout_format << " (image) " << endl;
    cerr << "            " << aviout_format << " (video)>" << endl;
    cerr << "        Determine the output file format and path." << endl;
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
    cerr << "        Supprted Image Type)" << endl;
    cerr << "            bmp|dib|jpeg|jpg|jpe|png|pbm|pgm|ppm|sr|ras|tiff|exr|jp2" << endl;
    cerr << "    -h" << endl;
    cerr << "    --help" << endl;
    cerr << "        Show this help" << endl;
    cerr << "    -s" << endl;
    cerr << "    --show" << endl;
    cerr << "        Show clipped image" << endl;
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
    // Initialization
    fs::path reference( "." );
    bool show = false;
    const char* imgout_format = "%d/imageclipper/%i.%e_%04x_%04y_%04w_%04h.png";
    const char* aviout_format = "%d/imageclipper/%i.%e_%04f_%04x_%04y_%04w_%04h.png";
    const char* output_format = NULL;
    boost::regex imagetypes( ".*\\.(bmp|dib|jpeg|jpg|jpe|png|pbm|pgm|ppm|sr|ras|tiff|exr|jp2)$", 
        boost::regex_constants::icase );
    boost::regex videotypes( ".*\\.(avi|mpg|mpeg)$", // readablility depends on codec
        boost::regex_constants::icase ); 
    const char* w_name = "<S> Save <F> Forward <SPACE> s and f <B> Backward <ESC> Exit";

    // Arguments
    for( int i = 1; i < argc; i++ )
    {
        if( !strcmp( argv[i], "-h" ) || !strcmp( argv[i], "--help" ) )
        {
            usage( argv[0], reference, imgout_format, aviout_format );
            return 0;
        } 
        else if( !strcmp( argv[i], "-o" ) || !strcmp( argv[i], "--output" ) )
        {
            output_format = argv[++i];
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

    // Retreive image files or video
    bool is_directory = fs::is_directory( reference );
    bool is_image = boost::regex_match( reference.native_file_string(), imagetypes );
    bool is_video = boost::regex_match( reference.native_file_string(), videotypes );

    vector<fs::path> filelist; // for image
    vector<fs::path>::iterator filename; // for image
    CvCapture* cap; // for video
    IplImage *img;
    if( is_directory || is_image )
    {
        if( output_format == NULL ) output_format = imgout_format;

        cerr << "Now reading the directory..... ";
        if( is_directory )
        {
            filelist = get_filelist( reference, imagetypes, fs::regular_file );
            if( filelist.empty() )
            {
                cerr << "No image file exist under a directory " << reference.native_file_string() << endl << endl;
                usage( argv[0], reference, imgout_format, aviout_format );
                exit(1);
            }
            filename = filelist.begin();
        }
        else
        {
            if( !fs::exists( reference ) )
            {
                cerr << "The image file " << reference.native_file_string() << " does not exist." << endl << endl;
                usage( argv[0], reference, imgout_format, aviout_format );
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
        if( output_format == NULL ) output_format = aviout_format;

        if ( !fs::exists( reference ) )
        {
            cerr << "The video file " << reference.native_file_string() << " does not exist or is not readable." << endl << endl;
            usage( argv[0], reference, imgout_format, aviout_format );
            exit(1);
        }
        cap = cvCaptureFromFile( reference.native_file_string().c_str() );
        img = cvQueryFrame( cap );
        if( img == NULL )
        {
            cerr << "The video file " << reference.native_file_string() << " is corrupted or it's codec is not supported." << endl << endl;
            usage( argv[0], reference, imgout_format, aviout_format );
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
        usage( argv[0], reference, imgout_format, aviout_format );
        exit(1);
    }

    // User interface
    MouseStruct param = { w_name, img, false, false, cvPoint(0,0), cvPoint(0,0), cvRect(0,0,0,0) };
    // KeyStruct keystruct = { w_name, img, reference, cap, filename, filelist, param.region }
    cvNamedWindow( param.w_name, CV_WINDOW_AUTOSIZE );
    cvShowImage( param.w_name, param.img );
    cvSetMouseCallback( param.w_name, on_mouse, (void *)&param );
    if( show ) cvNamedWindow( "Cropped", CV_WINDOW_AUTOSIZE );

    int frame = 1; // for video
    while( true ) // key callback
    {
        int key = cvWaitKey( 0 );
        if( key == 's' || key == 32 ) // SPACE
        {
            CvRect region = param.region;
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
            region.width = min( param.img->width - region.x, region.width );
            region.height = min( param.img->height - region.y, region.height );
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
                    param.img = tmpimg;
#if defined(WIN32) || defined(WIN64)
                    param.img->origin = 0;
                    cvFlip( param.img );
#endif
                    frame++;
                    cvShowImageAndRectangle( param.w_name, param.img, param.region );
                }
            }
            else
            {
                if( filename + 1 != filelist.end() )
                {
                    cvReleaseImage( &param.img );
                    filename++;
                    param.img = cvLoadImage( filename->native_file_string().c_str() );
                    cvShowImageAndRectangle( param.w_name, param.img, param.region );
                }
            }
        }
        else if( key == 'b' )
        {
            if( !is_video )
            {
                if( filename != filelist.begin() ) 
                {
                    cvReleaseImage( &param.img );
                    filename--;
                    param.img = cvLoadImage( filename->native_file_string().c_str() );
                    cvShowImageAndRectangle( param.w_name, param.img, param.region );
                }
            }
        }
        else if( key == 'q' || key == 27 ) // ESC
        {
            break;
        }
    }
    cvDestroyWindow( param.w_name );
    cvReleaseImage( &param.img );
    if( show ) cvDestroyWindow( "Cropped" );
}

