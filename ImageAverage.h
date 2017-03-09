#ifndef ImageAverage_h
#define ImageAverage_h

#include "cpputils.h"
#include <opencv2/opencv.hpp>
using namespace cv;

//1. introduce all images -> first clusters (each cluster represents one png image)
//2. Create new clusters by taking the average of clusters that are close by. Require clusters don't differ more than DM * avg diff (calculate average diff first)
//3. Repeat 2. until the number of clusters is low enough (for terrain images, probably between 4 and 10 (for seasonal groups)


#define TWODIM_HISTOGRAM_WIDTH 100
#define THREEDIM_HISTOGRAM_WIDTH 10

class ImageAverage{
private:
    double *mR;
    double *mG;
    double *mB;
    int mW;
    int mH;
    int mLen;
    int mBPP;
    int mType;
    
    double mSumMembers;//for the average
    //Raw image
    uchar* mOutData;
    
    //2d color histogram
    uchar* mHistogramOutData;
    static uchar* mHistogramColors;//colors represented by the histogram locations
    int* m2dColorHistogram;//pack the color into a triangle
    int m2dColorHistogramLen;
    
    int* m3dColorHistogram;
    int m3dColorHistogramLen;
    
    
    bool mImageReady;
    
    bool mMemorySmall;
    
public:
    ImageAverage* extNearbyCluster;
    float extNearbyClusterDist;
    int extId;
public:
    static ImageAverage* create( const Mat& aImg )
    {
        return new ImageAverage( aImg.cols, aImg.rows, aImg.type() );
    }
    ImageAverage( int aW, int aH, int aType ){
        mMemorySmall = false;
        extNearbyCluster = 0;
        mHistogramColors = 0;
        mHistogramOutData = 0;
        m2dColorHistogram = 0;
        m3dColorHistogram = 0;
        mType = aType;
        mSumMembers = 0;
        mImageReady = false;
        mW = aW;
        mH = aH;
        mLen = aW * aH;
        mR = (double*)malloc( sizeof(double)*mLen );
        mG = (double*)malloc( sizeof(double)*mLen );
        mB = (double*)malloc( sizeof(double)*mLen );
        mOutData = 0;
        
        for( int i = 0; i < mLen; ++i )
        {
            mR[i] = 0;
            mG[i] = 0;
            mB[i] = 0;
        }
    }
    ~ImageAverage()
    {
        if( mOutData ){ free(mOutData); }
        if( mHistogramOutData ){ free(mHistogramOutData); }
        if( mHistogramColors ){free(mHistogramColors);}
        free(mR);
        free(mG);
        free(mB);
        if( m2dColorHistogram ){ free(m2dColorHistogram); }
        if( m3dColorHistogram ){ free(m3dColorHistogram); }
    }
    //Assumes the dimensions are the same
    void addAvg( Mat& aImg )
    {
        //U::p("addAvg");
        mImageReady = false;//because it just changed
        mType = aImg.type();
        mBPP = aImg.elemSize();
        for( int i = 0; i < mLen; ++i )
        {
            mR[i] += ((uchar*)aImg.ptr())[i*mBPP+0];
            mG[i] += ((uchar*)aImg.ptr())[i*mBPP+1];
            mB[i] += ((uchar*)aImg.ptr())[i*mBPP+2];
        }
        mSumMembers += 1.0;
    }
    
