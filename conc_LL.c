#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sched.h>
#include <assert.h>
#include <pthread.h> 
#include <string.h> 

//-----------------------general purpose methods and structs------------------------------

long us(struct timeval t) { 
  return((t.tv_sec * 1000000 + t.tv_usec));
}

double seconds(long nanosec) {
    return ((double) nanosec) / 1000000.0;
}

//-------------------bad list nodes and methods-------------------------------------------

typedef struct _bnode {
    int value;
    struct _bnode *next;
} bnode;

typedef struct _blist {
    bnode *head;
    pthread_mutex_t lock;
} blist;

void binit(blist* l) {
    l->head = NULL;
    pthread_mutex_init(&l->lock, NULL);
}

void bcount(blist *bad_list) {
    bnode *cur =  bad_list->head;
    int acc = 0;

    while (cur) {
        acc = acc + 1;
        cur = cur->next;
    }

    printf("num entries bad list: %d\n", acc);
}

int binsert(blist *l , int value) {
    int ret = 0;
    bnode *new = malloc(sizeof(bnode));

    if (new == NULL) {
        perror("malloc");
        ret = -1;
    }

    new->value = value;

    pthread_mutex_lock(&l->lock);   
    new->next = l->head;
    l->head = new;
    pthread_mutex_unlock(&l->lock);

    return ret;
}

int blookup(blist *l, int value) {
    int ret = -1;
    pthread_mutex_lock(&l->lock);
    bnode *cur =  l->head;

    while (cur) {
        if (cur->value == value) {
            ret = 0;
            break;
        }
        cur = cur->next;
    }

    pthread_mutex_unlock(&l->lock);
    return ret;
}

//-------------------good list nodes and methods-------------------------------------------

typedef struct _gnode {
    int value;
    struct _gnode *next;
    pthread_mutex_t lock;
} gnode;

typedef struct _glist {
    gnode *head;
} glist;

void ginit(glist* l) {
    gnode *dummy = malloc(sizeof(gnode));
    pthread_mutex_init(&dummy->lock, NULL);
    dummy->next = NULL;
    l->head = dummy;
}

void gcount(glist *good_list) {
    gnode *cur =  good_list->head;
    int acc = 0;

    while (cur) {
        acc = acc + 1;
        cur = cur->next;
    }

    printf("num entries good list: %d\n", acc);
}

int ginsert(glist *l , int value) {
    int ret = 0;
    gnode *new = malloc(sizeof(gnode));

    if (new == NULL) {
        perror("malloc");
        ret = -1;
    }

    new->value = value;
    pthread_mutex_init(&new->lock, NULL);

    pthread_mutex_lock(&l->head->lock);

    new->next = l->head->next;

    pthread_mutex_lock(&new->lock);

    l->head->next = new;

    pthread_mutex_unlock(&l->head->next->lock);
    pthread_mutex_unlock(&l->head->lock);

    return ret;
}

int glookup(glist *l, int value) {
    int ret = -1;                      
    gnode *cur = l->head->next;

    if (cur == NULL)
        return -1;

    pthread_mutex_lock(&cur->lock);

    while (cur) {
        if (cur->value == value) {
            ret = 0;
            pthread_mutex_unlock(&cur->lock);
            break;
        }

        if (cur->next == NULL) {
            ret = -1;
            pthread_mutex_unlock(&cur->lock);
            break;
        }

        pthread_mutex_lock(&cur->next->lock);
        gnode *prev = cur;
        cur = cur->next;
        pthread_mutex_unlock(&prev->lock);
    }

    return ret;
}

int millionNodeNum = 50000;
int printStatements = 0;

//---------------bad thread worker functions--------------------------------------------------------

//bad insert worker
void *badInsertWorker(void *arg) {
    for (int i = 0 ; i < millionNodeNum ; i++) {
        if (i % (millionNodeNum / 10) == 0 && printStatements == 1)
            printf("badinsertworker called %d times.\n",i);
        binsert((blist*) arg, rand() % 1000001);
    }  
        
    return 0;
}

//bad lookup worker
void *badLookupWorker(void *arg) {
    for (int i = 0 ; i < millionNodeNum ; i++) {
        if (i % (millionNodeNum / 10) == 0 && printStatements == 1)
            printf("badlookupworker called %d times.\n",i);
        blookup((blist*) arg, rand() % 1000001);
    }

    return 0;
}

//--------------good thread worker functions-----------------------------------------------------

//good insert worker
void *goodInsertWorker(void *arg) {
    for (int i = 0 ; i < millionNodeNum ; i++) {
        if (i % (millionNodeNum / 10) == 0 && printStatements == 1)
            printf("goodinsertworker called %d times.\n",i);
         ginsert((glist*) arg, rand() % 1000001);
    }

    return 0;
}

//good lookup worker
void *goodLookupWorker(void *arg) {
    for (int i = 0 ; i < millionNodeNum ; i++) {
        if (i % (millionNodeNum / 10) == 0 && printStatements == 1)
            printf("goodlookupworker called %d times.\n",i);
        glookup((glist*) arg, rand() % 1000001);
    }

    return 0;
}

