#include "main.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "stdio.h"

#include "dyn_arrays.h"
#include "vector.h"
#include "lasvm.h"

#define LINEAR  0
#define POLY    1
#define RBF     2
#define SIGMOID 3

#define ONLINE 0
#define ONLINE_WITH_FINISHING 1

#define RANDOM 0
#define GRADIENT 1
#define MARGIN 2

#define ITERATIONS 0
#define SVS 1
#define TIME 2

const char *kernel_type_table[] = {"linear","polynomial","rbf","sigmoid"};

/* Data and model */
int m=0;                          // training set size
lasvm_sparsevector_array_t X; // feature vectors
int_array_t Y;                   // labels
float_array_t kparam;           // kernel parameters
float_array_t alpha;            // alpha_i, SV weights
float b0;                        // threshold

/* Hyperparameters */
int kernel_type=RBF;              // LINEAR, POLY, RBF or SIGMOID kernels
float degree=3,kgamma=0.005,coef0=0;// kernel params
int use_b0=1;                     // use threshold via constraint \sum a_i y_i =0
int selection_type=RANDOM;        // RANDOM, GRADIENT or MARGIN selection strategies
int optimizer = ONLINE_WITH_FINISHING; // strategy of optimization
float C=1;                       // C, penalty on errors
float C_neg=1;                   // C-Weighting for negative examples
float C_pos=1;                   // C-Weighting for positive examples
int epochs=1;                     // epochs of online learning
int candidates=50;				  // number of candidates for "active" selection process
float deltamax=1000;			  // tolerance for performing reprocess step, 1000=1 reprocess only
float_array_t select_size;      // Max number of SVs to take with selection strategy (for early stopping)
float_array_t x_square;         // norms of input vectors, used for RBF

/* Programm behaviour*/
int verbosity=0;                  // verbosity level, 0=off
int saves=1;
char report_file_name[1024];             // filename for the training report
char split_file_name[1024]="\0";         // filename for the splits
int cache_size=32;                       // 32Kb cache size as default
float epsgr=1e-3;                       // tolerance on gradients
long long kcalcs=0;                      // number of kernel evaluations
int binary_files=0;
ID_array_t splits;
int max_index=0;
int_array_t iold, inew;		  // sets of old (already seen) points + new (unseen) points
int termination_type=0;

int sv1,sv2;
float max_alpha,alpha_tol;

int libsvm_load_data(char *filename)
// loads the same format as LIBSVM
{
    int index; double value;
    int elements, i;
    FILE *fp = fopen(filename,"r");
    lasvm_sparsevector_t* v;

    if(fp == NULL)
    {
        fprintf(stderr,"Can't open input file \"%s\"\n",filename);
        exit(1);
    }
    else
        printf("loading \"%s\"..  \n",filename);
    int splitpos=0;

    int msz = 0;
    elements = 0;
    while(1)
    {
        int c = fgetc(fp);
        switch(c)
        {
        case '\n':
            if(splits.used>0)
            {
                if(splitpos<(int)splits.used && splits.array[splitpos].x==msz)
                {
                    v=lasvm_sparsevector_create();
                    sparsevectorInsertArray(&X,v);
                    splitpos++;
                }
            }
            else
            {
                v=lasvm_sparsevector_create();
                sparsevectorInsertArray(&X,v);
            }
            ++msz;
            //printf("%d\n",m);
            elements=0;
            break;
        case ':':
            ++elements;
            break;
        case EOF:
            goto out;
        default:
            ;
        }
    }
 out:
    rewind(fp);


    max_index = 0;splitpos=0;
    for(i=0;i<msz;i++)
    {

        int write=0;
        if(splits.used>0)
        {
            if(splitpos<(int)splits.used && splits.array[splitpos].x==i)
            {
                write=2;splitpos++;
            }
        }
        else
            write=1;

        int label;
        fscanf(fp,"%d",&label);
        //	printf("%d %d\n",i,label);
        if(write)
        {
            if(splits.used>0)
            {
                if(splits.array[splitpos-1].y!=0)
                	intInsertArray(&Y,splits.array[splitpos-1].y);
                else
                	intInsertArray(&Y,label);
            }
            else
            	intInsertArray(&Y,label);
        }

        while(1)
        {
            int c;
            do {
                c = getc(fp);
                if(c=='\n') goto out2;
            } while(isspace(c));
            ungetc(c,fp);
            fscanf(fp,"%d:%lf",&index,&value);

            if (write==1) lasvm_sparsevector_set(X.array[m+i],index,value);
            if (write==2) lasvm_sparsevector_set(X.array[splitpos-1],index,value);
            if (index>max_index) max_index=index;
        }
    out2:
        label=1; // dummy
    }

    fclose(fp);

    msz=X.used-m;
    printf("examples: %d   features: %d\n",msz,max_index);

    return msz;
}

