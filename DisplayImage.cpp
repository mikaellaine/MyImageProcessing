#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "DisplayImage.h"
#include "cpputils.h"

using namespace cv;

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

    ImageAverage* avg = 0;
    //Go through all the files in the directory:
    std::string fp = argv[1];
    printf("\n Traversing directory %s", fp.c_str());
    if( CppUtils::openDir( fp.c_str() ) )
    {
      while( const char* p = CppUtils::iterDir() )
      {
          std::string filepath(fp.c_str());
          filepath.append("/");
          filepath.append(p);
          printf("\n>> crop: %s", filepath.c_str());
          Mat image;
          image = imread( filepath, 1 );
          if( image.data )
          {
              printf("\ncreate ImageAverage");fflush(stdout);
              if( avg == 0 ){ avg = ImageAverage::create( image ); }
              printf("\nAdding to average");fflush(stdout);
              avg->addAvg( image );
          }
          //crop( filepath.c_str() );
      }
      CppUtils::closeDir();
      if( avg )
      {
          std::string filepath(fp.c_str());
          filepath.append("/");
          filepath.append("average.png");
          
          Mat avgImg = avg->getImage();
          imwrite( filepath, avgImg );
      }
    }
  return 0;
}
