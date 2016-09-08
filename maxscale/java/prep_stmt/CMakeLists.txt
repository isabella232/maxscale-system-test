add_jar(java_prep_stmt SOURCES PrepStmtTest.java ${MXS_JAR}
  ENTRY_POINT maxscale.java.test1.SimpleConnectorJTest INCLUDE_JARS ${MXS_JAR} ${JDBC_JAR})
add_test(NAME java_prep_stmt COMMAND java
  -cp ${TEST_JARPATH}:${CMAKE_CURRENT_BINARY_DIR}/java_prep_stmt.jar
  maxscale.java.prep_stmt.PrepStmtTest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_template(java_prep_stmt java_prep_stmt)
