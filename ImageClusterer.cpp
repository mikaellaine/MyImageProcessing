#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "ImageClusterer.h"
#include "ImageAverage.h"
#include "cpputils.h"

using namespace cv;

bool ImageClusterer::addCluster(const char* path)
{
    Mat image;
    image = imread( path, 1 );
    if ( !image.data )
    {
        printf("\nNo image data \n");
        return false;
    }
    ImageAverage* cluster = ImageAverage::create( image );
    cluster->addAvg( image );
    mClusters.push_back(cluster);
    return true;
}

//@param aMaxDist: how many times the average distance can the distance within a cluster be
//If the closest cluster's distance > aMaxDist*avgMinDist, the cluster won't be  merged
void ImageClusterer::mergeClusters( float aMaxDist )
{
    U::p("mergeClusters");
    double avgmindist = 0.0;
    float avgcnt = 0;
    ImageAverage* a;
    for( int i = 0; i < mClusters.size(); ++i )
    {
        printf("\ncluster #%d", i);fflush(stdout);
        a = mClusters[i];
        ImageAverage* avg;
        float dist = (float)minDist( a, &avg );
        a->extNearbyClusterDist = dist;
        a->extNearbyCluster = avg;
        a->extId = i;
        avgmindist += dist;
        avgcnt += 1.0f;
    }
    mAvgMinDist = avgmindist / ((double)avgcnt);
    float require_dist = aMaxDist * mAvgMinDist;
    printf( "\nmergeClusters: average minimum distance is: %g", avgmindist );fflush(stdout);

    printf("\nMerging clusters...");fflush(stdout);
    //Collect the new clusters
    std::vector<ImageAverage*> newClusters;
    for( int i = 0; i < mClusters.size(); ++i )
    {
        //printf("\n%d:%s",i,( a->extNearbyCluster != 0 ? "not assigned": "assigned"));
        a = mClusters[i];
        if( a->extNearbyCluster != 0 )
        {
            //if the nearest is NOT already assigned to a new cluster and the distance is not too great..
            if( a->extNearbyCluster->extNearbyCluster != 0 &&
                a->extNearbyClusterDist < require_dist )
            {
                printf("\nJOIN: %d, %d", a->extId, a->extNearbyCluster->extId);
                //Merge clusters
                Mat aimg = a->getImage();
                Mat bimg = a->extNearbyCluster->getImage();
                ImageAverage* newCluster = ImageAverage::create( aimg );
                newCluster->addAvg( aimg );
                newCluster->addAvg( bimg );
                a->extNearbyCluster->extNearbyCluster = 0;
                a->extNearbyCluster = 0;
                newClusters.push_back(newCluster);
            }
            else
            {
                printf("\nJOIN: %d", a->extId);
                //Create a single-member cluster
                Mat aimg = a->getImage();
                ImageAverage* newCluster = ImageAverage::create( aimg );
                newCluster->addAvg( aimg );
                a->extNearbyCluster->extNearbyCluster = 0;
                a->extNearbyCluster = 0;
                newClusters.push_back(newCluster);
            }
        }
    }
    //Clear the old clusters
    clearClusters();
    
    //Use the new clusters
    mClusters = newClusters;
}

float ImageClusterer::minDist( ImageAverage* aCluster, ImageAverage** aMinCluster )
{
    float mindist = BIG_DIST;
    float dist;
    ImageAverage *a;
    for( int i = 0; i < mClusters.size(); ++i )
    {
        a = mClusters[i];
        if( aCluster != a )
        {
            dist = (float)aCluster->diff(a);
            if( dist < mindist )
            {
                (*aMinCluster) = a;
                mindist = dist;
            }
        }
    }
    printf( "\nMindiff: %g", mindist );
    return mindist;
}

void ImageClusterer::clearClusters()
{
    ImageAverage *a;
    for( int i = 0; i < mClusters.size(); ++i )
    {
        a = mClusters[i];
        delete a;
    }
    mClusters.clear();
}

void ImageClusterer::outputClusterImagesToFilesystem()
{
    ImageAverage *a;
    system("mkdir ClusterImages");
    system("rm -r -f ClusterImages/*");
    for( int i = 0; i < mClusters.size(); ++i )
    {
        a = mClusters[i];
        std::string filepath("ClusterImages/");
        filepath.append(std::to_string(i));
        filepath.append(".png");
        printf("\nProcess: %s", filepath.c_str());
        imwrite(filepath, mClusters[i]->getImage());
    }
}


void crop( const char* path )
{
    Mat image;
    image = imread( path, 1 );
    if ( !image.data )
    {
        printf("\nNo image data \n");
        return;
    }
    
    Rect croprect(950, 150, 1470, 1140);
    Mat image_cropped = image(croprect);
    image_cropped.copyTo(image);
    imwrite(path, image);
}

int main(int argc, char** argv )
{
  if ( argc != 2 )
    {
      printf("usage: DisplayImage.out <Image_Path>\n");
      return -1;
    }
    ImageClusterer* clusters = new ImageClusterer();

    ImageAverage* avg = 0;
    //Go through all the files in the directory:
    std::string fp = argv[1];
    printf("\n Traversing directory %s", fp.c_str());
    if( U::openDir( fp.c_str() ) )
    {
      while( const char* p = U::iterDir() )
      {
          std::string filepath(fp.c_str());
          filepath.append("/");
          filepath.append(p);
          printf("\nProcess: %s", filepath.c_str());
          clusters->addCluster( filepath.c_str());
      }
        printf("\nNow we have %d clusters", clusters->clusterCount());
        clusters->mergeClusters(2.0f);
        printf("\nNow we have %d clusters", clusters->clusterCount());
        clusters->mergeClusters(2.0f);
        printf("\nNow we have %d clusters", clusters->clusterCount());
        clusters->mergeClusters(2.0f);
        printf("\nNow we have %d clusters", clusters->clusterCount());
        clusters->outputClusterImagesToFilesystem();
        
      U::closeDir();
    }
  return 0;
}
