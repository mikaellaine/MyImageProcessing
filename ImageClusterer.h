
#ifndef ImageClusterer_h
#define ImageClusterer_h
class ImageAverage;//forward declaration

class ImageClusterer
{
private:
    std::vector<ImageAverage*> mClusters;
public:
    ImageClusterer(){}
    bool addCluster(const char* path);
    
};
#endif
