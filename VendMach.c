#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define SNUM 5
#define CNUM 8
#define MAXQ 100


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
    // local variable
    char strout[512];
    char *c_time_string;
    time_t tm;
    int mul_time = 1;
    int count_repeat = 0;
    while(1)
    {
        // set sleep time
        int sleep_time = myarg->cfg->interval*mul_time;
        sleep_time = (sleep_time>60)?60:sleep_time;
        // waiting follow by interval
        sleep(myarg->cfg->interval*mul_time);
        // critical section
        pthread_mutex_lock( myarg->lock );
        // check if full
        if ( *(myarg->supply) >= MAXQ )
        {
            // don't increase supply
            memset(strout,0,512);
            tm = time(NULL);
            c_time_string = strtok(ctime( &tm ),"\n");
            sprintf(strout,"\e[93m%s %s supplier going to wait\n",c_time_string, myarg->cfg->name);
            write(2,strout,sizeof(strout));
            // increase wait count time
            count_repeat++;
            // when loop repeat eq with config
            if (count_repeat >= myarg->cfg->repeat)
            {
                // increase mul time
                mul_time++;
                count_repeat = 0;
            }
        }
        // when is not full
        else
        {
            // increase supply
            (*(myarg->supply))++;
            memset(strout,0,512);
            tm = time(NULL);
            c_time_string = strtok(ctime( &tm ),"\n");
            sprintf(strout,"\e[92m%s %s supplied 1 unit. stock after = %d\n",c_time_string, myarg->cfg->name,*(myarg->supply));
            write(2,strout,sizeof(strout));
            // reset mul time
            mul_time = 1;
        }
        pthread_mutex_unlock( myarg->lock );
    }
}

void *consumerDo(void* arg)
{
    // convert argument
    struct argument *myarg = arg;
    // local variable
    char strout[512];
    char *c_time_string;
    time_t tm;
    int mul_time = 1;
    int count_repeat = 0;
    while(1)
    {
        // set sleep time
        int sleep_time = myarg->cfg->interval*mul_time;
        sleep_time = (sleep_time>60)?60:sleep_time;
        // waiting follow by interval
        sleep(myarg->cfg->interval);
        // critical section
        pthread_mutex_lock( myarg->lock );
        // check if empty
        if ( *(myarg->supply) <= 0 )
        {
            // don't decrease supply
            memset(strout,0,512);
            tm = time(NULL);
            c_time_string = strtok(ctime( &tm ),"\n");
            sprintf(strout,"\e[94m%s %s consumer going to wait\n",c_time_string, myarg->cfg->name);
            write(2,strout,sizeof(strout));
            // increase wait count time
            count_repeat++;
            // when loop repeat eq with config
            if (count_repeat >= myarg->cfg->repeat)
            {
                // increase mul time
                mul_time++;
                count_repeat = 0;

            }
        }
        // when is not empty
        else
        {
            // decrease supply
            (*(myarg->supply))--;
            memset(strout,0,512);
            tm = time(NULL);
            c_time_string = strtok(ctime( &tm ),"\n");
            sprintf(strout,"\e[31m%s %s consumed 1 unit. stock after = %d\n",c_time_string,myarg->cfg->name,*(myarg->supply));
            write(2,strout,sizeof(strout));
            // reset mul time
            mul_time = 1;
        }
        pthread_mutex_unlock( myarg->lock );
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
    struct argument suparg[SNUM];
    struct argument conarg[CNUM];
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
        pthread_create(&supthread[i],NULL,&supplierDo,&suparg[i]);
    }
    // loop for create consumer thread
    for(int i=0;i<CNUM;i++)
    {
        // set parameter to send to thread func
        conarg[i].cfg = &mycon[i];
        // search index of supply
        int idx = indexOfSupply(mysup,sizeof(mysup)/sizeof(mysup[0]),mycon[i].name);
        conarg[i].supply = &supply[idx];
        conarg[i].lock = &slock[idx];
        pthread_create(&conthread[i],NULL,&consumerDo,&conarg[i]);
    }

    // wait for all thread finished.
    for(int i=0;i<SNUM;i++)
        pthread_join(supthread[i],NULL);
    for(int i=0;i<CNUM;i++)
        pthread_join(conthread[i],NULL);

}