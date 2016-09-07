add_jar(java_prep_stmt SOURCES PrepStmtTest.java ../maxscale_java.jar
  ENTRY_POINT maxscale.java.test1.SimpleConnectorJTest INCLUDE_JARS maxscale_java ../mariadb-java-client-1.3.3.jar)
add_test(NAME java_prep_stmt COMMAND java
  -cp ${CMAKE_CURRENT_BINARY_DIR}/../maxscale_java.jar:${CMAKE_CURRENT_BINARY_DIR}/java_prep_stmt.jar:${CMAKE_CURRENT_SOURCE_DIR}/../mariadb-java-client-1.3.3.jar 
  maxscale.java.prep_stmt.PrepStmtTest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
file(APPEND ${CMAKE_BINARY_DIR}/templates "java_prep_stmt java_prep_stmt\n")
add_template(java_prep_stmt java_prep_stmt)
