#ifdef TEST
#include "main.h"

#define LINEAR  0
#define POLY    1
#define RBF     2
#define SIGMOID 3

const char *kernel_type_table[] = {"linear","polynomial","rbf","sigmoid"};

int m,msv;                         // training and test set sizes
lasvm_sparsevector_array_t X; // feature vectors for test set
lasvm_sparsevector_array_t Xsv;// feature vectors for SVs
int_array_t Y;                   // labels
float_array_t alpha;            // alpha_i, SV weights
float b0;                        // threshold
int use_b0=1;                     // use threshold via constraint \sum a_i y_i =0
int kernel_type=RBF;              // LINEAR, POLY, RBF or SIGMOID kernels
float degree=3,kgamma=0.005,coef0=0;// kernel params
float_array_t x_square;         // norms of input vectors, used for RBF
float_array_t xsv_square;        // norms of test vectors, used for RBF
//char split_file_name[1024]="\0";         // filename for the splits
int binary_files=0;
ID_array_t splits;
int max_index=0;
int reporting_interval=100;

int split_file_load(char *f)
{
    int binary_file=0,labs=0,inds=0;
    FILE *fp;
    fp=fopen(f,"r");

    if(fp==NULL) {
        printf("[couldn't load split file: %s]\r\r\n",f);
        exit(1);
    }
    char dummy[100],dummy2[100];
    unsigned int i,j=0;
    for(i=0; i<strlen(f); i++) if(f[i]=='/') j=i+1;
    fscanf(fp,"%s %s",dummy,dummy2);
    strcpy(&(f[j]),dummy2);

    fscanf(fp,"%s %d",dummy,&binary_file);
    fscanf(fp,"%s %d",dummy,&inds);
    fscanf(fp,"%s %d",dummy,&labs);
    printf("[split file: load:%s binary:%d new_indices:%d new_labels:%d]\r\r\n",dummy2,binary_file,inds,labs);
    //printf("[split file:%s binary=%d]\r\r\n",dummy2,binary_file);
    if(!inds) return binary_file;
    ID ID;
    while(1)
    {
        int i,j;
        int c=fscanf(fp,"%d",&i);
        if(labs) c=fscanf(fp,"%d",&j);
        ID.x = i-1;
        if(c==-1) break;
        if (labs)
        {
            ID.y = j;
            //splits.id_insert_(ID(i-1,j));
        	IDInsertArray(&splits,ID);
        }
        else
        {
        	ID.y = 0;
            //splits.push_back(ID(i-1,0));
        	IDInsertArray(&splits,ID);
        }
    }

    qsort(splits.array,splits.used,sizeof(ID),&compareIDs);

    return binary_file;
}


int libsvm_load_data(char *filename)
// loads the same format as LIBSVM
{
    int index;
    float value;
    int elements, i;
    FILE *fp = fopen(filename,"r");
    lasvm_sparsevector_t* v;

    if(fp == NULL)
    {
        fprintf(stderr,"Can't open input file \"%s\"\r\r\n",filename);
        exit(1);
    }
    else
        printf("loading \"%s\"..  \r\r\n",filename);
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
                    //X.push_back(v);
                    sparsevectorInsertArray(&X,v);
                    splitpos++;
                }
            }
            else
            {
                v=lasvm_sparsevector_create();
                //X.push_back(v);
                sparsevectorInsertArray(&X,v);
            }
            ++msz;
            //printf("%d\r\n",m);
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


    max_index = 0;
    splitpos=0;
    for(i=0; i<msz; i++)
    {

        int write=0;
        if(splits.used>0)
        {
            if(splitpos<(int)splits.used && splits.array[splitpos].x==i)
            {
                write=2;
                splitpos++;
            }
        }
        else
            write=1;

        int label;
        fscanf(fp,"%d",&label);
        //  printf("%d %d\r\n",i,label);
        if(write)
        {
            if(splits.used>0)
            {
                if(splits.array[splitpos-1].y!=0)
                    //Y.push_back(splits[splitpos-1].y);
                	intInsertArray(&Y,splits.array[splitpos-1].y);
                else
                    //Y.push_back(label);
                	intInsertArray(&Y,label);
            }
            else
                //Y.push_back(label);
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
            fscanf(fp,"%d:%f",&index,&value);

            if (write==1) lasvm_sparsevector_set(X.array[m+i],index,value);
            if (write==2) lasvm_sparsevector_set(X.array[splitpos-1],index,value);
            if (index>max_index) max_index=index;
        }
out2:
        label=1; // dummy
    }

    fclose(fp);

    msz=X.used-m;
    printf("examples: %d   features: %d\r\r\n",msz,max_index);

    return msz;
}


