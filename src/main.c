#include "main.h"

/*HELP
            "Usage: la_svm [options] training_set_file [model_file]\n"
            "options:\n"
            "-B file format : files are stored in the following format:\n"
            "	0 -- libsvm ascii format (default)\n"
            "	1 -- binary format\n"
            "	2 -- split file format\n"
            "-o optimizer: set the type of optimization (default 1)\n"
            "	0 -- online \n"
            "	1 -- online with finishing step \n"
            "-t kernel_type : set type of kernel function (default 2)\n"
            "	0 -- linear: u'*v\n"
            "	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
            "	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
            "	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
            "-s selection: set the type of selection strategy (default 0)\n"
            "	0 -- random \n"
            "	1 -- gradient-based \n"
            "	2 -- margin-based \n"
            "-T termination: set the type of early stopping strategy (default 0)\n"
            "	0 -- number of iterations \n"
            "	1 -- number of SVs \n"
            "	2 -- time-based (not implemented) \n"
            "-l sample: number of iterations/SVs/seconds to sample for early stopping (default all)\n"
            " if a list of numbers is given a model file is saved for each element of the set\n"
            "-C candidates : set number of candidates to search for selection strategy (default 50)\n"
            "-d degree : set degree in kernel function (default 3)\n"
            "-g gamma : set gamma in kernel function (default 1/k)\n"
            "-r coef0 : set coef0 in kernel function (default 0)\n"
            "-c cost : set the parameter C of C-SVC\n"
            "-m cachesize : set cache memory size in KB (default 32)\n"
            "-wi weight: set the parameter C of class i to weight*C (default 1)\n"
            "-b bias: use a bias or not i.e. no constraint sum alpha_i y_i =0 (default 1=on)\n"
            "-e epsilon : set tolerance of termination criterion (default 0.001)\n"
            "-p epochs : number of epochs to train in online setting (default 1)\n"
            "-D deltamax : set tolerance for reprocess step, 1000=1 call to reprocess >1000=no calls to reprocess (default 1000)\n"
 */

/* Data and model */
const char *kernel_type_table[] = {"linear","polynomial","rbf","sigmoid"};
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
float C=100;                       // C, penalty on errors
float C_neg=1;                   // C-Weighting for negative examples
float C_pos=1;                   // C-Weighting for positive examples
int epochs=1;                     // epochs of online learning
int candidates=50;				  // number of candidates for "active" selection process
float deltamax=1000;			  // tolerance for performing reprocess step, 1000=1 reprocess only
float_array_t select_size;      // Max number of SVs to take with selection strategy (for early stopping)
float_array_t x_square;         // norms of input vectors, used for RBF

/* Programm behaviour*/
int verbosity=1;                  // verbosity level, 0=off
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


void error_Handler(int error)
{
	while(1)
	{
		for(int i = 0;i<error;i++)
		{
			HAL_GPIO_TogglePin(GPIOB,LED2_PIN);
			HAL_Delay(1000);
			HAL_GPIO_TogglePin(GPIOB,LED2_PIN);
		}
		HAL_Delay(5000);
	}
}

void SystemClock_Config()
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 0x10;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;

	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		error_Handler(CLOCK_ERROR);

	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
			RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_SYSCLK);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
		error_Handler(CLOCK_ERROR);

	while(SysTick_Config(SystemCoreClock/1000) != 0){}
}

