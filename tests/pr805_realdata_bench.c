#define _POSIX_C_SOURCE 200809L
#include <blosc2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
static double ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (double)t.tv_sec*1e9+t.tv_nsec;}
int main(int argc,char**argv){
 if(argc!=5){fprintf(stderr,"usage: bench dataset bytes threads iterations\n");return 2;}
 const char*file=argv[1];size_t n=(size_t)strtoull(argv[2],0,10);int nt=atoi(argv[3]),loops=atoi(argv[4]);
 FILE*f=fopen(file,"rb");if(!f){perror(file);return 2;}fseek(f,0,SEEK_END);long fl=ftell(f);rewind(f);
 if(fl<=0||n==0||loops<=0||nt<1){fprintf(stderr,"invalid data/params\n");return 2;}
 unsigned char*sample=malloc(fl),*src=malloc(n),*dst=malloc(n*2+4096);if(!sample||!src||!dst)return 2;
 if(fread(sample,1,fl,f)!=(size_t)fl)return 2;fclose(f);
 for(size_t i=0;i<n;i++)src[i]=sample[i%(size_t)fl];
 blosc2_init();
 blosc2_cparams p=BLOSC2_CPARAMS_DEFAULTS;p.nthreads=nt;p.blocksize=16384;p.clevel=5;p.typesize=4;
 double elapsed=0;int total=loops+20;
 for(int i=0;i<total;i++){
   double start=ns();
   blosc2_context*ctx=blosc2_create_cctx(p);if(!ctx){fprintf(stderr,"ctx failed\n");return 3;}
   int out=blosc2_compress_ctx(ctx,src,(int32_t)n,dst,(int32_t)(n*2+4096));
   blosc2_free_ctx(ctx);
   double dt=ns()-start;
   if(out<=0){fprintf(stderr,"compression failed %d\n",out);return 3;}
   if(i>=20)elapsed+=dt;
 }
 printf("RESULT,bytes=%zu,threads=%d,iterations=%d,mean_us=%.6f,dataset_bytes=%ld\n",n,nt,loops,elapsed/loops/1000.,fl);
 blosc2_destroy();free(src);free(dst);free(sample);return 0;
}