void load_data_file(char *filename)
{
    int msz,i;
    IDResizeArray(&splits,1);

    int bin=binary_files;
    if(bin==0) // if ascii, check if it isn't a split file..
    {
        FILE *f=fopen(filename,"r");
        if(f == NULL)
        {
            fprintf(stderr,"Can't open input file \"%s\"\n",filename);
            exit(1);
        }
        char c; fscanf(f,"%c",&c);
        if(c=='f') bin=2; // found split file!
    }

    switch(bin)  // load different file formats
    {
    case 0: // libsvm format
        msz=libsvm_load_data(filename); break;
    //case 1:
        //msz=binary_load_data(filename); break;
    //case 2:
        //ft=split_file_load(filename);
        //if(ft==0)
        //{msz=libsvm_load_data(filename); break;}
        //else
        //{msz=binary_load_data(filename); break;}
    default:
        fprintf(stderr,"Illegal file type '-B %d'\n",bin);
        exit(1);
    }

    if(kernel_type==RBF)
    {
        floatResizeArray(&x_square,m+msz);
        for(i=0;i<msz;i++)
        	floatInsertArray(&x_square,lasvm_sparsevector_dot_product(X.array[i+m],X.array[i+m]));
    }

    if(kgamma==-1)
        kgamma=1.0/ ((double) max_index); // same default as LIBSVM

    m+=msz;
}

int count_svs()
{
    int i;
    max_alpha=0;
    sv1=0;sv2=0;

    for(i=0;i<m;i++) 	// Count svs..
    {
        if(alpha.array[i]>max_alpha) max_alpha=alpha.array[i];
        if(-alpha.array[i]>max_alpha) max_alpha=-alpha.array[i];
    }

    alpha_tol=max_alpha/1000.0;

    for(i=0;i<m;i++)
    {
        if(Y.array[i]>0)
        {
            if(alpha.array[i] >= alpha_tol) sv1++;
        }
        else
        {
            if(-alpha.array[i] >= alpha_tol) sv2++;
        }
    }
    return sv1+sv2;
}

int libsvm_save_model(const char *model_file_name)
    // saves the model in the same format as LIBSVM
{
    FILE *fp = fopen(model_file_name,"w");
    if(fp==NULL) return -1;

    count_svs();

    // printf("nSV=%d\n",sv1+sv2);

    fprintf(fp,"svm_type c_svc\n");
    fprintf(fp,"kernel_type %s\n", kernel_type_table[kernel_type]);

    if(kernel_type == POLY)
        fprintf(fp,"degree %g\n", degree);

    if(kernel_type == POLY || kernel_type == RBF || kernel_type == SIGMOID)
        fprintf(fp,"gamma %g\n", kgamma);

    if(kernel_type == POLY || kernel_type == SIGMOID)
        fprintf(fp,"coef0 %g\n", coef0);

    fprintf(fp, "nr_class %d\n",2);
    fprintf(fp, "total_sv %d\n",sv1+sv2);

    {
        fprintf(fp, "rho %g\n",b0);
    }

    fprintf(fp, "label 1 -1\n");
    fprintf(fp, "nr_sv");
    fprintf(fp," %d %d",sv1,sv2);
    fprintf(fp, "\n");
    fprintf(fp, "SV\n");

    for(int j=0;j<2;j++)
        for(int i=0;i<m;i++)
        {
            if (j==0 && Y.array[i]==-1) continue;
            if (j==1 && Y.array[i]==1) continue;
            if (alpha.array[i]*Y.array[i]< alpha_tol) continue; // not an SV

            fprintf(fp, "%.16g ",alpha.array[i]);

            lasvm_sparsevector_pair_t *p1 = X.array[i]->pairs;
            while (p1)
            {
                fprintf(fp,"%d:%.8g ",p1->index,p1->data);
                p1 = p1->next;
            }
            fprintf(fp, "\n");
        }

    fclose(fp);
    return 0;
}

