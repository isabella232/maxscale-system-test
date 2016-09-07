add_jar(simplejavatest SOURCES SimpleConnectorJTest.java ${MXS_JAR}
  ENTRY_POINT maxscale.java.test1.SimpleConnectorJTest INCLUDE_JARS ${MXS_JAR} ${JDBC_JAR})
add_test(NAME simplejavatest COMMAND java
  -cp ${TEST_JARPATH}:${CMAKE_CURRENT_BINARY_DIR}/simplejavatest.jar
  maxscale.java.test1.SimpleConnectorJTest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_template(simplejavatest simplejavatest)
