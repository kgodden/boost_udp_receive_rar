all: test_boost_udp_receive_rar.cpp
	g++ -o  test_boost_udp_receive_rar -pthread  test_boost_udp_receive_rar.cpp -lboost_system
	
.PHONY: clean
clean:
	-rm -f test_boost_udp_receive_rar *.gch 2> /dev/null
