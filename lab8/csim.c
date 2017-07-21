/*
 * NAME:Wu Xinyue
 * ID:515030910231
 */

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "getopt.h"
#include "cachelab.h"

#define POW2(x) (1<<(x))
#define ADDR_BIT 64

typedef unsigned long long bit64;

/* enum status_t - the status of cache access. */
typedef enum status{MISS,HIT,MISS_EVIC}status_t;

/* struct cline - cache line: include update time, valid bit, tag bit and block. */
typedef struct cline{
	/* store latest update time for LRU strategy. */
	bit64 clock;

	/* 0 represents no valid information, 1 represents valid information.*/
	int valid;
	/* save tag information. */
	bit64 tag;
}cline_t;

/* struct set - cache set: maintain time counter and pointer to cache lines. */
typedef struct set{
	cline_t *lines;
	/* count time for LRU strategy. */
	bit64 count;
}set_t;

/* struct cache - save cache information. */
typedef struct cache{
	int s;
	int E;
	int b;
	/* points to cache sets. */
	set_t *sets;
}cache_t;



/* cache_init - initialize the cache. */
cache_t *cache_init(int s, int E, int b){
	cache_t *cache;
	cache = (cache_t *)malloc(sizeof(cache_t));

	cache->s = s;
	cache->E = E;
	cache->b = b;

	int nset = POW2(s);
	cache->sets = (set_t *)malloc(sizeof(set_t)*nset);

	int i;
	int j;
	for(i=0;i<nset;++i){
		(cache->sets[i]).lines = (cline_t *)malloc(sizeof(cline_t)*E);
		(cache->sets[i]).count = 0;
		for(j=0;j<E;++j){
			(cache->sets[i]).lines[j].valid = 0;
			(cache->sets[i]).lines[j].clock = 0;
		}
	}
	return cache;
}

/* cache_free - free malloced block. */
void cache_free(cache_t *cache){
	int nset = POW2(cache->s);
	int i;
	for(i=0;i<nset;++i){
		free(cache->sets[i].lines);
	}
	free(cache->sets);
	free(cache);
}



/* get_status: get the status of the address in cache. */
status_t get_status(cache_t *cache, bit64 addr){
	bit64 tag = addr>>(cache->b+cache->s);
	bit64 set_off = (addr>>(cache->b))&((1<<cache->s)-1);
	/* Update time. */
	cache->sets[set_off].count++;
	set_t set = cache->sets[set_off];
	/* record invalid place for MISS condition. */
	int invalid = 0, inv_pos = 0;
	/* record LRU place for EVICTION condition. */
	int least = set.count, lst_pos = 0;
	int i;
	int E = cache->E;
	for(i=0;i<E;++i){
		if(set.lines[i].valid&&set.lines[i].tag==tag){
			/* HIT condition. */
			set.lines[i].clock = set.count;
			return HIT;
		}
		else if(!set.lines[i].valid){
			invalid++;
			inv_pos = i;
		}
		else if(!invalid&&set.lines[i].clock<least){
			least = set.lines[i].clock;
			lst_pos = i;
		}
	}

	/* MISS condition. */
	if(invalid){
		set.lines[inv_pos].valid = 1;
		set.lines[inv_pos].tag = tag;
		set.lines[inv_pos].clock = set.count;
		return MISS;
	}

	/* EVICTION condition. */
	set.lines[lst_pos].valid = 1;
	set.lines[lst_pos].tag = tag;
	set.lines[lst_pos].clock = set.count;
	return MISS_EVIC;
}


/* print_status: print the status. */
void print_status(status_t status){
	switch(status){
		case MISS:
			printf(" miss");
			break;
		case HIT:
			printf(" hit");
			break;
		case MISS_EVIC:
			printf(" miss eviction");
			break;
		default:
			printf("\nERROR: Undefined status.\n");
			exit(0);
	}
}


/* sim_run - run the cache simulator. */
void sim_run(int verbose, FILE *file, cache_t *cache){
	int hits = 0, misses = 0, evictions = 0;
	char op[2];
	bit64 addr;
	int size;
	while(fscanf(file,"%s%llx,%d",op,&addr,&size) == 3){
		if(op[0] != 'I'){
			if(verbose){
				printf("%c %llx,%d",op[0],addr,size);
			}
			int n = (op[0]=='M')?2:1;
			int i;
			for(i=0;i<n;++i){
				status_t status = get_status(cache,addr);
				if(verbose){
					print_status(status);
				}
				switch(status){
					case MISS:
						misses++;
						break;
					case HIT:
						hits++;
						break;
					case MISS_EVIC:
						misses++;
						evictions++;
						break;
					default:
						exit(0);
				}
			}
			if(verbose){
				printf("\n");
			}
		}
	}
	printSummary(hits,misses,evictions);
}


/* usage - print the usage of the cachesim. */
void usage(char *argv[]){
    printf("usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n",argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}


int main(int argc, char *argv[])
{
    int opt;
	char *filename = NULL;
	int s = 0;
	int E = 0;
	int b = 0;
	int verbose = 0;

	/* Parse the command line argument. */
    while((opt=getopt(argc, argv, "hvs:E:b:t:"))!=-1){
		switch(opt){
			case 'v':
				verbose = 1;
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				filename = optarg;
				break;
			default:
				usage(argv);
				return 0;
		}
    }

	/* Missing required command line argument cases. */
	if(s==0||E==0||b==0||filename==NULL){
		printf("%s: Missing required command line argument\n",argv[0]);
		usage(argv);
		return 0;
	}

	FILE *file = fopen(filename,"r");
	
	/* Wrong file case. */
	if(file==NULL){
		printf("%s: No such file or directory\n",filename);
		usage(argv);
		return 0;
	}
	cache_t *cache = cache_init(s,E,b);
	sim_run(verbose,file,cache);
	cache_free(cache);
    return 0;
}