int binary_load_data(char *filename)
{
    int msz,i=0,j;
    lasvm_sparsevector_t* v;
    int nonsparse=0;

    FILE * pFile;

    pFile = fopen(filename,"r");

    // read number of examples and number of features
    int sz[2];

    fread((char*)sz,sizeof(int),2,pFile);

    msz=sz[0];
    max_index=sz[1];

    float_array_t val;
    floatInitArray(&val,max_index);
    int_array_t   ind;
    intInitArray(&ind,1);
    if(max_index>0) nonsparse=1;
    int splitpos=0;

    for(i=0; i<msz; i++)
    {
        int mwrite=0;
        if(splits.used>0)
        {
            if(splitpos<(int)splits.used && splits.array[splitpos].x==i)
            {
                mwrite=1;
                splitpos++;
                v=lasvm_sparsevector_create();
                //X.push_back(v);
                sparsevectorInsertArray(&X,v);
            }
        }
        else
        {
            mwrite=1;
            v=lasvm_sparsevector_create();
           // X.push_back(v);
            sparsevectorInsertArray(&X,v);
        }

        if(nonsparse) // non-sparse binary file
        {
        	fread((char*)sz,sizeof(int),1,pFile);
            if(mwrite)
            {
                if(splits.used>0 && splits.array[splitpos-1].y!=0)
                    //Y.push_back(splits[splitpos-1].y);
                	intInsertArray(&Y,splits.array[splitpos-1].y);
                else
                    //Y.push_back(sz[0]);
                	intInsertArray(&Y,sz[0]);
            }
            fread((char*)(&val.array[0]),sizeof(float),max_index,pFile);
            if(mwrite)
                for(j=0; j<max_index; j++) // set features for each example
                    lasvm_sparsevector_set(v,j,val.array[j]);
        }
        else            // sparse binary file
        {
            fread((char*)sz,sizeof(int),2,pFile); // get label & sparsity of example i
            if(mwrite)
            {
                if(splits.used>0 && splits.array[splitpos-1].y!=0)
                    //Y.push_back(splits[splitpos-1].y);
                	intInsertArray(&Y,splits.array[splitpos-1].y);
                else
                   // Y.push_back(sz[0]);
                	intInsertArray(&Y,sz[0]);
            }
            floatResizeArray(&val,sz[1]);
            intResizeArray(&ind,sz[1]);

            fread((char*)(&ind.array[0]),sizeof(int),sz[1],pFile);
            fread((char*)(&val.array[0]),sizeof(float),sz[1],pFile);
            /*val.resize(sz[1]);
            ind.resize(sz[1]);
            f.read((char*)(&ind[0]),sz[1]*sizeof(int));
            f.read((char*)(&val[0]),sz[1]*sizeof(float));*/
            if(mwrite)
                for(j=0; j<sz[1]; j++) // set features for each example
                {
                    lasvm_sparsevector_set(v,ind.array[j],val.array[j]);
                    //printf("%d=%g\r\n",ind[j],val[j]);
                    if(ind.array[j]>max_index) max_index=ind.array[j];
                }
        }
    }
    fclose(pFile);

    msz=X.used-m;
    printf("examples: %d   features: %d\r\r\n",msz,max_index);

    return msz;
}