int split_file_load(char *f)
{
	int binary_file=0,labs=0,inds=0;
	FILE *fp;
	fp=fopen(f,"r");
	if(fp==NULL) {printf("[couldn't load split file: %s]\n",f); exit(1);}
	char dummy[100],dummy2[100];
	unsigned int i,j=0; for(i=0;i<strlen(f);i++) if(f[i]=='/') j=i+1;
	fscanf(fp,"%s %s",dummy,dummy2);
	strcpy(&(f[j]),dummy2);
	fscanf(fp,"%s %d",dummy,&binary_file);
	fscanf(fp,"%s %d",dummy,&inds);
	fscanf(fp,"%s %d",dummy,&labs);
	printf("[split file: load:%s binary:%d new_indices:%d new_labels:%d]\n",dummy2,binary_file,inds,labs);
	//printf("[split file:%s binary=%d]\n",dummy2,binary_file);
	if(!inds) return binary_file;
	ID id;
	int m,n;
	while(1)
	{
		int c=fscanf(fp,"%d",&m);
		id.x = i-1;
		if(labs) c=fscanf(fp,"%d",&n);
		if(c==-1) break;
		if (labs)
		{
			id.y = j;
			//A splits.push_back(ID(i-1,j));
			IDInsertArray(&splits,id);
		}
		else
			id.y = 0;
		//A splits.push_back(ID(i-1,0));
		IDInsertArray(&splits,id);
	}


	//A sort(splits.array[0],splits.array[splits.used - 1]);
	IDResizeArray(&splits,splits.used);
	qsort(splits.array,splits.used,sizeof(ID),&compareIDs);

	return binary_file;
}

int libsvm_load_data(char *filename)
// loads the same format as LIBSVM
{
	int index; float value;
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
		if(splits.used > 0 )
		{
			if(splitpos<(int)splits.used && splits.array[splitpos].x==msz)
			{
				v=lasvm_sparsevector_create();
				//A X.push_back(v);	splitpos++;
				sparsevectorInsertArray(&X,v);
			}
		}
		else
		{
			v=lasvm_sparsevector_create();
			//A X.push_back(v);
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
					//A Y.push_back(splits[splitpos-1].y);
					intInsertArray(&Y,splits.array[splitpos-1].y);
				else
					//A Y.push_back(label);
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
			fscanf(fp,"%d:%f",&index,&value);

			if (write==1)
				lasvm_sparsevector_set(X.array[m+i],index,value);
			if (write==2)
				lasvm_sparsevector_set(X.array[splitpos-1],index,value);
			if (index>max_index)
				max_index=index;
		}
		out2:
		label=1; // dummy
	}

	fclose(fp);

	msz=X.used-m;
	printf("examples: %d   features: %d\n",msz,max_index);

	return msz;
}

int binary_load_data(char *filename)
{
	int msz,i=0,j;
	lasvm_sparsevector_t* v;
	int nonsparse=0;

	FILE *fp;
	fp = fopen(filename,"r");

	// read number of examples and number of features
	int sz[2];
	fread((char*)sz,sizeof(int),2,fp);
	if (!fp)
	{ printf("File writing error in line %d.\n",i); exit(1);}
	msz=sz[0]; max_index=sz[1];

	float_array_t val;
	int_array_t  ind;
	floatInitArray(&val,max_index);
	if(max_index>0) nonsparse=1;
	int splitpos=0;

	for(i=0;i<msz;i++)
	{
		int mwrite=0;
		if(splits.used>0)
		{
			if(splitpos<(int)splits.used && splits.array[splitpos].x==i)
			{
				mwrite=1;splitpos++;
				v=lasvm_sparsevector_create();
				//A X.push_back(v);
				sparsevectorInsertArray(&X,v);
			}
		}
		else
		{
			mwrite=1;
			v=lasvm_sparsevector_create();
			//A X.push_back(v);
			sparsevectorInsertArray(&X,v);
		}

		if(nonsparse) // non-sparse binary file
		{
			fread((char*)sz,sizeof(int),1,fp); // get label
			if(mwrite)
			{
				if(splits.used>0 && splits.array[splitpos-1].y!=0)
					//A Y.push_back(splits[splitpos-1].y);
					intInsertArray(&Y,splits.array[splitpos-1].y);
				else
					//A Y.push_back(sz[0]);
					intInsertArray(&Y,sz[0]);
			}
			fread((char*)(&val.array[0]),sizeof(float),max_index,fp);
			if(mwrite)
				for(j=0;j<max_index;j++) // set features for each example
					lasvm_sparsevector_set(v,j,val.array[j]);
		}
		else			// sparse binary file
		{
			fread((char*)sz,sizeof(int),2,fp); // get label & sparsity of example i
			if(mwrite)
			{//A
				if(splits.used>0 && splits.array[splitpos-1].y!=0)
					intInsertArray(&Y,splits.array[splitpos-1].y);
				else
					intInsertArray(&Y,sz[0]);
			}
			//A val.resize(sz[1]);
			floatResizeArray(&val,sz[1]);
			intResizeArray(&ind,sz[1]);
			fread((char*)(&ind.array[0]),sizeof(int),sz[1],fp);
			fread((char*)(&val.array[0]),sizeof(float),sz[1],fp);
			if(mwrite)
				for(j=0;j<sz[1];j++) // set features for each example
				{
					lasvm_sparsevector_set(v,ind.array[j],val.array[j]);
					//printf("%d=%g\n",ind[j],val[j]);
					if(ind.array[j]>max_index) max_index=ind.array[j];
				}
		}
	}
	fclose(fp);

	msz=X.used-m;
	printf("examples: %d   features: %d\n",msz,max_index);

	return msz;
}

