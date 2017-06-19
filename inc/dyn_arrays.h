/*
 * dyn_arrays.h
 *
 *  Created on: 14 giu 2017
 *      Author: l3uri
 */

#ifndef DYN_ARRAYS_H_
#define DYN_ARRAYS_H_

#include "vector.h"

// ID STRUCT & FUNCTIONS --------------------

typedef struct ID_s ID;
struct ID_s {
  int x;
  int y;
};

typedef struct ID_array ID_array_t;
struct ID_array
{
        ID *array;
        size_t used;
        size_t size;
};

int compareIDs(const void* a, const void* b);
void IDInsertArray(ID_array_t *a, ID element);
void IDResizeArray(ID_array_t *a, size_t newsize);
void IDInitArray(ID_array_t *a, size_t initialSize);
void IDFreeArray(ID_array_t *a);
ID IDPopFromArray(ID_array_t *a);

// INT STRUCT & FUNCTIONS --------------------

typedef struct int_array int_array_t;
struct int_array
{
        int *array;
        size_t used;
        size_t size;
};

void intInsertArray(int_array_t *a, int element);
void intInitArray(int_array_t *a, size_t initialSize);
void intResizeArray(int_array_t *a, size_t newsize);
void intFreeArray(int_array_t *a);
int intPopFromArray(int_array_t *a);

// FLOAT STRUCT & FUNCTIONS --------------------

typedef struct float_array float_array_t;
struct float_array
{
		float *array;
        size_t used;
        size_t size;
};
void floatInsertArray(float_array_t *a, float element);
void floatResizeArray(float_array_t *a, size_t newsize);
void floatInitArray(float_array_t *a, size_t initialSize);
void floatFreeArray(float_array_t *a);
float floatPopFromArray(float_array_t *a);

// SPARSEVECTOR STRUCT & FUNCTIONS --------------------
typedef struct lasvm_sparsevector_array lasvm_sparsevector_array_t;
struct lasvm_sparsevector_array
{
		lasvm_sparsevector_t* *array;
        size_t used;
        size_t size;
};

void sparsevectorInsertArray(lasvm_sparsevector_array_t *a, lasvm_sparsevector_t* element);
void sparsevectorInitArray(lasvm_sparsevector_array_t *a, size_t initialSize);
void sparsevectorFreeArray(lasvm_sparsevector_array_t *a);
void sparsevectorResizeArray(lasvm_sparsevector_array_t *a, size_t newsize);
lasvm_sparsevector_t* sparsevectorPopFromArray(lasvm_sparsevector_array_t *a);


#endif /* DYN_ARRAYS_H_ */