void load_data_file(char *filename)
{
    int msz,i,ft;
    //splits.resize(0);
    IDResizeArray(&splits,0);

    int bin=binary_files;
    if(bin==0) // if ascii, check if it isn't a split file..
    {
        FILE *f=fopen(filename,"r");
        if(f == NULL)
        {
            fprintf(stderr,"Can't open input file \"%s\"\r\r\n",filename);
            exit(1);
        }
        char c;
        fscanf(f,"%c",&c);
        if(c=='f') bin=2; // found split file!
    }

    switch(bin)  // load diferent file formats
    {
    case 0: // libsvm format
        msz=libsvm_load_data(filename);
        break;
    case 1:
        msz=binary_load_data(filename);
        break;
    case 2:
        ft=split_file_load(filename);
        if(ft==0)
        {
            msz=libsvm_load_data(filename);
            break;
        }
        else
        {
            msz=binary_load_data(filename);
            break;
        }
    default:
        fprintf(stderr,"Illegal file type '-B %d'\r\r\n",bin);
        exit(1);
    }

    if(kernel_type==RBF)
    {
        //x_square.resize(m+msz);
    	floatResizeArray(&x_square,m+msz);
        for(i=0; i<msz; i++)
            //x_square.array[i+m]=lasvm_sparsevector_dot_product(X.array[i+m],X.array[i+m]);
        	floatInsertArray(&x_square,lasvm_sparsevector_dot_product(X.array[i+m],X.array[i+m]));
    }

    if(kgamma==-1)
        kgamma=1.0/ ((float) max_index); // same default as LIBSVM

    m+=msz;
}


void libsvm_load_sv_data(FILE *fp)
// loads the same format as LIBSVM
{
    int max_index;
    int oldindex=0;
    int index;
    float value;
    int i;
    lasvm_sparsevector_t* v;

   // alpha.resize(msv);
    floatResizeArray(&alpha,msv);
    for(i=0; i<msv; i++)
    {
        v=lasvm_sparsevector_create();
        sparsevectorInsertArray(&Xsv,v);
    }

    max_index = 0;
    for(i=0; i<msv; i++)
    {
        float label;
        fscanf(fp,"%f",&label);
        //printf("%d:%g\r\n",i,label);
        alpha.array[i] = label;
        while(1)
        {
            int c;
            do {
                c = getc(fp);
                if(c=='\n') goto out2;
            } while(isspace(c));
            ungetc(c,fp);
            fscanf(fp,"%d:%f",&index,&value);
            if(index!=oldindex)
            {
                lasvm_sparsevector_set(Xsv.array[i],index,value);
            }
            oldindex=index;
            if (index>max_index) max_index=index;
        }
out2:
        label=1; // dummy
    }

    printf("loading model: %d svs\r\n",msv);

    if(kernel_type==RBF)
    {
        //xsv_square.resize(msv);
        floatResizeArray(&xsv_square,msv);
        for(i=0; i<msv; i++)
            xsv_square.array[i]=lasvm_sparsevector_dot_product(Xsv.array[i],Xsv.array[i]);
    }

}


int libsvm_load_model(const char *model_file_name)
// saves the model in the same format as LIBSVM
{
    int i;

    FILE *fp = fopen(model_file_name,"r");


    if(fp == NULL)
    {
        fprintf(stderr,"Can't open input file \"%s\"\r\n",model_file_name);
        exit(1);
    }

    static char tmp[1001];

    fscanf(fp,"%1000s",tmp); //svm_type
    fscanf(fp,"%1000s",tmp); //c_svc
    fscanf(fp,"%1000s",tmp); //kernel_type
    fscanf(fp,"%1000s",tmp); //rbf,poly,..

    kernel_type=LINEAR;
    for(i=0; i<4; i++)
        if (strcmp(tmp,kernel_type_table[i])==0) kernel_type=i;

    if(kernel_type == POLY)
    {
        fscanf(fp,"%1000s",tmp);
        fscanf(fp,"%f", &degree);
    }
    if(kernel_type == POLY || kernel_type == RBF || kernel_type == SIGMOID)
    {
        fscanf(fp,"%1000s",tmp);
        fscanf(fp,"%f",&kgamma);
    }
    if(kernel_type == POLY || kernel_type == SIGMOID)
    {
        fscanf(fp,"%1000s",tmp);
        fscanf(fp,"%f", &coef0);
    }

    fscanf(fp,"%1000s",tmp); // nr_class
    fscanf(fp,"%1000s",tmp); // 2
    fscanf(fp,"%1000s",tmp); // total_sv
    fscanf(fp,"%d",&msv);

    fscanf(fp,"%1000s",tmp); //rho
    fscanf(fp,"%f\r\n",&b0);

    fscanf(fp,"%1000s",tmp); // label
    fscanf(fp,"%1000s",tmp); // 1
    fscanf(fp,"%1000s",tmp); // -1
    fscanf(fp,"%1000s",tmp); // nr_sv
    fscanf(fp,"%1000s",tmp); // num
    fscanf(fp,"%1000s",tmp); // num
    fscanf(fp,"%1000s",tmp); // SV

    // now load SV data...

    libsvm_load_sv_data(fp);

    // finished!

    fclose(fp);
    return 0;
}


