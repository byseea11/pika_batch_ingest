# CMake generated Testfile for 
# Source directory: /home/byseea/code/opensum/pika_batch_ingest
# Build directory: /home/byseea/code/opensum/pika_batch_ingest/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_bingest "/home/byseea/code/opensum/pika_batch_ingest/build/test_bingest")
set_tests_properties(test_bingest PROPERTIES  _BACKTRACE_TRIPLES "/home/byseea/code/opensum/pika_batch_ingest/CMakeLists.txt;55;add_test;/home/byseea/code/opensum/pika_batch_ingest/CMakeLists.txt;0;")
subdirs("third/googletest")
subdirs("_deps/nlohmann_json-build")
