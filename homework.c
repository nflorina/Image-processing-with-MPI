#include<mpi.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <pthread.h>
#include <math.h>
// .
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

image *in_global;
image *out_global;

void deallocate_dynamic_matrix(unsigned char **matrix, int row)
{
    int i;
    for (i = 0; i < row; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

void readInput(const char * fileName, image *img) {
    char* first_line = (char *)malloc(sizeof(char));
    char* width = (char *)malloc(sizeof(char));
    char* height = (char *)malloc(sizeof(char));
    char* maxval = (char *)malloc(sizeof(char));
    FILE* fp;
    fp = fopen (fileName, "rb");
    if (fp == NULL) {
        perror("The file can't be opened!");
    }
    fseek(fp, 0, SEEK_SET);
    fscanf(fp, "%s\n%s %s\n%s\n",first_line, width, height, maxval);
    int row = atoi(height);
    int column = atoi(width);
    
    img->width = column;
    img->height = row;
    img->maxval = atoi(maxval);

    int size = column;
    int i = 0, j;
    int color = 1;
    if (strcmp("P6", first_line) == 0) {
        img->color_property = true;
        size *= 3;
        color *= 3;
        img->r = calloc(img->height, sizeof(unsigned char*));
        img->g = calloc(img->height, sizeof(unsigned char*));
        img->b = calloc(img->height, sizeof(unsigned char*));
        for (i = 0; i < img->height; i++) {
            img->r[i] = calloc(img->width, sizeof(unsigned char));
            img->g[i] = calloc(img->width, sizeof(unsigned char));
            img->b[i] = calloc(img->width, sizeof(unsigned char));
        }
    } else {
        img->color_property = false;
        img->matrix = calloc(img->height, sizeof(unsigned char*));
        for (i = 0; i < img->height; i++) {
            img->matrix[i] = calloc(img->width, sizeof(unsigned char));
        }
    }
    unsigned char buffer[size];
    for (i = 0; i < row; i++) {
        fread(buffer, sizeof(unsigned char) * color, column, fp);
        for (j = 0; j < column; j++) {
            if (img->color_property == false) {
                img->matrix[i][j] = buffer[j];
            } else {
                    img->r[i][j] = buffer[j * color];
                    img->g[i][j] = buffer[j * color + 1];
                    img->b[i][j] = buffer[j * color + 2];
                }
            }
    }

    fclose(fp);
    free(first_line);
    free(width);
    free(height);
    free(maxval);
}

void writeData(const char * fileName, image *img) {
    FILE *fp;
    int i, j;
    fp = fopen(fileName, "wb");
    if (fp == NULL) {
        perror("The file can't be opened!");
    }
    if (img->color_property == true) {
        fprintf(fp, "P6\n%d %d\n%d\n",
            img->width, img->height, img->maxval);
    } else {
        fprintf(fp, "P5\n%d %d\n%d\n",
            img->width, img->height, img->maxval);
    }
    if (img->color_property == false) {
        for(i = 0; i < img->height; i++) {
            fwrite(img->matrix[i], sizeof(unsigned char),
                img->width, fp);
        }
    } else {
        for (i = 0; i < img->height; i++) {
            unsigned char row[img->width * 3];
            for (j = 0; j < img->width; j++) {
                row[j*3] = img->r[i][j];
                row[j*3+1] = img->g[i][j];
                row[j*3+2] = img->b[i][j];
            }
            fwrite(row, sizeof(unsigned char), img->width*3, fp);
        }
    }
    fclose(fp);
    if (img->color_property == true) {
        deallocate_dynamic_matrix(img->r, img->height);
        deallocate_dynamic_matrix(img->g, img->height);
        deallocate_dynamic_matrix(img->b, img->height);
    } else {
        deallocate_dynamic_matrix(img->matrix, img->height);
    }
}



void resize(image *in, image * out, char * filter) {
    out->height = in->height;
    out->width = in->width;
    out->color_property = in->color_property;
    out->maxval = in->maxval;

    int i, j;
    if (in->color_property == false) {
    out->matrix = calloc(out->height, sizeof(unsigned char*));
    for (i = 0; i < out->height; i++) {
        out->matrix[i] = calloc(out->width, sizeof(unsigned char));
        }
    } else {
    out->r = calloc(out->height, sizeof(unsigned char*));
    for (i = 0; i < out->height; i++) {
        out->r[i] = calloc(out->width, sizeof(unsigned char));
    }
    out->g = calloc(out->height, sizeof(unsigned char*));
    for (i = 0; i < out->height; i++) {
        out->g[i] = calloc(out->width, sizeof(unsigned char));
    }
    out->b = calloc(out->height, sizeof(unsigned char*));
    for (i = 0; i < out->height; i++) {
        out->b[i] = calloc(out->width, sizeof(unsigned char));
        }
    }

    float matrix_smooth[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    float smooth_imp = 9;

    float matrix_blur[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    float blur_imp = 16;

    float matrix_sharpen[3][3] = {{0, -2, 0}, {-2, 11, -2}, {0, -2, 0}};
    float sharpen_imp = 3;

    float matrix_mean[3][3] = {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}};
    float mean_imp = 1;

    float matrix_emboss[3][3] = {{0, 1, 0}, {0, 0, 0}, {0, -1, 0}};
    float emboss_imp = 1;

    float matrix[3][3] = {{0}};
    
    if (strcmp("smooth", filter) == 0) {
        for (i = 0; i < 3; i ++) {
            for (j = 0; j < 3; j++) {
                matrix[i][j] = matrix_smooth[i][j]/smooth_imp;
            }
        }
    } else if(strcmp("blur", filter) == 0) {
        for (i = 0; i < 3; i ++) {
            for (j = 0; j < 3; j++) {
                matrix[i][j] = matrix_blur[i][j]/blur_imp;
            }
        }
    } else if(strcmp("sharpen", filter) == 0) {
        for (i = 0; i < 3; i ++) {
            for (j = 0; j < 3; j++) {
                matrix[i][j] = matrix_sharpen[i][j]/sharpen_imp;
            }
        }
    } else if(strcmp("mean", filter) == 0) {
        for (i = 0; i < 3; i ++) {
            for (j = 0; j < 3; j++) {
                matrix[i][j] = matrix_mean[i][j]/mean_imp;
            }
        }
    } else if(strcmp("emboss", filter) == 0) {
        for (i = 0; i < 3; i ++) {
            for (j = 0; j < 3; j++) {
                matrix[i][j] = matrix_emboss[i][j]/emboss_imp;
            }
        }
    } 
    if (in->color_property == false) {
        for (i = 0; i < in->height; i++) {
		    for (j = 0; j < in->width; j++) {
			    if (i == 0 || j == 0 || i == in->height - 1 || j == in->width - 1) {
				    out->matrix[i][j] = in->matrix[i][j];
			    }    else {
				    out->matrix[i][j] = in->matrix[i-1][j-1] * matrix[0][0] +
                                    in->matrix[i-1][j] * matrix[0][1] + 
						            in->matrix[i-1][j+1] * matrix[0][2] +
                                    in->matrix[i][j-1] * matrix[1][0] +
						            in->matrix[i][j] * matrix[1][1] +
                                    in->matrix[i][j+1] * matrix[1][2] + 
						            in->matrix[i+1][j-1] * matrix[2][0] +
                                    in->matrix[i+1][j] * matrix[2][1] + 
						            in->matrix[i+1][j+1] * matrix[2][2];
			    }
		    }
	    }
    } else {
         for (i = 0; i < in->height; i++) {
	 	    for (j = 0; j < in->width; j++) {
                if (i == 0 || i == in->height - 1) {
	 			    out->r[i][j] = in->r[i][j];
                    out->g[i][j] = in->g[i][j];
                    out->b[i][j] = in->b[i][j];
	 		    } else  if (j == 0) {
                    out->r[i][j] = in->r[i][j];
                    out->g[i][j] = in->g[i][j];
                    out->b[i][j] = in->b[i][j];
                 } else if (j == in->width - 1) {
                    out->r[i][j] = in->r[i][j];
                    out->g[i][j] = in->g[i][j];
                    out->b[i][j] = in->b[i][j];
                 } else {
	 			    out->r[i][j] = in->r[i-1][j-1] * matrix[0][0] +
                                     in->r[i-1][j] * matrix[0][1] + 
	 					            in->r[i-1][j+1] * matrix[0][2] +
                                     in->r[i][j-1] * matrix[1][0] +
	 					            in->r[i][j] * matrix[1][1] +
                                     in->r[i][j+1] * matrix[1][2] + 
	 					            in->r[i+1][j-1] * matrix[2][0] +
                                     in->r[i+1][j] * matrix[2][1] + 
	 					            in->r[i+1][j+1] * matrix[2][2];
                    out->g[i][j] = in->g[i-1][j-1] * matrix[0][0] +
                                     in->g[i-1][j] * matrix[0][1] + 
	 					            in->g[i-1][j+1] * matrix[0][2] +
                                     in->g[i][j-1] * matrix[1][0] +
	 					            in->g[i][j] * matrix[1][1] +
                                     in->g[i][j+1] * matrix[1][2] + 
	 					            in->g[i+1][j-1] * matrix[2][0] +
                                     in->g[i+1][j] * matrix[2][1] + 
	 					            in->g[i+1][j+1] * matrix[2][2];
                    out->b[i][j] = in->b[i-1][j-1] * matrix[0][0] +
                                     in->b[i-1][j] * matrix[0][1] + 
	 					            in->b[i-1][j+1] * matrix[0][2] +
                                     in->b[i][j-1] * matrix[1][0] +
	 					            in->b[i][j] * matrix[1][1] +
                                     in->b[i][j+1] * matrix[1][2] + 
	 					            in->b[i+1][j-1] * matrix[2][0] +
                                     in->b[i+1][j] * matrix[2][1] + 
	 					            in->b[i+1][j+1] * matrix[2][2];
	 		    }
	 	    }
	     }
     }
}


int main(int argc, char * argv[])
{
image input;
image output;

int rank;
int nProcesses;
MPI_Init(&argc, &argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

if (rank == 0) {
	readInput(argv[1], &input);
	int i;
	for (i = 3; i < argc; i++) {
		resize(&input, &output, argv[i]);
		input = output;
	}
	writeData(argv[2], &input);
}
MPI_Finalize();
return 0;
}