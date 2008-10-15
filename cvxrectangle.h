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
#include "highgui.h"
#include <stdio.h>
#include <math.h>

#ifndef CV_RECTANGLE_INCLUDED
#define CV_RECTANGLE_INCLUDED

/**
// Create a affine transform matrix
//
// @param CvMat* affine                   The 2 x 3 CV_32FC1|CV_64FC1 affine matrix to be created
// @param CvRect [rect = cvRect(0,0,1,1)] The translation (x, y) and scaling (width, height) parameter
// @param double [rotate = 0]             The rotation parameter in degree
// @param double [shear = 0]              The shear deformation orientation parameter in degree
// @return void
*/
CVAPI(void) cvCreateAffine( CvMat* affine, CvRect rect, double rotate = 0, double shear = 0 )
{
    CV_FUNCNAME( "cvCreateAffine" );
    __BEGIN__;
    CV_ASSERT( rect.width > 0 && rect.height > 0 );
    CV_ASSERT( affine->rows == 2 && affine->cols == 3 );

    CvMat* rotatetmp = cvCreateMat( 2, 3, CV_32FC1 );
    CvMat* rotation  = cvCreateMat( 2, 2, CV_32FC1 );
    CvMat* neg_shear = cvCreateMat( 2, 2, CV_32FC1 );
    CvMat* scale     = cvCreateMat( 2, 2, CV_32FC1 );
    CvMat* pos_shear = cvCreateMat( 2, 2, CV_32FC1 );

    cvmSet( affine, 0, 2, rect.x );
    cvmSet( affine, 1, 2, rect.y );
    cvZero( scale );
    cvmSet( scale, 0, 0, rect.width );
    cvmSet( scale, 1, 1, rect.height );
    cv2DRotationMatrix( cvPoint2D32f( 0, 0 ), rotate, 1.0, rotatetmp );
    cvmSet( rotation, 0, 0, cvmGet( rotatetmp, 0, 0 ) );
    cvmSet( rotation, 0, 1, cvmGet( rotatetmp, 0, 1 ) );
    cvmSet( rotation, 1, 0, cvmGet( rotatetmp, 1, 0 ) );
    cvmSet( rotation, 1, 1, cvmGet( rotatetmp, 1, 1 ) );
    cv2DRotationMatrix( cvPoint2D32f( 0, 0 ), -shear, 1.0, rotatetmp );
    cvmSet( neg_shear, 0, 0, cvmGet( rotatetmp, 0, 0 ) );
    cvmSet( neg_shear, 0, 1, cvmGet( rotatetmp, 0, 1 ) );
    cvmSet( neg_shear, 1, 0, cvmGet( rotatetmp, 1, 0 ) );
    cvmSet( neg_shear, 1, 1, cvmGet( rotatetmp, 1, 1 ) );
    cv2DRotationMatrix( cvPoint2D32f( 0, 0 ), shear, 1.0, rotatetmp );
    cvmSet( pos_shear, 0, 0, cvmGet( rotatetmp, 0, 0 ) );
    cvmSet( pos_shear, 0, 1, cvmGet( rotatetmp, 0, 1 ) );
    cvmSet( pos_shear, 1, 0, cvmGet( rotatetmp, 1, 0 ) );
    cvmSet( pos_shear, 1, 1, cvmGet( rotatetmp, 1, 1 ) );
    cvMatMul( rotation, neg_shear, neg_shear );
    cvMatMul( neg_shear, scale, scale );
    cvMatMul( scale, pos_shear, pos_shear );
    cvmSet( affine, 0, 0, cvmGet( pos_shear, 0, 0 ) );
    cvmSet( affine, 0, 1, cvmGet( pos_shear, 0, 1 ) );
    cvmSet( affine, 1, 0, cvmGet( pos_shear, 1, 0 ) );
    cvmSet( affine, 1, 1, cvmGet( pos_shear, 1, 1 ) );

    cvReleaseMat( &rotatetmp );
    cvReleaseMat( &rotation );
    cvReleaseMat( &neg_shear );
    cvReleaseMat( &scale );
    cvReleaseMat( &pos_shear );
    __END__;
}

