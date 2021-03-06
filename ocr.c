# include <err.h>
# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <time.h>
# include <gtk/gtk.h>
# include <string.h>

# include "property.h"
# include "LayerStruct.h"
# include "savesystem.h"
# include "matrix.h"
# include "Picture_Treatment/picture_treatment.h"

# include "ocr.h"

void insertChar (char* str, char c, int pos) {
  char *ptr = str + pos;
  memmove(ptr + 1, ptr, strlen(ptr) + 1); 
  *ptr = c;
}

char* execOcr(char* filename) {
  int *len = malloc(sizeof(int));
  int *l_back = malloc(sizeof(int)); 
  int *l_space = malloc(sizeof(int));
  double **array = getFinal(filename,len, &l_back, &l_space);
  //printf("%d\n", *(l_back));
  char* string = justforward(array,*len);
  //printf("%d\n", *(l_back));
  
  free(array);
  
  *len += *(l_back) + *(l_space);

  char* newstr = malloc(*len * sizeof(char));
  strcpy(newstr, string);
  int decalage = 0;

  int currentline = *(l_back);
  int currentspace = *(l_space);
  for (int i = *len; i > 0; i--) {
    if (i == *(l_back+currentline) && currentline != 0) {
      //decalage++;
      //printf("%d\n", *(l_back+currentline));
      insertChar(newstr, '\n', *(l_back+currentline)+decalage);
      currentline--;
    }
    if (i == *(l_space+currentspace) && currentspace != 0) {
      //printf("%d\n", *(l_space+currentspace));
      insertChar(newstr, ' ', *(l_space+currentspace)+decalage);
      currentspace--;
    }
  }

  free(l_back);
  free(len);
  free(string);
  free(l_space);
  return newstr;
}

char *justforward(double **input, int lenlist)
{
  Layer layerHidden;
  Layer layerOutput;

  char *result = malloc (lenlist * sizeof(char));


  LoadData(&layerHidden,&layerOutput);

  for (int i = 0; lenlist > i; i++)
  {
    front2(*(input + i),&layerHidden);
    front2(layerHidden.result,&layerOutput);

    double result2 = layerOutput.result[0];
    int resultpos = 0;

    for(int it = 1; it < 94; it++)
    {
        //printf("%lf:%c\n", layerOutput.result[it], 'A'+it);
        if(layerOutput.result[it] > result2)
        {
          result2 = layerOutput.result[it];
          resultpos = it;
        }
    }
    char reponse = 33 + resultpos;
    *(result + i) = reponse;
    free(*(input + i));
  }
  destroyLayer(&layerHidden);
  destroyLayer(&layerOutput);
  return result;
}


int apprentissage(int nbmid, int ite, int load)
{
    srand ( clock()  );

    // one arg = load savefile (futur)
    // Min arg = in, mid, out, test file (futur) + LEARNING_RATE
    // Max arg = in mid, ..., mid, out, test file (futur) + LEARNING_RATE

    // INIT
    Layer layerHidden;
    //Layer layerHidden2; (futur)
    Layer layerOutput;
    int nbinput = 26*26;

    int nbout = 94;
    double learning = LEARNING_RATE;

    if (load == 0)
    {
      if (nbmid > 99999)
        {
          printf("I don't think more than 40 is necessary for a XOR...\n");
          return 1;
        }
      initLayer(nbinput,nbmid,&layerHidden);
      initLayer(nbmid,nbout,&layerOutput);
    }
    else
    {
      learning = LoadData(&layerHidden,&layerOutput);
    }

    int truecount2 = 0;
    int falsecount2 = 0;
    int lastFalse2 = 0;


        for(int i = 0; i < ite; i++) { // On lit le fichier

            int r = rand() % 94;
            int alpha = rand() % 3;
            char path = r;
            //Faire un random 26 pour trouver une lettre, la mettre dans path, importer en matrice la lettre .bmp
            double *input = loadMatrix(path, alpha);
            //print_matrix(input,26,26);
            double *answer = malloc(94*sizeof(double));
            for(int i = 0; i < 94; i++)
              answer[i] = 0;

            answer[r] = 1;

            // FRONT 2
            front2(input,&layerHidden);
            front2(layerHidden.result,&layerOutput);
            //ENDFRONT
            double result2 = layerOutput.result[0];
            int resultpos = 0;

            for(int it = 1; it < 94; it++) {
                //printf("%lf:%c\n", layerOutput.result[it], 'A'+it);
                if(layerOutput.result[it]>result2) {
                  result2 = layerOutput.result[it];
                  resultpos = it;
                }
            }

            // AFFICHAGE

            char reponse = 33 + resultpos;


            if (answer[resultpos] >= 1) {
                //printf("Ité %d : target = %c\n", i,path+33);
                //printf("%lf (Struct) = %c",result2, reponse);
                //printf(" : TRUE (Struct)\n\n\n");
                truecount2++;
            } else {
                printf("Ité %d : target = %c\n", i,path+33);
                printf("%lf (Struct) = %c",result2, reponse);
                printf(" : FALSE (Struct)\n");
                falsecount2++;
                lastFalse2 = i;
                printf("Debug - %c : %lf\n", path+33, layerOutput.result[(int)path]);
            }
            if (answer[resultpos] == 0 || result2 < 0.95)
            //if (layerOutput.result[(int)path] < 0.000017)            
            {

                //double tanswer[1] = {answer};
                // back - propagation : Il faut commencer par l'output et
                //                      remonter vers l'input.
                outToHidden(answer, &layerOutput, &layerHidden);
                hiddenToInput(&layerOutput, &layerHidden, input);
                applyChanges(&layerHidden, learning);
                applyChanges(&layerOutput, learning);
                printf("Auto-correction\n\n");
            }

            free(input);
            free(answer);
          }


    printf("True2 : %d\nFalse2 : %d\n", truecount2, falsecount2);
    printf("Last False2 : %d\n", lastFalse2);

    SaveData(&layerHidden,&layerOutput,learning);
    destroyLayer(&layerHidden);
    destroyLayer(&layerOutput);


    return 0;
}
