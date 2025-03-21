#ifdef _OPENMP
#include <omp.h>
#endif

#include <string.h>
#include <assert.h>
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <algorithm>
#include<complex.h>
#include <fftw3.h>


#include <sys/time.h>

#include "tools.h"
struct timeval now;
double tm1;
double t;
double t0;

void timer(const std::string& Message,bool quiet){
    gettimeofday(&now,NULL);
    t=now.tv_sec+0.000001*now.tv_usec-t0;
    if (!quiet) std::cout<<" Time: "<<(t-tm1)<<std::endl<<Message<<t;
    tm1=t;
}


inline int flatsize(int ny){
    return (ny+1)*(ny+2)*(ny+3)/6;
}

#include <typeinfo>
int main(int argc, char *argv[]){
    
    gettimeofday(&now,NULL);
    t0=now.tv_sec+0.000001*now.tv_usec;
    
    int nside=256;
	int i=1;
	std::string filename("delta_k.dat");
    std::string filenameBk("bk.dat");
    std::string filenameBkind("bkinds.dat");
	bool quiet=0;
    bool fromrealspace=0;
    bool getBkinds=0;
    while (i<argc){
    	if (!strcmp(argv[i],"-quiet")) quiet=1;
    	else if (!strcmp(argv[i],"-nside")) nside = atoi(argv[++i]);
    	else if (!strcmp(argv[i],"-filename")) filename = argv[++i];
        else if (!strcmp(argv[i],"-fromrealspace")) fromrealspace=1;
        else if (!strcmp(argv[i],"-getBkinds")) getBkinds=1;
        else if (!strcmp(argv[i],"-n_thread")) omp_set_num_threads(atoi(argv[++i]));
        else if (!strcmp(argv[i],"-filenameBk")) filenameBk = argv[++i];
        else if (!strcmp(argv[i],"-filenameBkind")) filenameBkind = argv[++i];
        else {
            std::cout<<std::endl<<"Error in arguments"<<std::endl;
            assert(0);
        }
		i++;
    }
    assert(nside>0);
    assert(((nside+1)%2)&&"Only even sides accepted.");

    if (!quiet){
        std::cout<<"Starting Program"<<std::endl;
        std::cout<<"--------------"<<std::endl;
        std::cout<<std::endl<<"nside="<<nside<<std::endl;
        if (fromrealspace) std::cout<<"Will compute from realspace: "<<filename<<std::endl;
        else std::cout<<"From filenamek="<<filename<<std::endl;
    }

    /*------Start----*/
    timer("Start program at: ",quiet);

    int size=nside*nside*nside;
    int csize=nside*nside*(nside/2+1);

    fftw_complex *delta_k;
    delta_k = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*csize);

    if (fromrealspace){
        timer("Start Fourier Transform: ",quiet);
        double *delta;
        delta = (double*) fftw_malloc(sizeof(double)*size);
        std::ifstream filer;
        filer.open(filename);
        filer.read((char*)delta, sizeof(double)*size);
        filer.close();

        #ifdef _OPENMP
        fftw_init_threads();
        #endif
        fftw_plan p;
        #ifdef _OPENMP
        fftw_plan_with_nthreads(omp_get_max_threads());
        #endif
        p = fftw_plan_dft_r2c_3d(nside,nside,nside, delta, delta_k,FFTW_ESTIMATE);
        fftw_execute(p);
        fftw_destroy_plan(p);
        free(delta);
    }
    else{
        std::ifstream delta_k_in;
        delta_k_in.open(filename);
        delta_k_in.read((char*)delta_k,sizeof(fftw_complex)*csize);
        delta_k_in.close();
    }

    timer("Start Bk: ",quiet);
    double* Bk=new double[flatsize(nside/2)];
    int* Bkind;
    if (!getBkinds){
        Bkind=getBk_ind(nside);
    }
    else{
        assert(0&&("Not Implemented"));
    }
    memset(Bk, 0,flatsize(nside/2)*sizeof(double));
    getBk_naive(Bk,delta_k,nside);
    timer("Start Bk write at: ",quiet);
    std::ofstream fileb;
    fileb.open(filenameBk);
    fileb.write((char*)Bk, sizeof(double)*flatsize(nside/2));
    fileb.close();

    std::ofstream filebi;
    filebi.open(filenameBkind);
    filebi.write((char*)Bkind, sizeof(int)*3*flatsize(nside/2));
    filebi.close();
    free(Bk);

    if (!quiet){
        timer("\nRun successful. ",quiet);
        gettimeofday(&now,NULL);
        std::cout<<std::endl<<"--------------"<<std::endl;
        std::cout<<std::endl<<"Total Time: "<<now.tv_sec+0.000001*now.tv_usec-t0<<std::endl;
    }




	return 0;
}