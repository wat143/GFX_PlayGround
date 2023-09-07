#ifndef __IMAGELOADER__
#define __IMAGELOADER__

enum ImageType {
    TYPE_MIN=0,
    RAW,
    TYPE_MAX
};

class ImageLoader {
public:
    ImageLoader(){}
    ~ImageLoader(){}
    char* loadImageFromFile(const char*, int, int, int);
};

#endif
