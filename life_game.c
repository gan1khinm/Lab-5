#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#pragma pack(1)

#define BITMAPMAGICIDENTIFIER 0x4D42

typedef struct {
   uint16_t type;                  
   uint32_t size;                 
   uint16_t reserved1, reserved2;  
   uint32_t offset;           
} BMPHEADER;

typedef struct {
   uint32_t size;                  
   int32_t width,height;           // Ширина и высота изображения
   uint16_t planes;               
   uint16_t bits;                  // Бит на пиксель
   uint32_t compression;          
   uint32_t imagesize;             // Размер изображения в байтах       
   int32_t xres,yres;              
   uint32_t ncolours;              //Кол-во цветов
   uint32_t importantcolours;      
} BMPINFOHEADER;


#define EXIT_ERR_FILE_DOESNT_EXISTS   1
#define EXIT_ERR_FILE_NOT_BITMAP      2
#define EXIT_ERR_MEMORY_ALLOCATION    3
#define EXIT_ERR_READ                 4
#define EXIT_ERR_1BIT_SUPPORTED       5


void puterr(char* msg) {    //Ошибка
  printf("%s\n", msg);
  printf("Quit\n");
  return;
}


unsigned char *parse_bmp(char *fname, int *_w, int *_h, char *_order, char *_invert){    //Перевод bmp в массив 

  BMPHEADER header;
  BMPINFOHEADER infoheader;
  unsigned char *img;
  unsigned char *data;
  int i, j, rev_j, pos, ipos, tmp_w;

  FILE *fp = fopen(fname, "rb");
  if(fp == NULL) {
    puterr("File doesnt exists of cannot be opened.");
    exit(EXIT_ERR_FILE_DOESNT_EXISTS);
  }

  fread(&header, sizeof(BMPHEADER), 1, fp);

  //Проверка bmp
  if(header.type != BITMAPMAGICIDENTIFIER) {
    fclose(fp);
    puterr("File isn't bitmap image. Only BMP files are supported.");
    exit(EXIT_ERR_FILE_NOT_BITMAP);
  }

  fread(&infoheader, sizeof(BMPINFOHEADER), 1, fp);

  if(infoheader.bits != 1) {
    fclose(fp);
    puterr("Only 1-bit monochrome bitmap files are supported.");
    exit(EXIT_ERR_1BIT_SUPPORTED);
  }

  
  int lineSize = (infoheader.width / 32) * 4;
  if (infoheader.width % 32) {
    lineSize += 4;
  }

  
  int fileSize = lineSize * infoheader.height;

  data = malloc(fileSize);
  img = malloc(infoheader.width * infoheader.height);

  if (!img || !data) {
    free(img);
    free(data);
    puterr("Cannot allocate memory");
    fclose(fp);
    exit(EXIT_ERR_MEMORY_ALLOCATION);
  }

  //Переместить точку в начало
  fseek(fp, header.offset, SEEK_SET);

  fread(data, 1 , fileSize, fp);

  if (data == NULL) {
    fclose(fp);
    puterr("Cannot read image data");
    exit(EXIT_ERR_READ);
  }

  
  ipos = 0;
  if (infoheader.width < 8) {
    tmp_w = 8;
  } else {
    tmp_w = infoheader.width;
  }
  for (j = 0, rev_j = infoheader.height-1;j < infoheader.height; j++, rev_j--) {
    for (i = tmp_w/8-1; i >= 0; i--) {

      // Инвертировать ли порядок байт
      if (_order == NULL) {
        pos = rev_j * lineSize + i;
      } else {
        pos = j * lineSize + i;
      }

      // Инвертировать ли изображение
      if (_invert == NULL) {
        img[ipos] = data[pos];
      } else {
        img[ipos] = 0xFF - data[pos];
      }

      ipos += 1;
    }
  }

  fclose(fp);
  free(data);
  *_w = tmp_w;
  *_h = infoheader.height;
  return img;
}


