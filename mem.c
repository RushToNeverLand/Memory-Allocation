#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"

typedef struct block{
    int blank;
    int size;
}block;
void *head;
int region,init=0,m_error;

void mem_dump(){
    void *current=head;
    block *current_piece;
    current_piece=(block *)current;
    while(current!=(head+region)){ 
        //cast the style
        current_piece=(block *)current;
        int shift=current_piece->size;
        //show the condition 
        if(current_piece->blank){
            printf("□ %d   Bytes a free piece begins at %p and ends at %p\n",shift,current,current+shift);
        }
        else{
            printf("■ %d   Bytes a non-free piece begins at %p and ends at %p\n",shift,current,current+shift);
        }
        current+=shift;
    }
    printf("-------------------------------------\n");
}


int mem_init(int size_of_region){
    if (size_of_region <= 0 || init == 1){//error occurs
        m_error = E_BAD_ARGS;
        return -1;
    }
    int gps=getpagesize();
    if(size_of_region%gps!=0){//resize the size of memory size to fit times pagesize
        size_of_region=(1+size_of_region/gps)*gps;
    }
    int fd = open("/dev/zero", O_RDWR);
    // size_of_region (in bytes) needs to be evenly divisible by the page size
    head = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (head == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    // close the device (don't worry, mapping should be unaffected)
    close(fd);
    ((block*)head)->blank=1;
    ((block*)head)->size=size_of_region;
    region=size_of_region;
    // mem_dump();
    init=1;
    return 0;
}

void *mem_alloc(int size, int style){
    void *temp=head,*ans=NULL;
    // void *ans=NULL;
    if(size%8!=0){//resize the size to fit times eight
    	size=(1+size/8)*8;
    }
    size+=sizeof(block);
    if(style==M_BESTFIT){
        while(temp<=head+region){
            if(((block*)temp)->blank==1 &&((block*)temp)->size>=size){//find a proper space to fit the memory
                if(ans==NULL || ((block*)temp)->size<((block*)ans)->size){
                    ans=temp;
                }
            }
            temp+=((block*)temp)->size;
        }
    }
    else if(style==M_WORSTFIT){
        while(temp<=head+region){
            if(((block*)temp)->blank==1 &&((block*)temp)->size>=size){
                if(ans==NULL || ((block*)temp)->size>((block*)ans)->size){
                    ans=temp;
                }
            }
            temp+=((block*)temp)->size;
        }
    }
    else if(style==M_FIRSTFIT){
        while(temp<=head+region){
            if(((block*)temp)->blank==1 &&((block*)temp)->size>=size){
                ans=temp;
                break;
            }
            temp+=((block*)temp)->size;
        }
    }
    if (ans == NULL){
        m_error = E_NO_SPACE;
        return NULL;
    }
    if(((block*)ans)->size>size){ //set next
        ((block*)(ans+size))->blank=1;
        ((block*)(ans+size))->size=((block*)ans)->size-size;
    }
    ((block*)ans)->blank=0;
    ((block*)ans)->size=size;
    ans+=sizeof(block);
    return ans;
}

int mem_free(void* ptr){
    if (ptr == NULL){
        return -1;
    }
    ptr-=sizeof(block);
    ((block*)ptr)->blank=1;
    void * current=head;
    while(current<head+region && current+((block*)current)->size<head+region){
        void * next=current+((block*)current)->size;
        while(((block*)current)->blank==1 && ((block*)next)->blank==1 ){ //if current.blank==1 AND next.blank==true
            ((block*)current)->size+=((block*)next)->size; //current.size+=next.size
             next=current+((block*)current)->size;
        }
        current+=((block*)current)->size;
    }
    return 0;//success
}
