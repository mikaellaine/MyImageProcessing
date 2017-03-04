#include <opencv2/opencv.hpp>
using namespace cv;

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
public:
    static ImageAverage* create( const Mat& aImg )
    {
        return new ImageAverage( aImg.cols, aImg.rows, aImg.type() );
    }
    ImageAverage( int aW, int aH, int aType ){
        mType = aType;
        mSumMembers = 0;
        mW = aW;
        mH = aH;
        mLen = aW * aH;
        mR = (double*)malloc( sizeof(double)*mLen );
        mG = (double*)malloc( sizeof(double)*mLen );
        mB = (double*)malloc( sizeof(double)*mLen );
        
        for( int i = 0; i < mLen; ++i )
        {
            mR[i] = 0;
            mG[i] = 0;
            mB[i] = 0;
        }
        
    }
    //Assumes the dimensions are the same
    void addAvg( Mat& aImg )
    {
        mType = aImg.type();
        mBPP = aImg.elemSize();
        printf("\naddAvg");fflush(stdout);
        for( int i = 0; i < mLen; ++i )
        {
            mR[i] += ((uchar*)aImg.ptr())[i*mBPP+0];
            mG[i] += ((uchar*)aImg.ptr())[i*mBPP+1];
            mB[i] += ((uchar*)aImg.ptr())[i*mBPP+2];
        }
        mSumMembers += 1.0;
    }
    
    Mat getImage()
    {
        printf("\ngetImage()");
        printf("\nImage format: %d x %d BPP: %d", mW, mH, mBPP);
        fflush(stdout);
        uchar* data = (uchar*)malloc( sizeof(uchar) * mLen * mBPP );
        
        for( int i = 0; i < mLen; ++i )
        {
            data[i*mBPP + 0] = (unsigned char)((int)(mR[i] / mSumMembers ));
            data[i*mBPP + 1] = (unsigned char)((int)(mG[i] / mSumMembers ));
            data[i*mBPP + 2] = (unsigned char)((int)(mB[i] / mSumMembers ));
        }
        
        Mat image( mH, mW, mType, data );
        return image;
    }
};
