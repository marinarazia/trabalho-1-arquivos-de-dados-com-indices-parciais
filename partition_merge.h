#ifndef PARTITION_MERGE_H
#define PARTITION_MERGE_H

#include <stdio.h>
#include <stdlib.h>
#include "entities.h"

#ifndef MAX_RECORDS_IN_MEMORY
#define MAX_RECORDS_IN_MEMORY 10000
#endif

#ifndef TEMP_PREFIX
#define TEMP_PREFIX "temp"
#endif

int compareOrder(const void *a, const void *b);
int compareProduct(const void *a, const void *b);

int createSortedTemps(const char *inputFile,
                      const size_t recordSize,
                      const int (*comparator)(const void *, const void *));

int mergeTwoTemps(const char *file1,
                  const char *file2,
                  const char *outFile,
                  const size_t recordSize,
                  const int (*comparator)(const void *, const void *));

void mergeAllTemps(const int tempCount,
                   const size_t recordSize,
                   const int (*comparator)(const void *, const void *),
                   const char *finalOutput);

void removeDuplicateProducts(const char *sortedFile);

#endif