    //Returns an image that is only valid while this ImageAverage instance is
    Mat getImage()
    {
        if( mOutData == 0 )
        {
            mOutData = (uchar*)malloc( sizeof(uchar) * mLen * mBPP );
        }
        if( !mImageReady )
        {
            for( int i = 0; i < mLen; ++i )
            {
                mOutData[i*mBPP + 0] = (unsigned char)((int)(mR[i] / mSumMembers ));
                mOutData[i*mBPP + 1] = (unsigned char)((int)(mG[i] / mSumMembers ));
                mOutData[i*mBPP + 2] = (unsigned char)((int)(mB[i] / mSumMembers ));
            }
        }
        Mat image( mH, mW, mType, mOutData );
        mImageReady = true;
        return image;
    }
    Mat getHistogramImage()
    {
        collect2dHistogram();
        getHistogramColorTable();
        int maxval = 0;
        for( int i = 0; i < m2dColorHistogramLen; ++i ){
            if( m2dColorHistogram[i] > maxval ){ maxval = m2dColorHistogram[i]; }
        }
        if( mHistogramOutData == 0 )
        {
            mHistogramOutData = (uchar*)malloc( sizeof(uchar) * m2dColorHistogramLen * mBPP );
        }
        
        for( int i = 0; i < m2dColorHistogramLen; ++i )
        {
            mHistogramOutData[i*mBPP + 0] = (unsigned char)((int)(mHistogramColors[i*mBPP + 0]*((float)m2dColorHistogram[i]) / ((float)maxval) ));
            mHistogramOutData[i*mBPP + 1] = (unsigned char)((int)(mHistogramColors[i*mBPP + 1]*((float)m2dColorHistogram[i]) / ((float)maxval) ));
            mHistogramOutData[i*mBPP + 2] = (unsigned char)((int)(mHistogramColors[i*mBPP + 2]*((float)m2dColorHistogram[i]) / ((float)maxval) ));
        }
        U::p("make image");
        free(mHistogramColors);mHistogramColors = 0;
        Mat image( TWODIM_HISTOGRAM_WIDTH, TWODIM_HISTOGRAM_WIDTH, mType, mHistogramOutData );
        return image;
    }
    Mat getHistogramColorTable()
    {
        //Initialize the histogram colors, just for show and fun
        if( mHistogramColors == 0 )
        {
            mHistogramColors = (uchar*)malloc(sizeof(uchar) * m2dColorHistogramLen * mBPP );
            float increment = 1.0 / TWODIM_HISTOGRAM_WIDTH;
            for( float r = 0.0; r < 1.0; r += increment )
            {
                for( float g = 0.0; g < 1.0; g += increment )
                {
                    for( float b = 0.0; b < 1.0; b += increment )
                    {
                        Vec3f col( r, g, b);
                        //normalize
                        col = normalize(col);
                        //now project to the r-g plane
                        int x = (int)(col[0] * ((float)TWODIM_HISTOGRAM_WIDTH));
                        int y = (int)(col[1] * ((float)TWODIM_HISTOGRAM_WIDTH));
                        int imgindex = x+y*TWODIM_HISTOGRAM_WIDTH;
                        mHistogramColors[imgindex*mBPP + 0] = (uchar)((int)(255.0 * r));
                        mHistogramColors[imgindex*mBPP + 1] = (uchar)((int)(255.0 * g));
                        mHistogramColors[imgindex*mBPP + 2] = (uchar)((int)(255.0 * b));
                    }
                }
            }
        }//end histogram colors
        
        if( mHistogramOutData == 0 )
        {
            mHistogramOutData = (uchar*)malloc( sizeof(uchar) * m2dColorHistogramLen * mBPP );
        }
        for( int i = 0; i < m2dColorHistogramLen; ++i )
        {
            mHistogramOutData[i*mBPP + 0] = mHistogramColors[i*mBPP+0];
            mHistogramOutData[i*mBPP + 1] = mHistogramColors[i*mBPP+1];
            mHistogramOutData[i*mBPP + 2] = mHistogramColors[i*mBPP+2];
        }
        Mat image( TWODIM_HISTOGRAM_WIDTH, TWODIM_HISTOGRAM_WIDTH, mType, mHistogramOutData );
        return image;
    }
    
