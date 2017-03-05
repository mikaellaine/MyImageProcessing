#ifndef ImageAverage_h
#define ImageAverage_h

#include "cpputils.h"
#include <opencv2/opencv.hpp>
using namespace cv;

//1. introduce all images -> first clusters (each cluster represents one png image)
//2. Create new clusters by taking the average of clusters that are close by. Require clusters don't differ more than DM * avg diff (calculate average diff first)
//3. Repeat 2. until the number of clusters is low enough (for terrain images, probably between 4 and 10 (for seasonal groups)



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
    int* mHistogram;
    bool mImageReady;
public:
    static ImageAverage* create( const Mat& aImg )
    {
        return new ImageAverage( aImg.cols, aImg.rows, aImg.type() );
    }
    ImageAverage( int aW, int aH, int aType ){
        mType = aType;
        mSumMembers = 0;
        mHistogram = 0;
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
        if( mHistogram ){ free(mHistogram); }
        if( mOutData ){ free(mOutData); }
        free(mR);
        free(mG);
        free(mB);
    }
    //Assumes the dimensions are the same
    void addAvg( Mat& aImg )
    {
        printf("\naddAvg");fflush(stdout);
        mImageReady = false;//because it just changed
        if( mOutData == 0 )
        {
            mType = aImg.type();
            mBPP = aImg.elemSize();
            mOutData = (uchar*)malloc( sizeof(uchar) * mLen * mBPP );
        }
        printf("\naddAvg");fflush(stdout);
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
        printf("\ngetImage()");
        printf("\nImage format: %d x %d BPP: %d", mW, mH, mBPP);
        fflush(stdout);
        
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
    int diff( ImageAverage* aO )
    {
        return diffByPixels(aO);
    }
    long diffByPixels( ImageAverage* aO )
    {
        long diff = 0;
        for( int i = 0; i < mLen; ++i )
        {
            diff += CppUtils::absInt( mR[i] - aO->mR[i] );
            diff += CppUtils::absInt( mG[i] - aO->mG[i] );
            diff += CppUtils::absInt( mB[i] - aO->mB[i] );
        }
        diff /= 255;
        diff /= 3;
        return diff;
    }
    long diffByHistogram( ImageAverage* aO )
    {
        return 0;
    }
};
#endif