void load_data_file(char *filename)
{
	int msz,i,ft;
	//A splits.resize(0);
	IDFreeArray(&splits);
	IDInitArray(&splits,1);


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

	switch(bin)  // load diferent file formats
	{
	case 0: // libsvm format
		msz=libsvm_load_data(filename); break;
	case 1:
		msz=binary_load_data(filename); break;
	case 2:
		ft=split_file_load(filename);
		if(ft==0)
		{msz=libsvm_load_data(filename); break;}
		else
		{msz=binary_load_data(filename); break;}
	default:
		fprintf(stderr,"Illegal file type '-B %d'\n",bin);
		exit(1);
	}

	if(kernel_type==RBF)
	{
		//A x_square.resize(m+msz);
		floatResizeArray(&x_square,m+msz);
		for(i=0;i<msz;i++)
			x_square.array[i+m]=lasvm_sparsevector_dot_product(X.array[i+m],X.array[i+m]);
	}

	if(kgamma==-1)
		kgamma=1.0/ ((float) max_index); // same default as LIBSVM

	m+=msz;
}

int count_svs()
{
	int i;
	max_alpha=0;
	sv1=0;sv2=0;

	for(i=0;i<m;i++) 	// Count svs..
	{
		if(alpha.array[i]>max_alpha)
			max_alpha=alpha.array[i];
		if(-alpha.array[i]>max_alpha)
			max_alpha=-alpha.array[i];
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
	float dot;
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
	int i,l;

	if (optimizer==ONLINE_WITH_FINISHING)
	{
		fprintf(stdout,"..[finishing]");

		int iter=0;

		do {
			iter += lasvm_finish(sv, epsgr);
		} while (lasvm_get_delta(sv)>epsgr);

	}

	l=(int) lasvm_get_l(sv);
	int svs;
	int svind[l];
	svs=lasvm_get_sv(sv,svind);
	//A alpha.resize(m);
	floatResizeArray(&alpha, m);
	for(i=0;i<m;i++)
		alpha.array[i]=0;
	float svalpha[l];
	lasvm_get_alpha(sv,svalpha);
	for(i=0;i<svs;i++)
		alpha.array[svind[i]]=svalpha[i];
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
		//A inew.pop_back();
		intResizeArray(&inew, inew.used-1);
		//A iold.push_back(val);
		intInsertArray(&iold,val);

	}
}

int selectstrategy(lasvm_t *sv)
{
	// selection strategy
	int s=-1;
	int t,i,r,j;
	float tmp,best; int ind=-1;

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
	//A inew.pop_back();
	intResizeArray(&inew, inew.used-1);
	//A iold.push_back(t);
	intInsertArray(&iold,t);

	//printf("(%d %d)\n",iold.used(),inew.used());

	return t;
}