void toBmp(bool mas[16][16], int height, int width, int n) {        //Перевод из массива в bmp 
    

    int i, row, column;
    int size = width * height * 4; 

    char header[54] = { 0 };
    strcpy(header, "BM");
    memset(&header[2],  (int)(54 + size), 1);
    memset(&header[10], (int)54, 1);
    memset(&header[14], (int)40, 1);
    memset(&header[18], (int)width, 1);
    memset(&header[22], (int)height, 1);
    memset(&header[26], (short)1, 1);
    memset(&header[28], (short)32, 1);
    memset(&header[34], (int)size, 1);

    unsigned char *pixels = malloc(size);
    for(row = height - 1, i = 0; row >= 0; row--, i++) {
        for(column = 0; column < width; column++) {
            int p = (row * width + column) * 4;
            if(mas[i][column]){
            pixels[p + 0] = 0; //черный
            pixels[p + 1] = 0;
            pixels[p + 2] = 0;
            } else {
                pixels[p + 0] = 255;  //белый 
                pixels[p + 1] = 255;
                pixels[p + 2] = 255;
            }
        }
    
    }
    char filename[15];
    sprintf(filename, "16bit%d.bmp", n);

    FILE *fout = fopen(filename , "wb");
    fwrite(header, 1, 54, fout);
    fwrite(pixels, 1, size, fout);
    free(pixels);
    fclose(fout);
    return 0;
}



int main(int argc, char* argv[]){
    int rev_i, w, h;
    unsigned char* img;
    img = parse_bmp("test16x16.bmp", &w, &h, 'r', 'i');
    bool mas[h][w];
    int i, j = 0,k, l = 0;
    int count_of_life = 0;
    char buffer[9] = "";
    for(i = 0, rev_i = h * (w/8)-1; i < h * (w/8); i++, rev_i--) {


        sprintf(buffer,
            "%c%c%c%c%c%c%c%c",
            (img[rev_i] & 0x80) ? '1':'0',
            (img[rev_i] & 0x40) ? '1':'0',
            (img[rev_i] & 0x20) ? '1':'0',
            (img[rev_i] & 0x10) ? '1':'0',
            (img[rev_i] & 0x08) ? '1':'0',
            (img[rev_i] & 0x04) ? '1':'0',
            (img[rev_i] & 0x02) ? '1':'0',
            (img[rev_i] & 0x01) ? '1':'0');

        for(k = 0; k < 8;k++) {

            mas[l][j] = buffer[k] == '1' ? true: false;
            j++;
        }
        if ((i+1) % (w/8) ==0) {
            j = 0;
            l++;
        }
    }

    for(i = 0; i < h; i++){
        for(j = 0; j < w; j++){
            printf("%d ", mas[i][j]);
            count_of_life += mas[i][j];
        }
        printf("\n");
    }



    printf("\n");

    bool temp[h][w];

    int n = 0;
    while(count_of_life > 0){
        n++;
        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++){  // первый проход: вычисляем будущее состоянее
                bool isAlive = mas[i][j];
                int numNeigbours = 0;
                if (mas[i - 1][j - 1] && i != 0 && j != 0) numNeigbours++;
                if (mas[i - 1][j] && i != 0) numNeigbours++;
                if (mas[i - 1][j + 1] && i != 0 && j != w- 1) numNeigbours++;
                if (mas[i][j - 1] && j != 0) numNeigbours++;
                if (mas[i][j + 1] && j != w- 1) numNeigbours++;
                if (mas[i + 1][j - 1] && j != 0 && i != h- 1) numNeigbours++;
                if (mas[i + 1][j] && i != h - 1) numNeigbours++;
                if (mas[i + 1][j + 1] && i != h - 1 && j != w- 1) numNeigbours++;

                bool keepAlive;
                bool makeNewLive;

                if(isAlive == true && (numNeigbours == 2 || numNeigbours == 3)){
                    keepAlive = true;
                }
                else{
                    keepAlive = false;

                }
                if(!isAlive && numNeigbours == 3){
                    makeNewLive = true;

                }
                else{
                    makeNewLive = false;
                }

                if(keepAlive || makeNewLive){
                    temp[i][j] = true;
                }
                else{
                    temp[i][j] = false;
                }
                printf("%d ", temp[i][j]);
            

            }
            printf("\n");

        }
        count_of_life = 0;
        for(i = 0; i < 10; i++){
            for(j = 0; j < 10; j++){
                mas[i][j] = temp[i][j];
                count_of_life += mas[i][j];
            }
        }
        toBmp(temp, h, w, n);
        
        

        printf("\n");

    }
    return 0;
}