/**
// Crop image with rotated and sheared rectangle (affine transformation of (0,0,1,1) rectangle)
//
// @param IplImage* img       The target image
// @param IplImage* dst       The cropped image
//    IplImage* dst = cvCreateImage( cvSize( rect.width, rect.height ), img->depth, img->nChannels );
// @param CvRect rect         The translation (x, y) and scaling (width, height) parameter or the rectangle region
// @param double [rotate = 0] The rotation parameter in degree
// @param double [shear = 0]  The shear deformation orientation parameter in degree
// @return void
*/
CVAPI(void) cvCropImageROI( IplImage* img, IplImage* dst, CvRect rect, double rotate = 0, double shear = 0 )
{
    CV_FUNCNAME( "cvCropImageROI" );
    __BEGIN__;
    CV_ASSERT( rect.width > 0 && rect.height > 0 );
    CV_ASSERT( dst->width == rect.width );
    CV_ASSERT( dst->height == rect.height );


    if( rotate == 0 && shear == 0 && 
        rect.x >= 0 && rect.y >= 0 && 
        rect.x + rect.width < img->width && rect.y + rect.height < img->height )
    {
        cvSetImageROI( img, rect );
        cvCopy( img, dst );
        cvResetImageROI( img );                
    }
    else
    {
        int x, y, z, xp, yp;
        CvMat* affine = cvCreateMat( 2, 3, CV_32FC1 );
        CvMat* xy     = cvCreateMat( 3, 1, CV_32FC1 );
        CvMat* xyp    = cvCreateMat( 2, 1, CV_32FC1 );
        cvmSet( xy, 2, 0, 1.0 );
        cvCreateAffine( affine, rect, rotate, shear );
        cvZero( dst );

        for( x = 0; x < rect.width; x++ )
        {
            cvmSet( xy, 0, 0, x / (double) rect.width );
            for( y = 0; y < rect.height; y++ )
            {
                cvmSet( xy, 1, 0, y / (double) rect.height );
                cvMatMul( affine, xy, xyp );
                xp = (int)cvmGet( xyp, 0, 0 );
                yp = (int)cvmGet( xyp, 1, 0 );
                if( xp < 0 || xp >= img->width || yp < 0 || yp >= img->height ) continue;
                for( z = 0; z < img->nChannels; z++ )
                {
                    dst->imageData[dst->widthStep * y + x * dst->nChannels + z]
                    = img->imageData[img->widthStep * yp + xp * img->nChannels + z];
                }
            }
        }
        cvReleaseMat( &affine );
        cvReleaseMat( &xy );
        cvReleaseMat( &xyp );
    }
    __END__;
}

