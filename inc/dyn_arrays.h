/*
 * dyn_arrays.h
 *
 *  Created on: 14 giu 2017
 *      Author: l3uri
 */

#ifndef DYN_ARRAYS_H_
#define DYN_ARRAYS_H_

#include "vector.h"

typedef struct ID_s ID;
int compareIDs(const void* a, const void* b);
struct ID_s {
  int x;
  int y;
};

typedef struct ID_array ID_array_t;
void IDinsertArray(ID_array_t *a, ID element);
void IDresizeArray(ID_array_t *a, size_t newsize);
void IDinitArray(ID_array_t *a, size_t initialSize);
void IDfreeArray(ID_array_t *a);
struct ID_array
{
        ID *array;
        size_t used;
        size_t size;
};

typedef struct int_array int_array_t;
void intInsertArray(int_array_t *a, int element);
void intInitArray(int_array_t *a, size_t initialSize);
void intresizeArray(int_array_t *a, size_t newsize);
void intfreeArray(int_array_t *a);
struct int_array
{
        int *array;
        size_t used;
        size_t size;
};


typedef struct float_array float_array_t;
void floatInsertArray(float_array_t *a, float element);
void floatresizeArray(float_array_t *a, size_t newsize);
void floatInitArray(float_array_t *a, size_t initialSize);
void floatfreeArray(float_array_t *a);
struct float_array
{
		float *array;
        size_t used;
        size_t size;
};



typedef struct lasvm_sparsevector_array lasvm_sparsevector_array_t;
void sparsevectorinsertArray(lasvm_sparsevector_array_t *a, lasvm_sparsevector_t* element);
void sparsevectorinitArray(lasvm_sparsevector_array_t *a, size_t initialSize);
void sparsevectorfreeArray(lasvm_sparsevector_array_t *a);
void sparsevectorResizeArray(lasvm_sparsevector_array_t *a, size_t newsize);
struct lasvm_sparsevector_array
{
		lasvm_sparsevector_t* *array;
        size_t used;
        size_t size;
};

#endif /* DYN_ARRAYS_H_ */
