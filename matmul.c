#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


double **a;//first matrix
double **b;//second matrix
double **c1;//first output matrix
double **c2;//second output matrix
char* fileA;//file of first matrix
char* fileB;//file  of second matrix
char* fileC1;//file of  first output matrix
char* fileC2;//file of second output matrix


struct row{
    int rowNum;
};
struct element{
    int rowNum;
    int colNum;
};

//holds  the dimensions of the two matrices a,b
struct dimensions{
    int r1,r2,c1,c2;
}*idx;


//to set file names

void intialize(int argc,char **argv){



    if(argc == 1){
        //default case no arguments in the terminal
        fileA = (char*)malloc(6);
        fileB = (char*)malloc(6);
        fileC1 = (char*)malloc(7);
        fileC2 = (char*)malloc(7);

        strcpy(fileA,"A.txt");
        strcpy(fileB,"B.txt");
        strcpy(fileC1,"C1.txt");
        strcpy(fileC2,"C2.txt");
    }else if(argc == 4){
        fileA = (char*)malloc(strlen(argv[1])+4);
        fileB = (char*)malloc(strlen(argv[2])+4);
        fileC1 = (char*)malloc(strlen(argv[3])+6);
        fileC2 = (char*)malloc(strlen(argv[3])+6);

        strcpy(fileA,argv[1]);
        strcat(fileA,".txt");

        strcpy(fileB,argv[2]);
        strcat(fileB,".txt");

        strcpy(fileC1,argv[3]);
        strcat(fileC1,"1.txt");

        strcpy(fileC2,argv[3]);
        strcat(fileC2,"2.txt");
        printf("%d",argc);
    }else{
        //more than 4 parameters which is not acceptable
        printf("Error in the arguments\n");
        exit(0);
    }
}

void readMatrix(char *fileName,int isFirst,int *r ,int *c){
    FILE* f = fopen(fileName,"r");
    if(f == NULL){
        printf("file cannot be found\n");
        exit(0);
    }
    //read the matrix dimensions
    char buffer[255];
    fgets(buffer,255,f);
    char *ptr = strchr(buffer,'=');
    if(ptr == NULL){
        printf("Error in matrix dimension\n");
        exit(0);
    }
    ptr++;
    //remove spaces after = sign
    while(*ptr == ' '){
        ptr++;
    }
    int val = 0;
    while(*ptr <= '9' && *ptr >= '0'){
        val *= 10;
        val += ((*ptr) - '0');
        ptr++;
    }
    *r = val;
    val = 0;
    ptr = strchr(ptr,'=');
    if(ptr == NULL){
        printf("Error in matrix dimension\n");
        exit(0);
    }
    ptr++;
    //remove spaces after = sign
    while(*ptr == ' '){
        ptr++;
    }
    val = 0;
    while(*ptr <= '9' && *ptr >= '0'){
        val *= 10;
        val += ((*ptr) - '0');
        ptr++;
    }
    *c = val;
    int i,j;
    //allocate space for  the matrix
    if(isFirst){
        a = (double**)malloc((*r) *  sizeof(double*));

        for(i = 0 ; i < (*r) ; i++){
            a[i] = (double*)malloc((*c) * sizeof(double));
        }
    }else{
        b = (double**)malloc((*r) *  sizeof(double*));
        for( i = 0 ; i < (*r) ; i++){
            b[i] = (double*)malloc((*c) * sizeof(double));
        }
    }
    //read the values from file
    for( i = 0 ; i < (*r) ; i++){
        for( j = 0 ; j < (*c) ; j++){
            fscanf(f,"%s",buffer);
            if(isFirst){
                a[i][j] = atof(buffer);
            }else{
                b[i][j] = atof(buffer);
            }

        }
    }
    fclose(f);
    return ;
}
void* rowRunnable(void* arg){
    struct row  *parameter= (struct row *)arg;
    int i,j;
    for(i =  0 ; i < idx->c2; i++){//various columns
        double sum = 0;
        for(j = 0 ; j < idx->c1 ; j++){
            sum += a[parameter->rowNum][j] * b[j][i];
        }
        c1[parameter->rowNum][i] = sum;
    }
    pthread_exit(NULL);
}

void *elementRunnable(void *arg){
    struct element *parameter = (struct element*)arg;
    double sum = 0;
    int i;
    for(i = 0 ; i < idx->c1;i++){
        sum += a[parameter->rowNum][i] * b[i][parameter->colNum];
    }
    c2[parameter->rowNum][parameter->colNum] = sum;
    pthread_exit(NULL);
}

// to write the matrix  to file called : *fileName
void writeMatrix(char* fileName,int first){
    FILE *f;
    f = fopen(fileName,"w");

    int i,j;
    for(i = 0 ; i < idx->r1 ; i++){
        for(j = 0 ; j < idx->c2 ; j++){
            char cast[512];
            if(first){
                sprintf(cast,"%f",c1[i][j]);
                fputs(cast,f);
                fputs(" ",f);
            }else{
                sprintf(cast,"%f",c2[i][j]);
                fputs(cast,f);
                fputs(" ",f);
            }
        }
        fputs("\n",f);
    }
    fclose(f);
    return ;
}
int main(int arc ,char **argv)
{
    intialize(arc,argv);
    int i,j,r_1,r_2,c_1,c_2;
    readMatrix(fileA,1,&r_1,&c_1);
    readMatrix(fileB,0,&r_2,&c_2);
    if(r_1 <= 0 || r_2 <= 0 || c_1 <= 0 || c_2 <= 0 || c_1 != r_2){
        printf("Error in the matrix dimensions\n");
        exit(0);
    }
    //get the  dimensions of  the  two matrices
    struct dimensions temp;
    temp.r1 = r_1;
    temp.c1 = c_1;
    temp.r2 = r_2;
    temp.c2 = c_2;
    idx = &temp;
    //allocate space for the output matrices
    c1 = (double**)malloc(r_1 * sizeof(double*));
    c2 = (double**)malloc(r_1 * sizeof(double*));
    for(i =  0 ; i < r_1 ; i++){
        c1[i] = (double*)malloc(c_2*sizeof(double));
        c2[i] = (double*)malloc(c_2*sizeof(double));
    }
    //make row processing
    for(i = 0 ; i < idx->r1 ; i++){
        pthread_attr_t attr;
		pthread_attr_init(&attr);
		struct row info;
		info.rowNum = i;
		pthread_t tid;
		pthread_create(&tid, &attr, rowRunnable, &info);
        pthread_join(tid, NULL);
    }
    //make element processing
    for(i = 0 ; i < idx->r1 ; i++){
        for(j = 0 ; j < idx->c2 ; j++){
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            struct element info;
            info.rowNum = i;
            info.colNum = j;
            pthread_t tid;
            pthread_create(&tid, &attr, elementRunnable, &info);
            pthread_join(tid,NULL);
        }
    }
    //write the output matrices in the files : fileC1 ,fileC2
    writeMatrix(fileC1,1);
    writeMatrix(fileC2,0);
    //free the memory used
    free(a);
    free(b);
    free(c1);
    free(c2);
    free(fileA);
    free(fileB);
    free(fileC1);
    free(fileC2);
    return 0;
}