void train_online(char *model_file_name)
{
	int t1,t2=0,i,s,l,j,k;

	char t[1000];
	strcpy(t,model_file_name);
	strcat(t,".time");

	lasvm_kcache_t *kcache=lasvm_kcache_create(kernel, NULL);
	lasvm_kcache_set_maximum_size(kcache, cache_size*1024);
	lasvm_t *sv=lasvm_create(kcache,use_b0,C*C_pos,C*C_neg);
	printf("set cache size %d\n",cache_size);

	// everything is new when we start
	for(i=0;i<m;i++)
		intInsertArray(&inew,i);

	// first add 5 examples of each class, just to balance the initial set
	int c1=0,c2=0;
	for(i=0;i<m;i++)
	{
		if(Y.array[i]==1 && c1<5) {lasvm_process(sv,i,(float) Y.array[i]); c1++; make_old(i);}
		if(Y.array[i]==-1 && c2<5){lasvm_process(sv,i,(float) Y.array[i]); c2++; make_old(i);}
		if(c1==5 && c2==5) break;
	}

	for(j=0;j<epochs;j++)
	{
		for(i=0;i<m;i++)
		{
			if(inew.used==0) break; // nothing more to select
			s=selectstrategy(sv);            // selection strategy, select new point

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
						|| (termination_type==SVS && l>=select_size.array[k])
						/*|| (termination_type==TIME && sw->get_time()>=select_size.array[k])*/
				)

				{
					if(saves>1) // if there is more than one model to save, give a new name
					{
						// save current version before potential finishing step
						int save_l=(int)lasvm_get_l(sv);
						float_array_t save_alpha;
						floatInitArray(&save_alpha,1);
						lasvm_get_alpha(sv,save_alpha.array);
						float_array_t save_g;
						floatInitArray(&save_g,1);
						lasvm_get_g(sv,save_g.array);
						int_array_t save_sv;
						intInitArray(&save_sv,1);
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
						//A delete save_alpha; delete save_sv; delete save_g;
						floatFreeArray(&save_alpha);
						intFreeArray(&save_sv);
						floatFreeArray(&save_g);
					}
					select_size.array[k]=select_size.array[select_size.used-1];
					//A select_size.pop_back();
					floatResizeArray(&select_size,select_size.used-1);
				}
			}
			if(select_size.used==0) break; // early stopping, all intermediate models saved
		}
		/* //A
        inew.resize(0);
        iold.resize(0); // start again for next epoch..
        for(i=0;i<m;i++)
        	inew.push_back(i); */
		intFreeArray(&inew);
		intFreeArray(&iold);
		intInitArray(&iold,1);
		intInitArray(&inew,1);
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
	printf("kcalcs= %llu",kcalcs);
	//f.close();
	lasvm_destroy(sv);
	lasvm_kcache_destroy(kcache);
}

void mainThread(void const *argument)
{
	printf("Incremental SVM algorithm on NUCLEO F401RE \r\n");

	char *input_file_name = "data.trn";
	char *model_file_name = "model.dat";

	load_data_file(input_file_name);

	train_online(model_file_name);

	libsvm_save_model(model_file_name);

	osThreadTerminate(NULL);
}

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	USART1_Init();
	USART2_Init();

	select_USART(TERMINAL);

	if(HAL_UART_Receive_IT(&UARTHandle1,(uint8_t *)aRxBuffer1,1) != HAL_OK)
		error_Handler(UART_ERROR);

	//Struct arrays initialization
	sparsevectorInitArray(&X,1);
	intInitArray(&Y,1);
	intInitArray(&iold,1);
	intInitArray(&inew,1);
	floatInitArray(&kparam,1);
	floatInitArray(&alpha,1);
	floatInitArray(&select_size,1);
	floatInitArray(&x_square,1);
	IDInitArray(&splits,1);

	//RTOS Thread initialization
	osThreadDef(main,mainThread,osPriorityNormal,0,100);
	osThreadCreate(osThread(main),NULL);
	osKernelStart();

	while(1);
}
