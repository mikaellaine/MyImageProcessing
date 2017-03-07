#ifndef ImageAverage_h
#define ImageAverage_h

#include "cpputils.h"
#include <opencv2/opencv.hpp>
using namespace cv;

//1. introduce all images -> first clusters (each cluster represents one png image)
//2. Create new clusters by taking the average of clusters that are close by. Require clusters don't differ more than DM * avg diff (calculate average diff first)
//3. Repeat 2. until the number of clusters is low enough (for terrain images, probably between 4 and 10 (for seasonal groups)


#define COL_PACK_WIDTH 100

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
    uchar* mHistogramOutData;
    static uchar* mHistogramColors;//colors represented by the histogram locations
    int* mNormalizedColor;//pack the color into a triangle
    int mNormalizedColorLen;
    bool mImageReady;
    
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
        extNearbyCluster = 0;
        mHistogramOutData = 0;
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
        mNormalizedColorLen = COL_PACK_WIDTH * COL_PACK_WIDTH;
        mNormalizedColor = (int*)malloc( sizeof(int) * mNormalizedColorLen );
        for( int i = 0; i < mNormalizedColorLen; ++i )
        {
            mNormalizedColor[i] = 0;
        }
    }
    ~ImageAverage()
    {
        if( mOutData ){ free(mOutData); }
        if( mHistogramOutData ){ free(mHistogramOutData); }
        free(mR);
        free(mG);
        free(mB);
        free(mNormalizedColor);
    }
    //Assumes the dimensions are the same
    void addAvg( Mat& aImg )
    {
        //U::p("addAvg");
        mImageReady = false;//because it just changed
        if( mOutData == 0 )
        {
            mType = aImg.type();
            mBPP = aImg.elemSize();
            mOutData = (uchar*)malloc( sizeof(uchar) * mLen * mBPP );
            if( mHistogramColors == 0 )
            {
                mHistogramColors = (uchar*)malloc(sizeof(uchar) * mNormalizedColorLen * mBPP );
                float increment = 1.0 / COL_PACK_WIDTH;
                for( float r = 0; r < 1.0; r += increment )
                {
                    for( float g = 0; g < 1.0; g += increment )
                    {
                        for( float b = 0; b < 1.0; b += increment )
                        {
                            //now project to the r-g plane
                            int x = (int)(r * ((float)COL_PACK_WIDTH));
                            int y = (int)(g * ((float)COL_PACK_WIDTH));
                            int imgindex = x+y*COL_PACK_WIDTH;
                            mHistogramColors[imgindex*mBPP + 0] = (uchar)((int)(255.0 * r));
                            mHistogramColors[imgindex*mBPP + 1] = (uchar)((int)(255.0 * g));
                            mHistogramColors[imgindex*mBPP + 2] = (uchar)((int)(255.0 * b));
                        }
                    }
                }
            }
        }
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
        if( !mHistogramOutData )
        {
            mHistogramOutData = (uchar*)malloc( sizeof(uchar) * mNormalizedColorLen * mBPP );
        }
        int maxval = 0;
        for( int i = 0; i < mNormalizedColorLen; ++i ){
            if( mNormalizedColor[i] > maxval ){ maxval = mNormalizedColor[i]; }
        }
        for( int i = 0; i < mNormalizedColorLen; ++i )
        {
            mHistogramOutData[i*mBPP + 0] = (unsigned char)((int)(mHistogramColors[i*mBPP+0]*((float)mNormalizedColor[i]) / ((float)maxval) ));
            mHistogramOutData[i*mBPP + 1] = (unsigned char)((int)(mHistogramColors[i*mBPP+1]*((float)mNormalizedColor[i]) / ((float)maxval) ));
            mHistogramOutData[i*mBPP + 2] = (unsigned char)((int)(mHistogramColors[i*mBPP+2]*((float)mNormalizedColor[i]) / ((float)maxval) ));
        }
        Mat image( COL_PACK_WIDTH, COL_PACK_WIDTH, mType, mHistogramOutData );
        return image;
    }
    Mat getHistogramColorTable()
    {
        for( int i = 0; i < mNormalizedColorLen; ++i )
        {
            mHistogramOutData[i*mBPP + 0] = mHistogramColors[i*mBPP+0];
            mHistogramOutData[i*mBPP + 1] = mHistogramColors[i*mBPP+1];
            mHistogramOutData[i*mBPP + 2] = mHistogramColors[i*mBPP+2];
        }
        Mat image( COL_PACK_WIDTH, COL_PACK_WIDTH, mType, mHistogramOutData );
        return image;
    }
    
    
    void packColors()
    {
        getImage();
        //Clear first
        for( int i = 0; i < mNormalizedColorLen; ++i ){mNormalizedColor[i] = 0;}
        for( int i = 0; i < mLen; ++i )
        {
            Vec3f col( mOutData[i*mBPP + 0], mOutData[i*mBPP + 1], mOutData[i*mBPP + 2]);
            //normalize
            col = normalize(col);
            
            //now project to the r-g plane
            int x = (int)(col[0] * ((float)COL_PACK_WIDTH));
            int y = (int)(col[1] * ((float)COL_PACK_WIDTH));
            //Add to the color histogram
            mNormalizedColor[ x + y * COL_PACK_WIDTH ] = mNormalizedColor[ x + y * COL_PACK_WIDTH ] + 1;
        }
    }
    
    int diff( ImageAverage* aO )
    {
        //Choose a differentiation method
        //return diffByPixels(aO);
        return diffByColorHistogram(aO);
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
    long diffByColorHistogram( ImageAverage* aO )
    {
        long diff = 0;
        for( int i = 0; i < mNormalizedColorLen; ++i )
        {
            diff += U::absInt( mNormalizedColor[i] - aO->mNormalizedColor[i] );
        }
        return diff;
    }
};
#endif