//-------main testing code of homework 3--------------------------------------------------------

struct timeval t1;
struct timeval t2;

int rc;
int i;
pthread_t tids[2];

int main() {
    gettimeofday(&t1,NULL); //start time

    blist *bad_list = malloc(sizeof(blist));
    binit(bad_list);

    rc = pthread_create(&tids[0], NULL, badInsertWorker, bad_list); assert(rc == 0);
    rc = pthread_create(&tids[1], NULL, badInsertWorker, bad_list); assert(rc == 0);

    for (i = 0 ; i < 2 ; i++)
        rc = pthread_join(tids[i],NULL); assert(rc == 0);

    gettimeofday(&t2,NULL); //end time
    printf("Inserting 2M (1M each) entries into bad list using 2 threads took: %f sec\n",seconds(us(t2) - us(t1)));

    gettimeofday(&t1,NULL); //start time

    glist *good_list = malloc(sizeof(glist));
    ginit(good_list);

    rc = pthread_create(&tids[0], NULL, goodInsertWorker, good_list); assert(rc == 0);
    rc = pthread_create(&tids[1], NULL, goodInsertWorker, good_list); assert(rc == 0);

    for (i = 0 ; i < 2 ; i++)
        rc = pthread_join(tids[i],NULL); assert(rc == 0);

    gettimeofday(&t2,NULL); //end time
    printf("Inserting 2M (1M each) entries into good list using 2 threads took: %f sec\n",seconds(us(t2) - us(t1)));

    gettimeofday(&t1,NULL); //start time

    //init bad list
    blist *bad_list_2 = malloc(sizeof(blist));
    binit(bad_list_2);

    //spawn a thread to insert 1M entries into bad list
    rc = pthread_create(&tids[0], NULL, badInsertWorker, bad_list_2); assert(rc == 0);
    //spawn a thread to lookup 1M entires from bad list at the same time
    rc = pthread_create(&tids[1], NULL, badLookupWorker, bad_list_2); assert(rc == 0);
    //wait for threads to finish
    for (i = 0 ; i < 2 ; i++)
        rc = pthread_join(tids[i],NULL); assert(rc == 0);
    
    gettimeofday(&t2,NULL); //end time
    printf("Empty list 1 thread insert 1M entries, 1 thread lookup 1M concurrently using bad list took: %f sec\n",seconds(us(t2) - us(t1)));

    gettimeofday(&t1,NULL); //start time

    glist *good_list_2 = malloc(sizeof(glist));
    ginit(good_list_2);

    rc = pthread_create(&tids[0], NULL, goodInsertWorker, good_list_2); assert(rc == 0);
    rc = pthread_create(&tids[1], NULL, goodLookupWorker, good_list_2); assert(rc == 0);

    for (i = 0 ; i < 2 ; i++)
        rc = pthread_join(tids[i],NULL); assert(rc == 0);

    gettimeofday(&t2,NULL); //end time
    printf("Empty list 1 thread insert 1M entries, 1 thread lookup 1M concurrently using good list took: %f sec\n",seconds(us(t2) - us(t1)));

    //init bad list
    blist *bad_list_3 = malloc(sizeof(blist));
    binit(bad_list_3);

    //insert 1M entries
    for (int i = 0 ; i < millionNodeNum ; i++)
        rc = binsert(bad_list_3, rand() % 1000001);

    gettimeofday(&t1,NULL); //start time

    //spawn 2 threads that each will look up 1M entries from bad list
    rc = pthread_create(&tids[0], NULL, badLookupWorker, bad_list_3); assert(rc == 0);
    rc = pthread_create(&tids[1], NULL, badLookupWorker, bad_list_3); assert(rc == 0);

    //wait for threads to finish
    for (i = 0 ; i < 2 ; i++)
        rc = pthread_join(tids[i],NULL); assert(rc == 0); 

    gettimeofday(&t2,NULL); //end time
    printf("Start with 1M entries, 2 threads lookup 1M entries each with bad list took: %f sec\n",seconds(us(t2) - us(t1)));

    //init good list
    glist *good_list_3 = malloc(sizeof(glist));
    ginit(good_list_3);

    //insert 1M entries
    for (int i = 0 ; i < millionNodeNum ; i++)
        rc = ginsert(good_list_3, rand() % 1000001);

    gettimeofday(&t1,NULL); //start time

    //spawn 2 threads that each will look up 1M entries from good list
    rc = pthread_create(&tids[0], NULL, goodLookupWorker, good_list_3); assert(rc == 0);
    rc = pthread_create(&tids[1], NULL, goodLookupWorker, good_list_3); assert(rc == 0);

    //wait for threads to finish
    for (i = 0 ; i < 2 ; i++)
        rc = pthread_join(tids[i],NULL); assert(rc == 0); 
    
    gettimeofday(&t2,NULL); //end time
    printf("Start with 1M entries, 2 threads lookup 1M entries each with good list took: %f sec\n",seconds(us(t2) - us(t1)));

    return 0;
}