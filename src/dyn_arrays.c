/*
 * dyn_arrays.c
 *
 *  Created on: 14 giu 2017
 *      Author: l3uri
 */

#include "dyn_arrays.h"

int compareIDs(const void* a, const void* b)
{
  return ((ID *)a)->x < ((ID *)b)->x;
}

void intInitArray(int_array_t *a, size_t initialSize) {
  a->array = (int *)malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
}

void intInsertArray(int_array_t *a, int element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (int *)realloc(a->array, a->size * sizeof(int));
  }
  a->array[a->used++] = element;
}

void intfreeArray(int_array_t *a) {
	if(a->array != NULL)
	{
		free(a->array);
		a->array = NULL;
		a->used = a->size = 0;
	}
}

void intresizeArray(int_array_t *a, size_t newsize)
{
	if(newsize >= a->size)
	{}
	else
	{
		int * tmp = (int *)malloc(newsize * sizeof(int));
		int i;
		for (i=0;i<newsize;i++)
		{
			tmp[i] = a->array[i];
		}
		free(a->array);
		a->array = tmp;
		a->size = newsize;
		a->used = newsize;
	}
}



void IDinitArray(ID_array_t *a, size_t initialSize) {
  a->array = (ID *)malloc(initialSize * sizeof(ID));
  a->used = 0;
  a->size = initialSize;
}

void IDinsertArray(ID_array_t *a, ID element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (ID *)realloc(a->array, a->size * sizeof(ID));
  }
  a->array[a->used++] = element;
}

void IDfreeArray(ID_array_t *a) {
	if(a->array != NULL)
	{
		free(a->array);
		a->array = NULL;
		a->used = a->size = 0;
	}
}

void IDresizeArray(ID_array_t *a, size_t newsize)
{
	if(newsize >= a->size)
	{}
	else
	{
		ID * tmp = (ID *)malloc(newsize * sizeof(ID));
		int i;
		for (i=0;i<newsize;i++)
		{
			tmp[i] = a->array[i];
		}
		free(a->array);
		a->array = tmp;
		a->size = newsize;
		a->used = newsize;
	}
}



void floatInitArray(float_array_t *a, size_t initialSize) {
  a->array = (float *)malloc(initialSize * sizeof(float));
  a->used = 0;
  a->size = initialSize;
}

void floatInsertArray(float_array_t *a, float element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (float *)realloc(a->array, a->size * sizeof(float));
  }
  a->array[a->used++] = element;
}

void floatfreeArray(float_array_t *a) {
  if(a->array != NULL)
  {
	  free(a->array);
	  a->array = NULL;
	  a->used = a->size = 0;
  }
}

void floatresizeArray(float_array_t *a, size_t newsize)
{
	if(newsize >= a->size)
	{}
	else
	{
		float * tmp = (float *)malloc(newsize * sizeof(float));
		int i;
		for (i=0;i<newsize;i++)
		{
			tmp[i] = a->array[i];
		}
		free(a->array);
		a->array = tmp;
		a->size = newsize;
		a->used = newsize;
	}
}



void sparsevectorinitArray(lasvm_sparsevector_array_t *a, size_t initialSize) {
  a->array = (lasvm_sparsevector_t* *)malloc(initialSize * sizeof(lasvm_sparsevector_t*));
  a->used = 0;
  a->size = initialSize;
}

void sparsevectorinsertArray(lasvm_sparsevector_array_t *a, lasvm_sparsevector_t* element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (lasvm_sparsevector_t* *)realloc(a->array, a->size * sizeof(lasvm_sparsevector_t*));
  }
  a->array[a->used++] = element;
}

void sparsevectorfreeArray(lasvm_sparsevector_array_t *a) {
	if(a->array != NULL)
	{
		free(a->array);
		a->array = NULL;
		a->used = a->size = 0;
	}
}

void sparsevectorResizeArray(lasvm_sparsevector_array_t *a, size_t newsize)
{
	if(newsize >= a->size)
	{}
	else
	{
		lasvm_sparsevector_t* * tmp = (lasvm_sparsevector_t* *)malloc(newsize * sizeof(lasvm_sparsevector_t*));
		int i;
		for (i=0;i<newsize;i++)
		{
			tmp[i] = a->array[i];
		}
		free(a->array);
		a->array = tmp;
		a->size = newsize;
		a->used = newsize;
	}
}
