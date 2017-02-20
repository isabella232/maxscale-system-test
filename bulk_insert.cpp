/**
 * MXS-1121: MariaDB 10.2 Bulk Insert test
 *
 * This test is a copy of one of the examples for bulk inserts:
 * https://mariadb.com/kb/en/mariadb/bulk-insert-column-wise-binding/
 */

#include "testconnections.h"

static int show_mysql_error(MYSQL *mysql)
{
    printf("Error(%d) [%s] \"%s\"\n", mysql_errno(mysql),
           mysql_sqlstate(mysql),
           mysql_error(mysql));
    return 1;
}

static int show_stmt_error(MYSQL_STMT *stmt)
{
    printf("Error(%d) [%s] \"%s\"\n", mysql_stmt_errno(stmt),
           mysql_stmt_sqlstate(stmt),
           mysql_stmt_error(stmt));
    return 1;
}

int bind_by_column(MYSQL *mysql)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];

    /* Data for insert */
    const char *surnames[] = {"Widenius", "Axmark", "N.N."};
    unsigned long surnames_length[] = {8, 6, 4};
    const char *forenames[] = {"Monty", "David", "will be replaced by default value"};
    char forename_ind[] = {STMT_INDICATOR_NTS, STMT_INDICATOR_NTS, STMT_INDICATOR_DEFAULT};
    char id_ind[] = {STMT_INDICATOR_NULL, STMT_INDICATOR_NULL, STMT_INDICATOR_NULL};
    unsigned int array_size = 3;

    if (mysql_query(mysql, "DROP TABLE IF EXISTS test.bulk_example1"))
    {
        return show_mysql_error(mysql);
    }

    if (mysql_query(mysql, "CREATE TABLE test.bulk_example1 (id INT NOT NULL AUTO_INCREMENT PRIMARY KEY," \
                    "forename CHAR(30) NOT NULL DEFAULT 'unknown', surname CHAR(30))"))
    {
        return show_mysql_error(mysql);
    }

    stmt = mysql_stmt_init(mysql);
    if (mysql_stmt_prepare(stmt, "INSERT INTO test.bulk_example1 VALUES (?,?,?)", -1))
    {
        return show_stmt_error(stmt);
    }

    memset(bind, 0, sizeof(MYSQL_BIND) * 3);

    /* We autogenerate id's, so all indicators are STMT_INDICATOR_NULL */
    bind[0].u.indicator = id_ind;
    bind[0].buffer_type = MYSQL_TYPE_LONG;

    bind[1].buffer = forenames;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].u.indicator = forename_ind;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = surnames;
    bind[2].length = surnames_length;

    /* set array size */
    mysql_stmt_attr_set(stmt, STMT_ATTR_ARRAY_SIZE, &array_size);

    /* bind parameter */
    mysql_stmt_bind_param(stmt, bind);

    /* execute */
    if (mysql_stmt_execute(stmt))
    {
        return show_stmt_error(stmt);
    }

    mysql_stmt_close(stmt);
    return 0;
}

int main(int argc, char** argv)
{
    TestConnections::require_repl_version("10.2");
    TestConnections test(argc, argv);
    test.connect_maxscale();
    bind_by_column(test.conn_rwsplit);
    test.close_maxscale_connections();
    return test.global_result;
}
