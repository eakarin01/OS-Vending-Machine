#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define SNUM 5
#define CNUM 8
#define MAXQ 20


typedef struct conf {
    char name[256];
    int interval;
    int repeat;
}cfile;

struct argument
{
    char *name;
    cfile *cfg;
    int *supply;
    pthread_mutex_t *lock;
    pthread_cond_t *queue;
    pthread_cond_t *wakeup;
};


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

int indexOfSupply(cfile sup[],int size,char name[])
{
    // default value
    int index = -1;
    for(int i=0;i<size;i++)
    {
        // check name of supply
        if(!strcmp(sup[i].name,name))
        {
            index = i;
            return index;
        }
    }
    return index;
}

void *supplierDo(void* arg)
{
    // convert argument
    struct argument *myarg = arg;
    while(1){
    // critical section
    pthread_mutex_lock( myarg->lock );
    // check if full
    while ( *(myarg->supply) >= MAXQ )
    {
        // move to wait queue
        printf("\e[93m%s supplier going to wait\n", myarg->cfg->name);
        pthread_cond_wait( myarg->queue , myarg->lock   );
    }
    // when can pass from while unit++
    (*(myarg->supply))++;
    printf("\e[92m%s supplied 1 unit. stock after = %d\n",myarg->cfg->name,*(myarg->supply));
    // wake up consumer queue
    pthread_cond_signal( myarg->wakeup );
    pthread_mutex_unlock( myarg->lock );
    sleep(1);
    }
}

void *consumerDo(void* arg)
{
    // convert argument
    struct argument *myarg = arg;
    while(1){
    // critical section
    pthread_mutex_lock( myarg->lock );
    // check if empty
    while ( *(myarg->supply) <= 0 )
    {
        // move to wait queue
        printf("\e[94m%s consumer going to wait\n", myarg->cfg->name);
        pthread_cond_wait( myarg->queue , myarg->lock   );
    }
    // when can pass from while unit--
    (*(myarg->supply))--;
    printf("\e[31m%s consumed 1 unit. stock after = %d\n",myarg->cfg->name,*(myarg->supply));
    // wake up supplier queue
    pthread_cond_signal( myarg->wakeup );
    pthread_mutex_unlock( myarg->lock );
    sleep(1);
    }
}


void main()
{
    // initial my vending machine
    printf("Vending Machine Setup...\n");
    // init supply variable
    cfile mysup[SNUM];
    cfile mycon[CNUM];
    int supply[SNUM];
    loadSuppCfg(mysup,sizeof(mysup)/sizeof(mysup[0]));
    init_supply(supply,sizeof(supply)/sizeof(supply[0]));
    loadConsCfg(mycon,sizeof(mycon)/sizeof(mycon[0]));
    printf("Vending Machine Ready!!\n");

    // print to test bug
    for (int i=0;i<SNUM;i++)
    {
        printf("Supplier %d\n",i+1);
        printf("Name : %s\n",mysup[i].name);
        printf("Interval : %d\n",mysup[i].interval);
        printf("Repeat : %d\n\n",mysup[i].repeat);
    }
    for (int i=0;i<CNUM;i++)
    {
        printf("Consumer %d\n",i+1);
        printf("Name : %s\n",mycon[i].name);
        printf("Interval : %d\n",mycon[i].interval);
        printf("Repeat : %d\n\n",mycon[i].repeat);
    }

    printf("---------------------------------------\n");
    // create supplier & consumer thread
    pthread_t supthread[SNUM];
    pthread_t conthread[CNUM];
    pthread_mutex_t slock[SNUM];
    pthread_cond_t supq[SNUM];
    pthread_cond_t conq[SNUM];
    struct argument suparg[SNUM];
    struct argument conarg[CNUM];
    // initial all queue
    for(int i=0;i<SNUM;i++)
    {
        pthread_cond_init(&supq[i],NULL);
        pthread_cond_init(&conq[i],NULL);
    }
    // initial all supply mutex
    for(int i=0;i<SNUM;i++)
    {
        pthread_mutex_init(&slock[i],NULL);
    }
    // --- [Thread Zone no expected sequential] ---
    // loop for create suppplier thread
    for(int i=0;i<SNUM;i++)
    {
        // set parameter to send to thread func
        suparg[i].cfg = &mysup[i];
        suparg[i].supply = &supply[i];
        suparg[i].lock = &slock[i];
        suparg[i].queue = &supq[i];
        suparg[i].wakeup = &conq[i];
        pthread_create(&supthread[i],NULL,&supplierDo,&suparg[i]);
    }
    for(int i=0;i<CNUM;i++)
    {
        // set parameter to send to thread func
        conarg[i].cfg = &mycon[i];
        // search index of supply
        int idx = indexOfSupply(mysup,sizeof(mysup)/sizeof(mysup[0]),mycon[i].name);
        conarg[i].supply = &supply[idx];
        conarg[i].lock = &slock[idx];
        conarg[i].queue = &conq[idx];
        conarg[i].wakeup = &supq[idx];
        pthread_create(&conthread[i],NULL,&consumerDo,&conarg[i]);
    }


    // wait for all thread finished.
    for(int i=0;i<SNUM;i++)
        pthread_join(supthread[i],NULL);
    for(int i=0;i<CNUM;i++)
        pthread_join(conthread[i],NULL);

}