    void collect2dHistogram()
    {
        
        if( m2dColorHistogram == 0 || !mImageReady )
        {
            m2dColorHistogramLen = TWODIM_HISTOGRAM_WIDTH * TWODIM_HISTOGRAM_WIDTH;
            if( m2dColorHistogram == 0){m2dColorHistogram = (int*)malloc( sizeof(int) * m2dColorHistogramLen );}
            for( int i = 0; i < m2dColorHistogramLen; ++i ){ m2dColorHistogram[i] = 0; }
            getImage();
            //Clear first
            for( int i = 0; i < mLen; ++i )
            {
                Vec3f col( mOutData[i*mBPP + 0], mOutData[i*mBPP + 1], mOutData[i*mBPP + 2]);
                //normalize
                col = normalize(col);
                //now project to the r-g plane
                int x = (int)(col[0] * ((float)TWODIM_HISTOGRAM_WIDTH));
                int y = (int)(col[1] * ((float)TWODIM_HISTOGRAM_WIDTH));
                //Add to the color histogram
                m2dColorHistogram[ x + y * TWODIM_HISTOGRAM_WIDTH ] = m2dColorHistogram[ x + y * TWODIM_HISTOGRAM_WIDTH ] + 1;
            }
        }
    }
    
    void collect3dHistogram()
    {
        if( m3dColorHistogram == 0 || !mImageReady )
        {
            getImage();
            //Clear first
            //Allocate the 3d histogram
            m3dColorHistogramLen = THREEDIM_HISTOGRAM_WIDTH * THREEDIM_HISTOGRAM_WIDTH * THREEDIM_HISTOGRAM_WIDTH;
            if( m3dColorHistogram == 0 ){ m3dColorHistogram = (int*)malloc( sizeof(int) * m3dColorHistogramLen ); }
            for( int i = 0; i < m3dColorHistogramLen; ++i ){m3dColorHistogram[i] = 0;}
            for( int i = 0; i < mLen; ++i )
            {
                Vec3f col( mOutData[i*mBPP + 0], mOutData[i*mBPP + 1], mOutData[i*mBPP + 2]);
                //normalize
                col = normalize(col);
                m3dColorHistogram[ (int)(col[0] + col[1] * THREEDIM_HISTOGRAM_WIDTH + col[2] * THREEDIM_HISTOGRAM_WIDTH * THREEDIM_HISTOGRAM_WIDTH)] = m3dColorHistogram[ (int)(col[0] + col[1] * THREEDIM_HISTOGRAM_WIDTH + col[2] * THREEDIM_HISTOGRAM_WIDTH * THREEDIM_HISTOGRAM_WIDTH)] + 1;
            }
        }
    }
    
    int diff( ImageAverage* aO )
    {
        //Choose a differentiation method
        //return diffByPixels(aO);
        return diffBy2dColorHistogram(aO);
        //return diffBy3dColorHistogram(aO);
    }
    long diffByPixels( ImageAverage* aO )
    {
        long diff = 0;
        for( int i = 0; i < mLen; ++i )
        {
            diff += U::absInt( mR[i] - aO->mR[i] );
            diff += U::absInt( mG[i] - aO->mG[i] );
            diff += U::absInt( mB[i] - aO->mB[i] );
        }
        diff /= 255;
        diff /= 3;
        return diff;
    }
    long diffBy2dColorHistogram( ImageAverage* aO )
    {
        long diff = 0;
        collect2dHistogram();
        aO->collect2dHistogram();
        for( int i = 0; i < m2dColorHistogramLen; ++i )
        {
            diff += U::absInt( m2dColorHistogram[i] - aO->m2dColorHistogram[i] );
        }
        if( mMemorySmall )
        {
        free(m2dColorHistogram);
        m2dColorHistogram = 0;
        }
        return diff;
    }
    long diffBy3dColorHistogram( ImageAverage* aO )
    {
        long diff = 0;
        collect3dHistogram();
        aO->collect3dHistogram();
        for( int i = 0; i < m3dColorHistogramLen; ++i )
        {
            diff += U::absInt( m3dColorHistogram[i] - aO->m3dColorHistogram[i] );
        }
        if( mMemorySmall )
        {
            free(m3dColorHistogram);
            m3dColorHistogram = 0;
        }
        return diff;
    }
    
};
#endif