/**
// Draw an rotated and sheared rectangle (affine transformation of (0,0,1,1) rectangle)
//
// @param IplImage* img       The image to be drawn rectangle
// @param CvRect rect         The translation (x, y) and scaling (width, height) parameter or the rectangle region
// @param double [rotate = 0] The rotation parameter in degree
// @param double [shear = 0]  The shear deformation orientation parameter in degree
// @param CvScalar color      The color
// @todo support thickness, line_type, shift
// @return void
*/
CVAPI(void) cvDrawRectangle( IplImage* img, CvRect rect, double rotate = 0, double shear = 0, 
                            CvScalar color = CV_RGB(255, 255, 255), int thickness = 1, int line_type = 8, int shift = 0)
{
    CV_FUNCNAME( "cvDrawRectangle" );
    __BEGIN__;
    CV_ASSERT( rect.width > 0 && rect.height > 0 );

    if( rotate == 0 && shear == 0 )
    {
        CvPoint pt1 = cvPoint( rect.x, rect.y );
        CvPoint pt2 = cvPoint( rect.x + rect.width, rect.y + rect.height );
        cvRectangle( img, pt1, pt2, color, thickness, line_type, shift );
    }
    else
    {
        int x, y, z, xp, yp;
        CvMat* affine = cvCreateMat( 2, 3, CV_32FC1 );
        CvMat* xy     = cvCreateMat( 3, 1, CV_32FC1 );
        CvMat* xyp    = cvCreateMat( 2, 1, CV_32FC1 );
        cvmSet( xy, 2, 0, 1.0 );
        cvCreateAffine( affine, rect, rotate, shear );

        for( x = 0; x < rect.width; x++ )
        {
            cvmSet( xy, 0, 0, x / (double) rect.width );
            for( y = 0; y < rect.height; y += max(1, rect.height - 1) )
            {
                cvmSet( xy, 1, 0, y / (double) rect.height );
                cvMatMul( affine, xy, xyp );
                xp = (int)cvmGet( xyp, 0, 0 );
                yp = (int)cvmGet( xyp, 1, 0 );
                if( xp < 0 || xp >= img->width || yp < 0 || yp >= img->height ) continue;
                for( z = 0; z < img->nChannels; z++ )
                {
                    img->imageData[img->widthStep * yp + xp * img->nChannels + z] = (char)color.val[z];
                }
            }
        }
        for( y = 0; y < rect.height; y++ )
        {
            cvmSet( xy, 1, 0, y / (double) rect.height );
            for( x = 0; x < rect.width; x += max( 1, rect.width - 1) )
            {
                cvmSet( xy, 0, 0, x / (double) rect.width );
                cvMatMul( affine, xy, xyp );
                xp = (int)cvmGet( xyp, 0, 0 );
                yp = (int)cvmGet( xyp, 1, 0 );
                if( xp < 0 || xp >= img->width || yp < 0 || yp >= img->height ) continue;
                for( z = 0; z < img->nChannels; z++ )
                {
                    img->imageData[img->widthStep * yp + xp * img->nChannels + z] = (char)color.val[z];
                }
            }
        }
        cvReleaseMat( &affine );
        cvReleaseMat( &xy );
        cvReleaseMat( &xyp );
    }
    __END__;
}


// Trivial inline functions
CV_INLINE double cvPointNorm( CvPoint p1, CvPoint p2, int norm_type = CV_L2 )
{
    // support only sqrt( sum( (p1 - p2)^2 ) )
    return sqrt( pow( (double)p2.x - p1.x, 2 ) + pow( (double)p2.y - p1.y, 2 ) );
}

CV_INLINE void cvPrintRect( const CvRect &rect )
{
    printf( "%d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
}

CV_INLINE void cvShowImageAndRectangle( const char* w_name, const IplImage* img, const CvRect& rect, double rotate = 0, double shear = 0,
                                       CvScalar color = CV_RGB(255, 255, 0), int thickness = 1, int line_type = 8, int shift = 0)
{
    if( rect.width <= 0 || rect.height <= 0 )
    {
        cvShowImage( w_name, img );
        return;
    }
    IplImage* clone = cvCloneImage( img );
    cvDrawRectangle( clone, rect, rotate, shear, color, thickness, line_type, shift );
    cvShowImage( w_name, clone );
    cvReleaseImage( &clone );
}

CV_INLINE void cvShowCroppedImage( const char* w_name, IplImage* orig, const CvRect rect, double rotate = 0, double shear = 0 )
{
    if( rect.width <= 0 || rect.height <= 0 ) return;
    IplImage* crop = cvCreateImage( cvSize( rect.width, rect.height ), orig->depth, orig->nChannels );
    cvCropImageROI( orig, crop, rect, rotate, shear );
    cvShowImage( w_name, crop );
    cvReleaseImage( &crop );
}

// If the rectangle runs off outside image, pick only inside regions
CV_INLINE CvRect cvValidateRect( CvRect rect, int max_width, int max_height )
{
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
    rect.width = min( max_width - rect.x, rect.width );
    rect.height = min( max_height - rect.y, rect.height );
    return rect;
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif