//
// Created by floodd on 23/03/2022.
//
#include <iostream>
#include <cstring>
#include <vector>
#include <valarray>
#include "Image.h"


bool Image::loadRaw(string filename)
{
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary|std::ios::in); // need to spec. binary & input mode for Windows users
    try {
        if (ifs.fail()) { throw("Can't open input file"); }
        std::string header;
        int w, h, b;
        ifs >> header;
        if (strcmp(header.c_str(), "P6") != 0) throw("Can't read input file");
        ifs >> w >> h >> b;
        this->w = w;
        this->h = h;
        this->pixels = new Rgb[w * h];  // this will throw an exception if bad_alloc
        ifs.ignore(256, '\n'); // skip empty lines if necessary until we get to the binary data

        unsigned char pixel_buffer[3]; // buffer to store one pixel, with three channels red,green and blue

        //My attempt at the Gamma encoding/RAW image load.
        int newR, newG, newB;
        ifs >> w >> h;
        for(int i = 0; i<(w * h); i++)
        {
            ifs >> pixels[i].r >> pixels[i].g >> pixels[i].b;
            this->pixels[i].r = pixels[i].r*255;
            this->pixels[i].g = pixels[i].g*255;
            this->pixels[i].b = pixels[i].b*255;
        }
        for(int i = 0; i<(w * h); i++)
        {
            newR = (pixels[i].r/255)^(int)(1/2.2);
            newG = (pixels[i].g/255)^(int)(1/2.2);
            newB = (pixels[i].b/255)^(int)(1/2.2);
            this->pixels[i].r = newR;
            this->pixels[i].g = newG;
            this->pixels[i].b = newB;
        }
        for(int i = 0; i<(w * h); i++)
        {
            ifs.read(reinterpret_cast<char *>(pixel_buffer), 3);
            this->pixels[i].r = pixel_buffer[0];
            this->pixels[i].g = pixel_buffer[1];
            this->pixels[i].b = pixel_buffer[2];
        }

        ifs.close();
    }
    catch (const char *err) {
        fprintf(stderr, "%s\n", err);
        ifs.close();
    }
    return true;
}

void Image::filterRed()
{
    for(int i=0; i<w*h; i++)
    {
        pixels[i].g = 0;
        pixels[i].b = 0;
    }
}
void Image::filterGreen()
{
    for(int i=0; i<w*h; i++)
    {
        pixels[i].r = 0;
        pixels[i].b = 0;
    }
}
void Image::filterBlue()
{
    for(int i=0; i<w*h; i++)
    {
        pixels[i].r = 0;
        pixels[i].g = 0;
    }
}
void Image::greyScale()
{
    for(int i=0; i<w*h; i++)
    {
        auto average = (pixels[i].r + pixels[i].g + pixels[i].b)/3;

        pixels[i].r = average;
        pixels[i].g = average;
        pixels[i].b = average;
    }
}

void Image::flipHorizontal()
{
    Rgb temp;
    for(int row=0; row<h; row++) {
        for(int col=0; col<w/2;col++) {
            temp = pixels[row*w+col];
            pixels[row*w+col] = pixels[row*w+(w-1-col)];
            pixels[row*w+(w-1-col)] = temp;
        }
    }
}

void Image::flipVertically()
{
    Rgb temp;
    for(int col=0; col<w; col++) {
        for(int row=0; row<h/2;row++) {
            temp = pixels[col+row*w];
            pixels[col+row*w] = pixels[col+(h-1-row)*w];
            pixels[col+(h-1-row)*w] = temp;
        }
    }
}

void Image::AdditionalFunction3()
{
    //Desaturation function
    //Help from this page => https://stackoverflow.com/questions/13328029/how-to-desaturate-a-color
    double percentage, greyscale;
    for(int i=0; i<w*h; i++)
    {
        percentage = 0.5;
        greyscale = 0.3*pixels[i].r + 0.6*pixels[i].g + 0.1*pixels[i].b;
        pixels[i].r = pixels[i].r + percentage * (greyscale - pixels[i].r);
        pixels[i].g = pixels[i].g + percentage * (greyscale - pixels[i].g);
        pixels[i].b =  pixels[i].b + percentage * (greyscale - pixels[i].b);
    }

    //Originally tried this function:
    //Shrink image - Couldn't solve problem with the extra pixels still displaying after resize.
//    Rgb temp;
//    float scale = 0.8;
//    for(int y = 0; y < h*scale; y++)
//    {
//        int scaleY = (int)(y/scale);
//        for(int x = 0; x < w*scale; x++) {
//            int scaleX = (int) (x / scale);
//            temp = pixels[x + y * w];
//            pixels[x + y * w] = pixels[scaleX + scaleY * w];
//            pixels[scaleX + scaleY * w] = temp;
//        }
//    }
}