float kernel(int i, int j, void *kparam)
{
	float dot;
    dot=lasvm_sparsevector_dot_product(X.array[i],Xsv.array[j]);

    // sparse, linear kernel
    switch(kernel_type)
    {
    case LINEAR:
        return dot;
    case POLY:
        return pow(kgamma*dot+coef0,degree);
    case RBF:
        return exp(-kgamma*(x_square.array[i]+xsv_square.array[j]-2*dot));
    case SIGMOID:
        return tanh(kgamma*dot+coef0);
    }
    return 0;
}


void test(char *output_name)
{
    FILE *fp=fopen(output_name,"w");
    int i,j;
    float y;
    int y_th; // thresholded version of y
    float acc=0; // overall accuracy

    // rows = predictions (0 or 1)
    // cols = labels (0 or 1)
    int contingency_table[2][2] = {{0,0},{0,0}};

    for(i=0; i<m; i++) // iterate through trainig data
    {
        if (0 == i % reporting_interval)
        {
            printf("Processing training record #%d out of %d (%.4lf%% complete) ...\r\n", i, m, (float)(i)/m * 100);
        }

        // build linear combination of inner products with support vectors
        y=-b0;
        for(j=0; j<msv; j++)
        {
            y+=alpha.array[j]*kernel(i,j,NULL);
        }

        // prediction = threshold( linear combination )
        y_th = (y>=0) ? 1 : -1;

        // overall accuracy
        if(y_th==Y.array[i]) acc++;

        // update contingency table
        contingency_table[ (y_th>0)?1:0 ][ (Y.array[i]>0)?1:0 ]++;
    }

    // print to stdout
    printf("accuracy: %g%% (%d/%d)\r\n",(acc/m)*100,((int)acc),m);
    printf("contingency table:\r\n");
    printf("%10s%10s%10s%10s\r\n","","","labels","");
    printf("%10s%10s%10s%10s\r\n","","","-1","+1");
    printf("%10s%10s%10d%10d\r\n","pred","-1",contingency_table[0][0],contingency_table[0][1]);
    printf("%10s%10s%10d%10d\r\n","","+1",contingency_table[1][0],contingency_table[1][1]);

    // print to file
    fprintf(fp,"accuracy: %g%% (%d/%d)\r\n",(acc/m)*100,((int)acc),m);
    fprintf(fp,"contingency table:\r\n");
    fprintf(fp,"%10s%10s%10s%10s\r\n","","","labels","");
    fprintf(fp,"%10s%10s%10s%10s\r\n","","","-1","+1");
    fprintf(fp,"%10s%10s%10d%10d\r\n","pred","-1",contingency_table[0][0],contingency_table[0][1]);
    fprintf(fp,"%10s%10s%10d%10d\r\n","","+1",contingency_table[1][0],contingency_table[1][1]);

    fclose(fp);
}


void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name, char *output_file_name)
{
    int i;

    // parse options
    for(i=1; i<argc; i++)
    {
        if(argv[i][0] != '-') break;
        ++i;
        switch(argv[i-1][1])
        {
        case 'B':
            binary_files=atoi(argv[i]);
            break;
        default:
            fprintf(stderr,"unknown option\r\n");
        }
    }

    // determine filenames

    strcpy(input_file_name, argv[i]);

    if(i<argc-1)
        strcpy(model_file_name,argv[i+1]);
    else
    {
        char *p = strrchr(argv[i],'/');
        if(p==NULL)
            p = argv[i];
        else
            ++p;
        sprintf(model_file_name,"%s.model",p);
    }

    strcpy(input_file_name, argv[i]);
    strcpy(model_file_name, argv[i+1]);
    strcpy(output_file_name, argv[i+2]);

}


int main(void)
{
	sparsevectorInitArray(&X,1);
	sparsevectorInitArray(&Xsv,1);
	intInitArray(&Y, 1);
	floatInitArray(&alpha, 1);
	floatInitArray(&x_square, 1);
	floatInitArray(&xsv_square, 1);
	IDInitArray(&splits,1);

    printf("\r\n");
    printf("la test\r\n");
    printf("_______\r\n");

    libsvm_load_model("model.model");// load model

    load_data_file("test.trn"); // load test data

    test("result.txt");
}
#endif