float kernel(int i, int j, void *kparam)
{
    double dot;
    kcalcs++;
    dot=lasvm_sparsevector_dot_product(X.array[i],X.array[j]);

    // sparse, linear kernel
    switch(kernel_type)
    {
    case LINEAR:
        return dot;
    case POLY:
        return pow(kgamma*dot+coef0,degree);
    case RBF:
        return exp(-kgamma*(x_square.array[i]+x_square.array[j]-2*dot));
    case SIGMOID:
        return tanh(kgamma*dot+coef0);
    }
    return 0;
}

void finish(lasvm_t *sv)
{

    if (optimizer==ONLINE_WITH_FINISHING)
    {
        fprintf(stdout,"..[finishing]");

        int iter=0;

        do {
            iter += lasvm_finish(sv, epsgr);
        } while (lasvm_get_delta(sv)>epsgr);

    }

    int l=(int) lasvm_get_l(sv);
    int_array_t svind;
    intInitArray(&svind,l);
    int svs=lasvm_get_sv(sv,svind.array);
    floatFreeArray(&alpha);
    floatInitArray(&alpha,m);

    float_array_t svalpha;
    floatInitArray(&svalpha,l);
    lasvm_get_alpha(sv,svalpha.array);
    for(int i=0;i<svs;i++)
    	alpha.array[svind.array[i]]=svalpha.array[i];
    b0=lasvm_get_b(sv);
}

void make_old(int val)
    // move index <val> from new set into old set
{
    int i,ind=-1;
    for(i=0;i<(int)inew.used;i++)
    {
        if(inew.array[i]==val) {ind=i; break;}
    }

    if (ind>=0)
    {
        inew.array[ind]=inew.array[inew.used-1];
        inew.used = inew.used -1;
        intInsertArray(&iold,val);
    }
}

int select(lasvm_t *sv) // selection strategy
{
    int s=-1;
    int t,i,r,j;
    double tmp,best; int ind=-1;

    switch(selection_type)
    {
    case RANDOM:   // pick a random candidate
        s=rand() % inew.used;
        break;

    case GRADIENT: // pick best gradient from 50 candidates
        j=candidates; if((int)inew.used<j) j=inew.used;
        r=rand() % inew.used;
        s=r;
        best=1e20;
        for(i=0;i<j;i++)
        {
            r=inew.array[s];
            tmp=lasvm_predict(sv, r);
            tmp*=Y.array[r];
            //printf("%d: example %d   grad=%g\n",i,r,tmp);
            if(tmp<best) {best=tmp;ind=s;}
            s=rand() % inew.used;
        }
        s=ind;
        break;

    case MARGIN:  // pick closest to margin from 50 candidates
        j=candidates; if((int)inew.used<j) j=inew.used;
        r=rand() % inew.used;
        s=r;
        best=1e20;
        for(i=0;i<j;i++)
        {
            r=inew.array[s];
            tmp=lasvm_predict(sv, r);
            if (tmp<0) tmp=-tmp;
            //printf("%d: example %d   grad=%g\n",i,r,tmp);
            if(tmp<best) {best=tmp;ind=s;}
            s=rand() % inew.used;
        }
        s=ind;
        break;
    }

    t=inew.array[s];
    inew.array[s]=inew.array[inew.used-1];
    inew.used = inew.used -1;
    intInsertArray(&iold,t);

    //printf("(%d %d)\n",iold.used,inew.used);

    return t;
}