void blur(float k, int width, int height, unsigned char *image)
{
    //blur
    //Help from this video => https://www.youtube.com/watch?v=tvVMLIIG9i0

    //To make symmetrical, had to have two for loops for each direction
    //to blur the image both ways

    //Horizontal blur
    float b[3] = { 0 };
    for(int row=0; row<height; row++){
        for(int col=0; col < width; col++){
            for(int i=0; i < 3; i++){
                b[i] = b[i] + k * (image[3 * (col + row * width) + i] - b[i]);
                image[3 * (col + row * width) + i] = b[i];
            }
        }
    }
    for(int row=0; row<height; row++){
        for(int col=width-1; col >= 0; col--){
            for(int i=0; i < 3; i++){
                b[i] = b[i] + k * (image[3 * (col + row * width) + i] - b[i]);
                image[3 * (col + row * width) + i] = b[i];
            }
        }
    }
    //Vertical blur
    for(int col=0; col < width; col++){
        for(int row=0; row<height; row++){
            for(int i=0; i < 3; i++){
                b[i] = b[i] + k * (image[3 * (col + row * width) + i] - b[i]);
                image[3 * (col + row * width) + i] = b[i];
            }
        }
    }
    for(int col=width-1; col >= 0; col--){
        for(int row=0; row<height; row++){
            for(int i=0; i < 3; i++){
                b[i] = b[i] + k * (image[3 * (col + row * width) + i] - b[i]);
                image[3 * (col + row * width) + i] = b[i];
            }
        }
    }
}

void Image::AdditionalFunction2()
{
    //Calling blur method
    blur(0.1, w, h, reinterpret_cast<unsigned char *>(pixels));
}

void Image::AdditionalFunction1()
{
    //Negative
    //Help from this site => https://dyclassroom.com/image-processing-project/how-to-convert-a-color-image-into-negative
    for(int i=0; i<w*h; i++)
    {
        pixels[i].r = 255 - pixels[i].r;
        pixels[i].g = 255 - pixels[i].g;
        pixels[i].b = 255 - pixels[i].b;
    }
}

void Image::AdvancedFeature()
{
    //Sepia filter
    //Help from this website =>
    //https://www.codeproject.com/Questions/5247617/How-do-I-convert-image-to-grayscale-then-sepia-in

    for(int i=0; i<w*h; i++) {
        if (pixels[i].r < 63) {
            pixels[i].r = (int) (pixels[i].r * 1.1);
            pixels[i].b = (int) (pixels[i].b * 0.9);
        } else if (pixels[i].r < 192) {
            pixels[i].r = (int) (pixels[i].r * 1.15);
            pixels[i].b = (int) (pixels[i].b * 0.85);
        } else {
            pixels[i].r = min(int(pixels[i].r * 1.08), 255);
            pixels[i].b = (int) (pixels[i].b * 0.93);
        }
    }
}

bool Image::savePPM(string filename)
{
    if (this->w == 0 || this->h == 0) { fprintf(stderr, "Can't save an empty image\n"); return true; }
    std::ofstream ofs;
    try {
        ofs.open(filename, std::ios::binary|std::ios::out); // need to spec. binary mode for Windows users
        if (ofs.fail()) throw("Can't open output file");
        ofs << "P6\n" << this->w << " " <<  this->h << "\n255\n";

        unsigned char r, g, b;
        // loop over each pixel in the image, clamp and convert to byte format
        for (int i = 0; i <  this->w *  this->h; ++i) {
//            r = static_cast<unsigned char>(std::min(1.f,  this->pixels[i].r) * 255);
//            g = static_cast<unsigned char>(std::min(1.f,  this->pixels[i].g) * 255);
//            b = static_cast<unsigned char>(std::min(1.f,  this->pixels[i].b) * 255);
//            ofs << r << g << b;

            ofs.write(reinterpret_cast<char *>(&this->pixels[i].r),1);    // write 1 byte;
            ofs.write(reinterpret_cast<char *>(&this->pixels[i].g),1);    // write 1 byte;
            ofs.write(reinterpret_cast<char *>(&this->pixels[i].b),1);    // write 1 byte;
        }
        ofs.close();
    }
    catch (const char *err) {
        fprintf(stderr, "%s\n", err);
        ofs.close();
    }
    return true;
}

bool Image::load(string filename)
{
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary|std::ios::in); // need to spec. binary & input mode for Windows users
    try {
        if (ifs.fail()) { throw("Can't open input file"); }
        std::string header;
        int w, h, b;
        ifs >> header;
        if (strcmp(header.c_str(), "P6") != 0) throw("Can't read input file");
        ifs >> w >> h >> b;
        this->w = w;
        this->h = h;
        this->pixels = new Rgb[w * h];  // this will throw an exception if bad_alloc
        ifs.ignore(256, '\n'); // skip empty lines if necessary until we get to the binary data

        unsigned char pixel_buffer[3]; // buffer to store one pixel, with three channels red,green and blue

        // read each pixel one by one and convert bytes to floats
        for (int i = 0; i < (w * h); ++i) {
            ifs.read(reinterpret_cast<char *>(pixel_buffer), 3);
            this->pixels[i].r = pixel_buffer[0]; // / 255.f;
            this->pixels[i].g = pixel_buffer[1]; // / 255.f;
            this->pixels[i].b = pixel_buffer[2]; // / 255.f;
        }
        ifs.close();
    }
    catch (const char *err) {
        fprintf(stderr, "%s\n", err);
        ifs.close();
    }
    return true;
}

/* Functions used by the GUI - DO NOT MODIFY */
int Image::getWidth()
{
    return w;
}

int Image::getHeight()
{
    return h;
}

Rgb* Image::getImage()
{
    return pixels;
}

void Image::rotate90cw()
{

}