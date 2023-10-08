#define _GNU_SOURCE
# include <stdio.h>
# include <unistd.h>
# include <float.h>
# include <numa.h>
# include <sched.h>
# include <pthread.h>
# include <stdatomic.h>

#define ARRAY_SIZE	100000000

int threads = 1;
int numa_of_threads = 0;
int numa_of_memory = 0;
int array_size = ARRAY_SIZE;

int numa_node_cpu[2][32] = {
        			{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47},
        			{16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}
    			  };

// int numa_node_cpu[2][32] = {
//         			{0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62},
//         			{1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63}
//     			  };

void usage(char *program){
    printf("Usage: \n");
	printf(" -s   <array_size>  array size, default is %d\n", ARRAY_SIZE);
    printf(" -t   <threads>  number of threads, default is 1\n");
	printf(" -nt  <numa_of_threads>  numa of threads, default is 0\n");
	printf(" -nm  <numa_of_memory>  numa of memory, default is 0\n");
    printf(" -h   display the help information\n");
}

// parsing command line parameters
void parse_args(int argc, char *argv[]){
    for (int i = 1; i < argc; ++i){
        if (strlen(argv[i]) == 2 && strcmp(argv[i], "-t") == 0){
            if(i+1 < argc){
                threads = atoi(argv[i+1]);
                if(threads <= 0){
                    printf("invalid numbers of thread\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read numbers of thread\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
		else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-s") == 0){
            if(i+1 < argc){
                array_size = atoi(argv[i+1]);
                if(array_size <= 0){
                    printf("invalid numbers of array_size\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read numbers of array_size\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 3 && strcmp(argv[i], "-nt") == 0){
            if(i+1 < argc){
                numa_of_threads = atoi(argv[i+1]);
				if(!(numa_of_threads == 0 || numa_of_threads == 1)){
                    printf("invalid numbers of numa_of_threads\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read numbers of numa_of_threads\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
		else if (strlen(argv[i]) == 3 && strcmp(argv[i], "-nm") == 0){
            if(i+1 < argc){
                numa_of_memory = atoi(argv[i+1]);
                if(!(numa_of_memory == 0 || numa_of_memory == 1)){
                    printf("invalid numbers of numa_of_memory\n");
                    exit(EXIT_FAILURE);
                }
                i++;
            }else {
                printf("cannot read numbers of numa_of_memory\n");
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (strlen(argv[i]) == 2 && strcmp(argv[i], "-h") == 0){
            usage(argv[0]);
            exit(EXIT_SUCCESS);
        }else {
            printf("invalid option: %s\n", argv[i]);
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

atomic_int i = ATOMIC_VAR_INIT(0);
 
// pin thread to one core
void pin_thread_to_core() {
	pthread_t thread = pthread_self();
    cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	int core_num = numa_node_cpu[numa_of_threads][atomic_fetch_add(&i, 1)];
	CPU_SET(core_num, &cpuset);
	if(pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0) {
		fprintf(stderr, "Failed to set thread affinity for thread %ld\n", thread);
	}
	fprintf(stdout, "pin thread to core %d\n", core_num);
}

void *memory_consumer(void* arg){
	pin_thread_to_core();

	double *a, *b, *c;
	a = (double*) numa_alloc_onnode(sizeof(double) * array_size, numa_of_memory);
	b = (double*) numa_alloc_onnode(sizeof(double) * array_size, numa_of_memory);
	c = (double*) numa_alloc_onnode(sizeof(double) * array_size, numa_of_memory);
	for (int j = 0; j < array_size; j++) {
		a[j] = 1.0;
	    b[j] = 2.0;
	    c[j] = 0.0;
	}

	while (1)
	{
		for (int j = 0; j < array_size; j++) {
			a[j] = b[j] + c[j] + a[j];
			c[j] = b[j] ;
			b[j] = a[j];
		}
	}

	numa_free(a, sizeof(double) * array_size);
	numa_free(b, sizeof(double) * array_size);
	numa_free(c, sizeof(double) * array_size);
}


int main(int argc, char *argv[]){
    parse_args(argc, argv);
	pthread_t thread_of_consumer[64];
	printf("array size is %d\nnumber of threads is %d\nnuma of threads is %d\nnuma of memory is %d\n", 
			array_size, threads, numa_of_threads, numa_of_memory);

	for(int j = 0; j < threads; ++j) {
		pthread_create(&thread_of_consumer[j], NULL, memory_consumer, &j);
	}

    for(int j = 0; j < threads; ++j) {
        pthread_join(thread_of_consumer[j], NULL);
    }

    return 0;
}
