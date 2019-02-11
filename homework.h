#ifndef HOMEWORK_H
#define HOMEWORK_H

typedef int bool;
enum { false, true };

typedef struct {
    bool color_property;
    int width;
    int height;
    int maxval;
    unsigned char **matrix;
    unsigned char **r;
    unsigned char **g;
    unsigned char **b;
} image;

typedef struct {
    image *in;
    image *out;
    int var;
} package;

void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out, char * filter);

#endif /* HOMEWORK_H */