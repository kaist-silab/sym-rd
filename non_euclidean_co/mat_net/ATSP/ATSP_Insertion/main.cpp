
/*
The MIT License

Copyright (c) 2021 MatNet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FARTHEST_INSERTION 0
#define NEAREST_INSERTION 1

#define MODE FARTHEST_INSERTION

#define DATA_DIR "../data/n20/"
#define FILENAME_PREFIX "problem_20_0_1000000_"

#define PRINT_RESULT_TRAJ false
#define NUM_DATA 10000

#define MAX_LEN 100
#define MAX_LINE_BUFF 2048


unsigned int N;
unsigned int batch_dist[NUM_DATA][MAX_LEN][MAX_LEN];

bool load_data_file(char* filepath, int data_index)
{
	unsigned int (*dist)[MAX_LEN] = batch_dist[data_index];
	
	FILE* fp = fopen(filepath, "r");
	char buff[MAX_LINE_BUFF];
	
	if ( fp==NULL ) 
	{
		printf("Error: Can't open %s", filepath);
		return false;
	}

	fgets(buff, MAX_LINE_BUFF, fp);
	fgets(buff, MAX_LINE_BUFF, fp);

	N = atoi(buff+11);

	fgets(buff, MAX_LINE_BUFF, fp);
	fgets(buff, MAX_LINE_BUFF, fp);
	fgets(buff, MAX_LINE_BUFF, fp);

	for ( int i = 0 ; i < N; i ++)		
		for(int j = 0 ; j < N; j ++) 
			fscanf(fp, "%d", &dist[i][j]);
	
	fclose(fp);
	
	return true;
}

void print_data()
{
	unsigned int (*dist)[MAX_LEN] = batch_dist[0];

	printf("N=%d\n", N);
	
	for ( int i = 0 ; i < N ; i ++)
	{
		for ( int j = 0 ; j < N ; j ++) printf("%d\t", dist[i][j]);			
		printf("\n");
	}
}


unsigned int atsp_nearest_insertion(int data_index)
{
	unsigned int (*dist)[MAX_LEN] = batch_dist[data_index];

	bool visited[MAX_LEN];
	int next[MAX_LEN];
	
	for (int i = 0 ; i < N; i ++) 
	{
		visited[i] = false;
		next[i] = i;
	}

	visited[0] = true;

	for (int size=1; size<N; size++)
	{
		int min_cost_inc = 0x7FFFFFFF;
		int bs=-1, bm=-1, be=-1;
		
		for (int m = 0 ; m < N; m++)	// candidate node to insert
		{
			if (visited[m]) continue;

			int s = 0;

			for (int i =0; i<size; i++)
			{
				int e = next[s];
				
				// check
				int cost_inc = dist[s][m] + dist[m][e] - dist[s][e];
				if(cost_inc < min_cost_inc)
				{
					min_cost_inc = cost_inc;
					bs = s;
					bm = m;
					be = e;
				}
				
				// move
				s = next[s];
			}
		}

		// insert node
		next[bs] = bm;
		next[bm] = be;
		visited[bm] = true;
	}

	unsigned int total_len = 0;
	int ss = 0;

	for (int i = 0; i < N; i++)
	{
		total_len += dist[ss][next[ss]];
		
		#if PRINT_RESULT_TRAJ==true
		printf("%d->%d, %d\n", ss, next[ss], total_len);
		fflush(stdout);
		#endif

		ss = next[ss];
	}
	#if PRINT_RESULT_TRAJ==true
	printf("%n");
	#endif

	return total_len;
}


unsigned int atsp_farthest_insertion(int data_index)
{
	unsigned int (*dist)[MAX_LEN] = batch_dist[data_index];

	bool visited[MAX_LEN];
	int next[MAX_LEN];
	
	for (int i = 0 ; i < N; i ++) 
	{
		visited[i] = false;
		next[i] = i;
	}

	visited[0] = true;

	for (int size=1; size<N; size++)
	{
		int min_cost_inc;
		int global_cost_inc = 0-0x7FFFFFFF;
		int bs=-1, bm=-1, be=-1;
		
		for (int m = 0 ; m < N; m++)	// candidate node to insert
		{
			int ls, le;
			int s = 0;
			
			if (visited[m]) continue;

			min_cost_inc = 0x7FFFFFFF;

			for (int i =0; i<size; i++)
			{
				int e = next[s];
				
				// check
				int cost_inc = dist[s][m] + dist[m][e] - dist[s][e];
				if(cost_inc < min_cost_inc)
				{
					min_cost_inc = cost_inc;
					ls = s;
					le = e;
				}
				
				// move
				s = next[s];
			}

			if (min_cost_inc > global_cost_inc)
			{
				global_cost_inc = min_cost_inc;
				bs = ls;
				bm = m;
				be = le;
			}
		}

		// insert node
		next[bs] = bm;
		next[bm] = be;
		visited[bm] = true;
	}

	unsigned int total_len = 0;
	int ss = 0;

	for (int i = 0; i < N; i++)
	{
		total_len += dist[ss][next[ss]];
		
		#if PRINT_RESULT_TRAJ==true
		printf("%d->%d, %d\n", ss, next[ss], total_len);
		fflush(stdout);
		#endif

		ss = next[ss];
	}
	#if PRINT_RESULT_TRAJ==true
	printf("%n");
	#endif

	return total_len;
}


int main()
{
	clock_t start, end;
	unsigned long total_dist=0;

	char dir[1024] = DATA_DIR;
	char fn_prefix[1024] = FILENAME_PREFIX;	
	char fn_postfix[1024] = ".atsp";

	printf("============================================\n");
	printf("dir: %s\n", dir);
	printf("filename_prefix: %s\n", fn_prefix);

	printf("start loading data\n");
	for ( int i = 0 ; i < NUM_DATA; i ++) 
	{
		char filename[2048];
		sprintf(filename, "%s%s%d%s", dir, fn_prefix, i, fn_postfix);
		printf("%cloading: %s", 13, filename);
		load_data_file(filename, i);
	}
	printf("\ncomplete loading data\n");

	start = clock();
	
#if MODE==FARTHEST_INSERTION
	for ( int i = 0 ; i < NUM_DATA; i ++) total_dist += (unsigned long)atsp_farthest_insertion(i);
#else
	for ( int i = 0 ; i < NUM_DATA; i ++) total_dist += (unsigned long)atsp_nearest_insertion(i);
#endif

	end = clock();
		
	printf("total_dist = %.2lf, time=%d ms\n", (double)total_dist/(double)NUM_DATA, (end-start)/(CLOCKS_PER_SEC/1000));
	printf("============================================\n");
}

