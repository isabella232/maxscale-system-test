#ifndef BLOB_TEST_H
#define BLOB_TEST_H

#include "testconnections.h"

/**
 * @brief test_longblob INSERT big amount of data into lobg_blob_table
 * @param Test TestConnection object
 * @param conn MYSQL connection handler
 * @param blob_name blob type (LONGBLOB; MEDIUMBLOB or BLOB)
 * @param chunk_size size of one data chunk (in sizeof(long usingned))
 * @param chunks number of chunks to INSERT
 * @param rows number of rows to INSERT (executes INSERT stetament 'rows' times)
 * @return 0 in case of success
 */
int test_longblob(TestConnections* Test, MYSQL * conn, char * blob_name, unsigned long chunk_size, int chunks,
                  int rows);

#endif // BLOB_TEST_H
