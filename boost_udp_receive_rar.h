#pragma once

//   Copyright 2022 Kevin Godden
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include <boost/asio.hpp>
#include <string>
#include <vector>

//
// 
//
// A really simple option-free wrapper around the boost::asio stuff for
// synchronous and asynchronous reception of UDP datagrams. Ease of
// use is prioritised over flexible options - if flexibility is required, 
// then just use the boost::asio functionality directly!
//
// The socket is opened & bound, and a buffer (~65KB) is allocated
// as soon as an object of this class is created. To keep things simple
// some data copies are performed.
//
// This class is not thread safe and should only be used from a single thread
//
// Receive and Rejoice! (RAR)
//
// Synopsis:
//
/*
	// Setup a receiver, specifying IP address and port
	// Note the IP address is that of the receiving network
	// interface.  As always with UDP and non-standard ports
	// check that a firewall isn't blocking data transfer.
	boost_udp_receive_rar rar("127.0.0.1", 8861);

	// Receive a datagram synchronously as a string
	//
	std::string datagram = rar.receive_sync();

	cout << "Sync received string: " << datagram << endl;

	// Asynchronously receive a datagram as a string
	//
	// We loop calling receive_async() until it returns
	// a non-empty string. receive_async() returns quickly
	// and we can do other work (or other async reads on different
	// ports) while we wait for something to arrive.
	do {
		datagram = rar.receive_async();

		// Do other stuff
		//
		// Better do a little sleep to be nice to the
		// CPU
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	} while (datagram.empty());

	cout << "Async, received string: " << datagram << endl;

	// Receive some binary data synchronously
	//
	std::vector<unsigned char> data = rar.receive_binary_sync();

	cout << "Sync, received a datagram of size " << data.size() << endl;

	do {
		data = rar.receive_binary_async();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (data.empty());

	cout << "Sync, received a datagram of size " << data.size() << endl;
*/

class boost_udp_receive_rar {
	// Some boost::asio necessaries!
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint endpoint;

	// Holds the received data for async receives
	std::vector<unsigned char> buffer;

	// Is there an async receive in progress?
	bool in_receive = false;

	// The number of bytes received
	// by async receive.
	size_t bytesRead = 0;

public:
	// Construct with IP address an port, note that the IP address is the 
	// address of the network interface on the _receiving_ computer on which you
	// want to receive UDP data.
	boost_udp_receive_rar(const std::string& ip_address, const int port) : socket(io_service) {

		// Open socket & make/bind endpoint
		socket.open(boost::asio::ip::udp::v4());
		endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ip_address), port);
		socket.bind(endpoint);

		// Figure out how big the datagrams can be and
		// create a buffer big enough to hold this amount
		// of received bytes.
		boost::asio::socket_base::receive_buffer_size option;
		socket.get_option(option);
		const int size = option.value();

		// Make sure vector has enough space.
		buffer.resize(size);
	}

	//
	// Receive a binary UDP Datagram asynchronously
	// If no datagram has been received, then
	// an empty vector is returned.
	//
	std::vector<unsigned char> receive_binary_async() {

		// Is there already an async receive
		// in progress?
		if (in_receive) {

			// Yes, let's see if any data has arrived..
			if (bytesRead && bytesRead > 0) {
				// Looks like we have some data, lets grab and
				// return it in a vector<>
				std::vector<unsigned char> out;
				out.resize(bytesRead);

				// Copy N bytes to the output vector.
				std::copy_n(buffer.begin(), bytesRead, out.begin());

				// Mark async receive as complete
				// so that the caller can setup a new
				// read they want to.
				in_receive = false;

				// Hopefully the compiler will perform 
				// Return Value Optimisation on this vector!
				return out;
			}
			else {
				// No data received yet, let's get
				// boost::asio to check again.
				io_service.poll_one();
			}

		}
		else {
			// There is no async receive already in progress
			// so let's start one now.

			// Clear flags
			bytesRead = 0;

			// Mark async receive in progress.
			in_receive = true;

			// Setup a boost::asio async receive providing a receive 
			// complete handler as a lambda
			socket.async_receive(boost::asio::buffer(buffer), 0,
				[&](boost::system::error_code ec, std::size_t N)
			{
				// This lambda will be called when the receive
				// completes, we just set the number of bytes
				// received, and the next time receive_async()
				// is called the data will be returned to the
				// caller.
				bytesRead = N;
			}

			);
		}

		// Nothing received yet!
		return{};
	}

	//
	// Receive a UDP Datagram asynchronously as a string.
	// If no datagram has been received, then
	// an empty string is returned.
	//
	std::string receive_async() {
		// Get the datagram as binary
		const auto datagram = receive_binary_async();

		// If we got back an empty vector, then
		// nothing was received, return empty string.
		if (datagram.empty())
			return{};

		// We got a datagram, Copy to a string.
		return std::string(datagram.begin(), datagram.end());
	}

	//
	// Receive a binary UDP Datagram synchronously
	// This function will block until a datagram
	// is received.
	//
	std::vector<unsigned char> receive_binary_sync() {

		// Sync receive of a UDP datagram as a vector
		const size_t bytesRead = socket.receive(boost::asio::buffer(buffer));

		// Copy just the received data from our
		// internal buffer to an output vector
		std::vector<unsigned char> out;
		out.resize(bytesRead);

		// Copy N bytes to the output vector.
		std::copy_n(buffer.begin(), bytesRead, out.begin());

		// Hope for RVO!
		return out;
	}

	//
	// Receive a UDP Datagram synchronously as a string
	// This function will block until a datagram
	// is received.
	//
	std::string receive_sync() {

		// Sync receive of a UDP datagram as a string
		const auto datagram = receive_binary_sync();

		// Copy to a string.
		return std::string(datagram.begin(), datagram.end());
	}
};