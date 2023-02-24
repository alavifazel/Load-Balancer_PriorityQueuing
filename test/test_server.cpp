#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <cadmium/lib/iestream.hpp>
#include <limits>
#include <fstream>
#include "server.hpp"
#include "job_pair.hpp"
#include "job.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer::iestream {
	struct iestream_coupled : public Coupled {
		iestream_coupled(const std::string& id, const char* filePath): Coupled(id) {
			auto iestream1 = addComponent<lib::IEStream<Job>>("iestream", filePath);
            auto server = addComponent<Server>("server",10);
            addCoupling(iestream1->out, server->inGenerated);
		}
	};
}

int main(int argc, char *argv[]) {
	std::ifstream file;

	if (argc < 2) { // Check that file is included
        std::cerr << "ERROR: not enough arguments" << std::endl;
        std::cerr << "    Usage:" << std::endl;
        std::cerr << "    > main_iestream INPUTFILE" << std::endl;
        return -1;
    }

	const char* filePath = argv[1];
    
	file.open(filePath);
    if(!file.is_open()) { // Check file can be opened
        std::cerr << "ERROR: file can not be opened. Check file path." << std::endl;
        return -1;
    }

	auto model = std::make_shared<iestream::iestream_coupled>("IEStreamCoupled", filePath);
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("test_server.csv", ";");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}