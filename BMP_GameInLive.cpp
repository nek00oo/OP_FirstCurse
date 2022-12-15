#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#pragma pack(1)
typedef struct {
    char ID[2];
    unsigned int SizeFile;
    unsigned char Reserved[4];
    long int PixelOffset;
} BMPHeader;

typedef struct {
    unsigned int HeaderSize;
    unsigned int width;
    unsigned int height;
    unsigned short ColorPlanes;
    unsigned short BitsPerPixel;
    unsigned int Compression;
    unsigned int DataSize;
    unsigned int PWidth;
    unsigned int PHeight;
    unsigned int ColorsCount;
    unsigned int ImportantColorsCount;
} DIBHeader;

typedef struct {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} Pixel;

typedef struct {
    BMPHeader BMPH;
    DIBHeader DIBH;
    Pixel** pixels;
} BMPFile;
#pragma pop

void swap(uint8_t*** Pattern, uint8_t*** PatternTwo){
    uint8_t **tmp = *Pattern;
    *Pattern = *PatternTwo;
    *PatternTwo = tmp;
}

uint8_t** Matrix(int width, int height){
    uint8_t** matrix;
    matrix = (uint8_t**) malloc(sizeof(uint8_t*) * height );
    for (int i = 0; i < height; ++i) {
        matrix[i] = (uint8_t*) malloc(sizeof(uint8_t) * width);
    }
    return matrix;
}

BMPFile* LoadBMPFile(char *FileName){
    FILE *fileBmp = fopen(FileName,"r");

    if (!fileBmp){
        printf("Can't load file %s \n",FileName);
        exit(0);
    }

    BMPFile *bmp_file = (BMPFile*)malloc(sizeof(BMPFile));

    fread(&bmp_file->BMPH,sizeof(BMPHeader),1,fileBmp);
    fread(&bmp_file->DIBH,sizeof(DIBHeader),1,fileBmp);

    bmp_file->pixels = (Pixel**)malloc(sizeof(Pixel*)*bmp_file->DIBH.height);
    for(int i = 0; i < bmp_file->DIBH.height; i++){
        bmp_file->pixels[i] = (Pixel*)malloc(sizeof(Pixel)*bmp_file->DIBH.width);
    }

    fseek(fileBmp,bmp_file->BMPH.PixelOffset,SEEK_SET);
    for(int i = 0; i < bmp_file->DIBH.height; i++){
        for(int j = 0; j < bmp_file->DIBH.width; j++){
            fread(&bmp_file->pixels[i][j],sizeof(Pixel),1,fileBmp);
        }
        unsigned int trash;
        fread(&trash,1,bmp_file->DIBH.width%4,fileBmp);
    }

    fclose(fileBmp);
    return bmp_file;

}

void PrintDataBMP(BMPFile* bmp_file){
    printf("ID[2]:%c%c \nfile size:%d\npixeloffset:%ld\n",bmp_file->BMPH.ID[0],bmp_file->BMPH.ID[1],bmp_file->BMPH.SizeFile,bmp_file->BMPH.PixelOffset);
    printf("HeaderSize = %d\n"
           "    width = %d\n"
           "    height = %d\n"
           "    ColorPlanes = %d\n"
           "    BitsPerPixel = %d\n"
           "    Compression = %d\n"
           "    DataSize = %d\n"
           "    PWidth = %d\n"
           "    PHeight = %d\n"
           "    ColorsCount = %d\n"
           "    ImportantColorsCount = %d\n",
           bmp_file->DIBH.HeaderSize,
           bmp_file->DIBH.width,
           bmp_file->DIBH.height,
           bmp_file->DIBH.ColorPlanes,
           bmp_file->DIBH.BitsPerPixel,
           bmp_file->DIBH.Compression,
           bmp_file->DIBH.DataSize,
           bmp_file->DIBH.PWidth,
           bmp_file->DIBH.PHeight,
           bmp_file->DIBH.ColorsCount,
           bmp_file->DIBH.ImportantColorsCount);
}

void PrintDataPixels(BMPFile* bmp_file){
    for(int i = 0; i < bmp_file->DIBH.height;i++){
        for(int j = 0; j < bmp_file->DIBH.width; j++){
            printf("r-%d g-%d b-%d\n",bmp_file->pixels[i][j].red,bmp_file->pixels[i][j].green,bmp_file->pixels[i][j].blue);
        }
    }
}

