/**
// cvxrectangle.h
//
// Copyright (c) 2008, Naotoshi Seo. All rights reserved.
//
// The program is free to use for non-commercial academic purposes,
// but for course works, you must understand what is going inside to 
// use. The program can be used, modified, or re-distributed for any 
// purposes only if you or one of your group understand not only 
// programming codes but also theory and math behind (if any). 
// Please contact the authors if you are interested in using the 
// program without meeting the above conditions.
*/
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )
#pragma comment( lib, "cv.lib" )
#pragma comment( lib, "cxcore.lib" )
#pragma comment( lib, "cvaux.lib" )
#endif

#include "cv.h"
#include "cvaux.h"

#ifndef CV_RECTANGLE_INCLUDED
#define CV_RECTANGLE_INCLUDED

// Trivial inline functions
inline double cvPointNorm( CvPoint p1, CvPoint p2, int norm_type = CV_L2 )
{
    // support only sqrt( sum( (p1 - p2)^2 ) )
    return sqrt( pow( (double)p2.x - p1.x, 2 ) + pow( (double)p2.y - p1.y, 2 ) );
}

inline void cvPrintRect( CvRect &rect )
{
    printf( "%d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
}


/**
// Crop image with rectangle
//
// @param IplImage* img The target image
// @param IplImage* dst The cropped image
//    IplImage* dst = cvCreateImage( cvSize( rect.width, rect.height ), img->depth, img->nChannels );
// @param CvRect rect   The rectangle region
// @param double degree The rotation degree of rectangle region
// @return void
*/
CVAPI(void) cvCropImage( IplImage* img, IplImage* dst, CvRect rect, double degree = 0 )
{
    CV_FUNCNAME( "cvCropImage" );
    __BEGIN__;
    CV_ASSERT( rect.width > 0 && rect.height > 0 );
    CV_ASSERT( dst->width == rect.width );
    CV_ASSERT( dst->height == rect.height );
    
    if( degree == 0 )
    {
        cvSetImageROI( img, rect );
        cvCopy( img, dst );
        cvResetImageROI( img );                
    }
    else
    {
        int x, y, xp, yp, z;
        CvMat* map_matrix = cvCreateMat( 2, 3, CV_32FC1 );
        CvPoint2D32f center = cvPoint2D32f( rect.x, rect.y );
        cv2DRotationMatrix( center, degree, 1.0, map_matrix );
        CvMat* xy = cvCreateMat( 3, 1, CV_32FC1 ); cvmSet( xy, 2, 0, 1 );
        CvMat* xyp = cvCreateMat( 2, 1, CV_32FC1 );
        cvZero( dst );
        for( x = 0; x < rect.width; x++ )
        {
            cvmSet( xy, 0, 0, x + rect.x );
            for( y = 0; y < rect.height; y ++ )
            {
                cvmSet( xy, 1, 0, y + rect.y );
                cvMatMul( map_matrix, xy, xyp );
                xp = (int)cvmGet( xyp, 0, 0 );
                yp = (int)cvmGet( xyp, 1, 0 );
                if( xp < 0 || xp > img->width || yp < 0 || yp > img->height ) continue;
                for( z = 0; z < img->nChannels; z++ )
                {
                    dst->imageData[dst->widthStep * y + x * dst->nChannels + z]
                        = img->imageData[img->widthStep * yp + xp * img->nChannels + z];
                }
            }
        }
    }
    //cvNamedWindow("hoge");
    //cvShowImage("hoge", dst);
    __END__;
}

/**
// Draw a rotated rectangle
//
// @param IplImage* img The image to be drawn rectangle
// @param CvRect rect   The rectangle region
// @param double degree The rotation degree of rectangle region
// @param CvScalar color Color
// @return void
*/
CVAPI(void) cvRotatedRectangle( IplImage* img, CvRect rect, double degree, CvScalar color, int thickness = 1, int line_type = 8, int shift = 0)
{
    CV_FUNCNAME( "cvRotatedRectangle" );
    __BEGIN__;
    CV_ASSERT( rect.width > 0 && rect.height > 0 );

    if( degree == 0 )
    {
        CvPoint pt1 = cvPoint( rect.x, rect.y );
        CvPoint pt2 = cvPoint( rect.x + rect.width, rect.y + rect.height );
        cvRectangle( img, pt1, pt2, color, thickness, line_type, shift );
    }
    else
    {
        //IplImage* mask = cvCreateImage( cvGetSize(img), img->depth, img->nChannels );
        //cvZero( mask );
        //for( x = rect.x; x < rect.x + rect.width; x++ )
        //{
        //    for( y = rect.y; y < rect.y + rect.height; y++ )
        //    {
        //        mask->imageData[mask->widthStep * y + x] = 1;
        //    }
        //}
        //cvWarpAffine( mask, mask, map_matrix, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0) );

        int x, y, xp, yp, z;
        CvMat* map_matrix = cvCreateMat( 2, 3, CV_32FC1 );
        CvPoint2D32f center = cvPoint2D32f( rect.x, rect.y );
        cv2DRotationMatrix( center, degree, 1.0, map_matrix );
        CvMat* xy = cvCreateMat( 3, 1, CV_32FC1 ); cvmSet( xy, 2, 0, 1 );
        CvMat* xyp = cvCreateMat( 2, 1, CV_32FC1 );
        /*
        [x' y']t = [o o o  * [x y 1]t
                    o o o]
        */
        for( x = 0; x < rect.width; x++ )
        {
            cvmSet( xy, 0, 0, x + rect.x );
            for( y = 0; y < rect.height; y += max(1, rect.height - 1) )
            {
                cvmSet( xy, 1, 0, y + rect.y );
                cvMatMul( map_matrix, xy, xyp );
                xp = (int)cvmGet( xyp, 0, 0 );
                yp = (int)cvmGet( xyp, 1, 0 );
                if( xp < 0 || xp > img->width || yp < 0 || yp > img->height ) continue;
                for( z = 0; z < img->nChannels; z++ )
                {
                    img->imageData[img->widthStep * yp + xp * img->nChannels + z] = (char)color.val[z];
                }
            }
        }
        for( y = 0; y < rect.height; y++ )
        {
            cvmSet( xy, 1, 0, y + rect.y );
            for( x = 0; x < rect.width; x += max( 1, rect.width - 1) )
            {
                cvmSet( xy, 0, 0, x + rect.x );
                cvMatMul( map_matrix, xy, xyp );
                xp = (int)cvmGet( xyp, 0, 0 );
                yp = (int)cvmGet( xyp, 1, 0 );
                if( xp < 0 || xp > img->width || yp < 0 || yp > img->height ) continue;
                for( z = 0; z < img->nChannels; z++ )
                {
                    img->imageData[img->widthStep * yp + xp * img->nChannels + z] = (char)color.val[z];
                }
            }
        }
    }
    __END__;
}

#endif
