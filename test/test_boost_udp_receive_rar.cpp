
#include "../boost_udp_receive_rar.h"
#include "boost_udp_send_faf.h"

#include <iostream>
#include <thread>

static void fail(const std::string& message, const std::string& received, const std::string& expected) {
	std::cout << "FAIL: " << message << ", received: " << received << ", expected: " << expected << std::endl;
	exit(1);
}

static void pass(const std::string& message, const std::string& received, const std::string& expected) {
	std::cout << "PASS: " << message << ", received: " << received << ", expected: " << expected << std::endl;
}

static void test_equals(const std::string& message, const std::string& received, const std::string& expected) {
	if (received != expected) {

		fail(message, received, expected);

	}
	else {

		pass(message, received, expected);

	}
}

static void test_equals(const std::string& message, const std::vector<unsigned char>& received, const std::vector<unsigned char>& expected) {
	if (received != expected) {

		std::cout << "FAIL: " << message << ", vectors don't match" << std::endl;
		exit(1);

	}
	else {

		std::cout << "PASS: " << message << ", vectors match" << std::endl;

	}
}

void test_boost_udp_receive_rar() {
	// Setup a receiver, specifying IP address and port
	// Note the IP address is that of the receiving network
	// interface.  As always with UDP and non-standard ports
	// check that a firewall isn't blocking data transfer.
	boost_udp_receive_rar rar("127.0.0.1", 8861);

	// Send a test message
	std::string m1("message1");
	boost_udp_send_faf("127.0.0.1", 8861).send(m1);

	// Receive a datagram synchronosuly as a string
	//
	std::string datagram = rar.receive_sync();

	test_equals("sync string", datagram, m1);

	// Asyncrounsly receive a datagram as a string
	//
	// We loop calling receive_async() until it returns
	// a non-empty string. receive_async() returns quickly
	// and we can do other work (or other async reads on different
	// ports) while we wait for somethign to arrive.

	int i = 0;

	std::string m2("message2");

	do {
		datagram = rar.receive_async();

		// Do other stuff
		//
		// Better do a little sleep to be nice to the
		// CPU
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// wait for a bit and then send a message
		if (++i == 10) {
			boost_udp_send_faf("127.0.0.1", 8861).send(m2);
		}

	} while (datagram.empty());

	test_equals("async string", datagram, m2);

	// Receive some binary data synchronously
	//
	std::string m3("message3");
	boost_udp_send_faf("127.0.0.1", 8861).send(m3);

	std::vector<unsigned char> data = rar.receive_binary_sync();

	test_equals("async binary", data, { m3.begin(), m3.end() });

	i = 0;

	std::string m4("message4");

	// Append some 'binary' data..
	m4 += '\0';
	m4 += 1;
	m4 += 128;
	m4 += 255;

	do {
		data = rar.receive_binary_async();

		// Do other stuff
		//
		// Better do a little sleep to be nice to the
		// CPU
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// wait for a bit and then send a message
		if (++i == 10) {
			boost_udp_send_faf("127.0.0.1", 8861).send(m4);
		}

	} while (data.empty());

	test_equals("async binary", data, { m4.begin(), m4.end() });
}

int main() {
	test_boost_udp_receive_rar();
	return 0;
}