void MakeFileBMP(int CounterIteration,BMPFile* bmp_file, uint8_t*** pattern_two,char *OutputDir){
    uint8_t** PatternTwo = *pattern_two;
    char NameFile[100];
    char Number[50];
    strcpy(NameFile,OutputDir);
    strcat(NameFile,"BMP");
    sprintf(Number,"%d",CounterIteration);
    strcat(NameFile,Number);
    strcat(NameFile,".bmp");

    FILE *IterBMPFile = fopen(NameFile,"wb");
    fputc('B',IterBMPFile);
    fputc('M',IterBMPFile);

    uint32_t TmpInt32 = (int32_t)bmp_file->BMPH.SizeFile;

    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);
    fputc(0,IterBMPFile);
    fputc(0,IterBMPFile);
    fputc(0,IterBMPFile);
    fputc(0,IterBMPFile);

    TmpInt32 = (int32_t)bmp_file->BMPH.PixelOffset;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);

    TmpInt32 = (uint32_t)bmp_file->DIBH.HeaderSize;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);

    TmpInt32 = (int32_t)bmp_file->DIBH.width;
    fwrite(&TmpInt32, sizeof(int32_t),1,IterBMPFile);
    TmpInt32 = (int32_t)bmp_file->DIBH.height;
    fwrite(&TmpInt32, sizeof(int32_t),1,IterBMPFile);

    uint16_t TmpInt16 = (uint16_t)bmp_file->DIBH.ColorPlanes;
    fwrite(&TmpInt16, sizeof(uint16_t),1,IterBMPFile);
    TmpInt16 = (uint16_t)bmp_file->DIBH.BitsPerPixel;
    fwrite(&TmpInt16, sizeof(uint16_t),1,IterBMPFile);
    TmpInt32 = (uint32_t)bmp_file->DIBH.Compression;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);
    TmpInt32 = (uint32_t)bmp_file->DIBH.DataSize;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);
    TmpInt32 = (uint32_t)bmp_file->DIBH.PWidth;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);
    TmpInt32 = (uint32_t)bmp_file->DIBH.PHeight;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);
    TmpInt32 = (uint32_t)bmp_file->DIBH.ColorsCount;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);
    TmpInt32 = (uint32_t)bmp_file->DIBH.ImportantColorsCount;
    fwrite(&TmpInt32, sizeof(uint32_t),1,IterBMPFile);

    Pixel* pixels;
    pixels = (Pixel*) malloc(bmp_file->DIBH.height*bmp_file->DIBH.width * sizeof(Pixel));

    for (int y = 0; y < bmp_file->DIBH.height; ++y) {
        for (int x = 0; x < bmp_file->DIBH.width; ++x){
            Pixel pixel = {255,255,255};
            if(PatternTwo[y][x] == 1){
                pixel.red = 0;
                pixel.green = 0;
                pixel.blue = 0;
            }
            pixels[y*bmp_file->DIBH.width+x] = pixel;
        }
    }


    for (int y = 0; y < bmp_file->DIBH.height; ++y) {
        for (int x = 0; x < bmp_file->DIBH.width; ++x) {
            fwrite(&pixels[y*bmp_file->DIBH.width+x],3,1,IterBMPFile);
        }
        fwrite("0",1,bmp_file->DIBH.width % 4,IterBMPFile);
    }

    fclose(IterBMPFile);

}


int main(int argc, char* argv[]){

    if (argc < 4){
        printf("You haven't entered all the required arguments");
        return 0;
    }

    BMPFile* bmp_file;

    int flag_iter = 0;
    int max_iter;
    int freq = 1;
    char OutputDir[150];
    int input_flag = 0;
    int output_flag = 0;

    for (int i = 1; i < argc; i++){
        if (!strcmp(argv[i], "--input")) {
            printf("Information about the first file:\n");
            bmp_file = LoadBMPFile(argv[i + 1]);
            PrintDataBMP(bmp_file);
            input_flag = 1;
        }
        if (!strcmp(argv[i], "--output")) {
            strcpy(OutputDir, argv[i + 1]);
            output_flag = 1;
        }
        if (!strcmp(argv[i], "--max_iter")) {
            sscanf(argv[i+1],"%d",&max_iter);
            flag_iter = 1;
        }
        if (!strcmp(argv[i], "--dump_freq")) {
            sscanf(argv[i+1],"%d",&freq);
        }
    }

    if (!input_flag || !output_flag){
        printf("Check if you have passed the required arguments: --input Filename.bmp and --output Directory");
        return 0;
    }

    int height = bmp_file->DIBH.height;
    int width = bmp_file->DIBH.width;

    uint8_t** pattern = Matrix(width,height);

    for(int y = 0; y < height; y++){
        for(int x = 0; x < width;x++ ){
            if (bmp_file->pixels[y][x].red == 0 && bmp_file->pixels[y][x].green == 0 && bmp_file->pixels[y][x].blue == 0){
                pattern[y][x] = 1;
            }
            else {
                pattern[y][x] = 0;
            }
        }
    }

    uint8_t** pattern_two = Matrix(width,height);

    int CounterIteration = 0;
    while ((flag_iter && CounterIteration != max_iter) || flag_iter == 0){
        CounterIteration++;
        for(int y = 0; y < height; y++){
            for (int x = 0; x < width; x ++){
                int counter = 0;
                if ((x > 0) && pattern[y][x-1]) counter++;
                if ((y > 0) && pattern[y-1][x]) counter++;
                if ((x+1 < width) && pattern[y][x+1]) counter++;
                if ((y+1 < height) && pattern[y+1][x]) counter++;
                if ((x+1 < width && y+1 < height) && pattern[y+1][x+1]) counter++;
                if ((x > 0 && y > 0) && pattern[y-1][x-1]) counter++;
                if ((x > 0 && y+1 < height) && pattern[y+1][x-1]) counter++;
                if ((x+1 < width && y > 0) && pattern[y-1][x+1]) counter++;

                if (pattern[y][x] && (counter < 2 || counter > 3))
                    pattern_two[y][x] = 0;
                else if (pattern[y][x] && (counter == 2 || counter == 3))
                    pattern_two[y][x] = 1;
                else if (!pattern[y][x] && counter == 3)
                    pattern_two[y][x] = 1;
            }
        }

        if (CounterIteration % freq == 0){
            MakeFileBMP(CounterIteration,bmp_file,&pattern_two,OutputDir);
        }

        swap(&pattern,&pattern_two);

    }

    return 0;
}