void train_online(char *model_file_name)
{
    int t1,t2=0,i,s,l,j,k;
    char t[1000];
    strcpy(t,model_file_name);
    strcat(t,".time");

    lasvm_kcache_t *kcache=lasvm_kcache_create(kernel, NULL);
    lasvm_kcache_set_maximum_size(kcache, cache_size*1024*1024);
    lasvm_t *sv=lasvm_create(kcache,use_b0,C*C_pos,C*C_neg);
    printf("set cache size %d\n",cache_size);

    // everything is new when we start
    for(i=0;i<m;i++)
    	intInsertArray(&inew,i);

    // first add 5 examples of each class, just to balance the initial set
    int c1=0,c2=0;
    for(i=0;i<m;i++)
    {
        if(Y.array[i]==1 && c1<5) {lasvm_process(sv,i,(double) Y.array[i]); c1++; make_old(i);}
        if(Y.array[i]==-1 && c2<5){lasvm_process(sv,i,(double) Y.array[i]); c2++; make_old(i);}
        if(c1==5 && c2==5) break;
    }

    for(j=0;j<epochs;j++)
    {
        for(i=0;i<m;i++)
        {
            if(inew.used==0) break; // nothing more to select
            s=select(sv);            // selection strategy, select new point

            t1=lasvm_process(sv,s,(float) Y.array[s]);

            if (deltamax<=1000) // potentially multiple calls to reprocess..
            {
                //printf("%g %g\n",lasvm_get_delta(sv),deltamax);
                t2=lasvm_reprocess(sv,epsgr);// at least one call to reprocess
                while (lasvm_get_delta(sv)>deltamax && deltamax<1000)
                {
                    t2=lasvm_reprocess(sv,epsgr);
                }
            }

            if (verbosity==2)
            {
                l=(int) lasvm_get_l(sv);
                printf("l=%d process=%d reprocess=%d\n",l,t1,t2);
            }
            else
                if(verbosity==1)
                    if( (i%100)==0){ fprintf(stdout, "..%d",i); fflush(stdout); }

            l=(int) lasvm_get_l(sv);
            for(k=0;k<(int)select_size.used;k++)
            {
                if   ( (termination_type==ITERATIONS && i==select_size.array[k])
                       || (termination_type==SVS && l>=select_size.array[k]))
                {
                    if(saves>1) // if there is more than one model to save, give a new name
                    {
                        // save current version before potential finishing step
                        int save_l=(int)lasvm_get_l(sv);
                        float_array_t save_alpha;
                        floatInitArray(&save_alpha,l);
                        lasvm_get_alpha(sv,save_alpha.array);
                        float_array_t save_g;
                        floatInitArray(&save_g,l);
                        lasvm_get_g(sv,save_g.array);
                        int_array_t save_sv;
                        intInitArray(&save_sv,l);
                        lasvm_get_sv(sv,save_sv.array);

                        finish(sv);
                        char tmp[1000];

                        //f << i << " " << count_svs() << " " << kcalcs << " " << timer << endl;

                        if(termination_type==TIME)
                        {
                            sprintf(tmp,"%s_%dsecs",model_file_name,i);
                            fprintf(stdout,"..[saving model_%d secs]..",i);
                        }
                        else
                        {
                            fprintf(stdout,"..[saving model_%d pts]..",i);
                            sprintf(tmp,"%s_%dpts",model_file_name,i);
                        }
                        libsvm_save_model(tmp);

                        // get back old version
                        //fprintf(stdout, "[restoring before finish]"); fflush(stdout);
                        lasvm_init(sv, save_l, save_sv.array, save_alpha.array, save_g.array);
                        intFreeArray(&save_sv);
                        floatFreeArray(&save_alpha);
                        floatFreeArray(&save_g);
                    }
                    select_size.array[k]=select_size.array[select_size.used-1];
                    select_size.used = select_size.used - 1;
                }
            }
            if(select_size.used==0) break; // early stopping, all intermediate models saved
        }

        intResizeArray(&inew,1);
        intResizeArray(&iold,1);
        for(i=0;i<m;i++)
        	intInsertArray(&inew,i);
    }

    if(saves<2)
    {
        finish(sv); // if haven't done any intermediate saves, do final save
        //f << m << " " << count_svs() << " " << kcalcs << " " << timer << endl;
    }

    if(verbosity>0) printf("\n");
    l=count_svs();
    printf("nSVs=%d\n",l);
    printf("||w||^2=%g\n",lasvm_get_w2(sv));
    printf("kcalcs= %f",(float)kcalcs);
    //f.close();
    lasvm_destroy(sv);
    lasvm_kcache_destroy(kcache);
}

int main(int argc, char **argv)
{
    printf("\r\n");
    printf("la SVM\r\n");
    printf("______\r\n");

	sparsevectorInitArray(&X,1);

	intInitArray(&Y, 1);
	intInitArray(&iold, 1);
	intInitArray(&inew, 1);

	floatInitArray(&kparam, 1);
	floatInitArray(&alpha, 1);
	floatInitArray(&select_size, 1);
	floatInitArray(&x_square, 1);

	load_data_file("data.trn");

	train_online("model.dat");

	libsvm_save_model("model.dat");

}

