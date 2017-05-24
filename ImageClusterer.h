
#ifndef ImageClusterer_h
#define ImageClusterer_h
#define BIG_DIST 1000000000.0f
class ImageAverage;//forward declaration

class ImageClusterer
{
private:
    std::vector<ImageAverage*> mClusters;
    float mAvgMinDist;//average minimum distance
public:
    ImageClusterer(){}
    bool addCluster(const char* path);
    int clusterCount(){return mClusters.size();}
    void mergeClusters( float aMaxDist );
    float minDist( ImageAverage* aCluster, ImageAverage** aMinCluster );//get the closest other cluster and dist to it
    void clearClusters();
    void outputClusterImagesToFilesystem();
    void outputClusterColorTable();
    void outputNormalized( const char* aOutputDirPath );
};
#endif
