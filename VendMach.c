#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define SCOUNT 5
#define CCOUNT 8
#define MAXQ 100


typedef struct conf {
    char name[256];
    int interval;
    int repeat;
}cfile;


void init_supply(int supply[],int size)
{
    for(int i=0;i<size;i++)
        supply[i] = 0;
}

void loadSuppCfg(cfile mysup[],int size)
{
    // read supplier config file
    for (int i=0;i<size;i++){
        // get file name
        char fname[256];
        sprintf(fname,"supplier%d.txt",i+1);
        printf("Loading file %s\n",fname);
        // open config file with read only
        FILE* fp = fopen(fname, "r");
        // when don't have config file
        if (!fp){
            printf("Can't load config file [%s]\n",fname);
            return;
        }
        // found config file
        else{
            // load supplier config file to memory
            fscanf(fp,"%[^\n]",mysup[i].name);
            fscanf(fp,"%d",&mysup[i].interval);
            fscanf(fp,"%d",&mysup[i].repeat);
        }
    }
    sleep(1);
}

void loadConsCfg(cfile mycon[],int size)
{
    // read consumer config file
    for (int i=0;i<size;i++){
        // get file name
        char fname[256];
        sprintf(fname,"consumer%d.txt",i+1);
        printf("Loading file %s\n",fname);
        // open config file with read only
        FILE* fp = fopen(fname, "r");
        // when don't have config file
        if (!fp){
            printf("Can't load config file [%s]\n",fname);
            return;
        }
        // found config file
        else{
            // load supplier config file to memory
            fscanf(fp,"%[^\n]",mycon[i].name);
            fscanf(fp,"%d",&mycon[i].interval);
            fscanf(fp,"%d",&mycon[i].repeat);
        }
    }
    sleep(1);
}


void main()
{
    // initial my vending machine
    printf("Vending Machine Setup...\n");
    // init supply variable
    cfile mysup[SCOUNT];
    cfile mycon[CCOUNT];
    int supply[SCOUNT];
    loadSuppCfg(mysup,sizeof(mysup)/sizeof(mysup[0]));
    init_supply(supply,sizeof(supply)/sizeof(supply[0]));
    loadConsCfg(mycon,sizeof(mycon)/sizeof(mycon[0]));
    printf("Vending Machine Ready!!\n");

    // print to test bug
    for (int i=0;i<SCOUNT;i++)
    {
        printf("Supplier %d\n",i+1);
        printf("Name : %s\n",mysup[i].name);
        printf("Interval : %d\n",mysup[i].interval);
        printf("Repeat : %d\n\n",mysup[i].repeat);
    }
    for (int i=0;i<CCOUNT;i++)
    {
        printf("Consumer %d\n",i+1);
        printf("Name : %s\n",mycon[i].name);
        printf("Interval : %d\n",mycon[i].interval);
        printf("Repeat : %d\n\n",mycon[i].repeat);
    }
}