/*
 * render-js-client.cpp
 *
 *  Created on: Sep 16, 2016
 *      Author: dan
 */

#include "RJSAsyncTcpClient.h"
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio.hpp>
#include <string>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char **argv)
{
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "produce help message")
	    ("server,s", po::value<string>(), "specify server ip:port")
		("file,f", po::value<string>(), "file to pass to server")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
	    cout << desc << "\n";
	    return 1;
	}

	if (!vm.count("server"))
	{
		cerr << "no server specified on command line" << endl;
		return -1;
	}

	if (!vm.count("file"))
	{
		cerr << "no file specified on command line" << endl;
		return -1;
	}

	boost::asio::io_service ioservice;
	render::RJSAsyncTcpClient tcpClient(ioservice, vm["server"].as<string>(), boost::filesystem::path(vm["file"].as<string>()));
	ioservice.run();

}

