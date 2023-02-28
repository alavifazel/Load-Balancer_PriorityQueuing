#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <cadmium/lib/iestream.hpp>
#include <limits>
#include <fstream>
#include "transducer.hpp"
#include "job_pair.hpp"
#include "job.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer::iestream {
	struct iestream_coupled : public Coupled {
		iestream_coupled(const std::string& id, const char* filePathGen, const char* filePathProc): Coupled(id) {
			auto iestreamGen = addComponent<lib::IEStream<JobPair>>("iestreamGen", filePathGen);
        	auto iestreamProc = addComponent<lib::IEStream<Job>>("iestreamProc", filePathProc);
        	auto iestream = addComponent<lib::IEStream<Job>>("iestream", filePathProc);            
            auto transducer = addComponent<Transducer>("transducer", 10, 2);
            addCoupling(iestreamGen->out, transducer->inGenerated);
            addCoupling(iestreamProc->out, transducer->inProcessed[0]);
		}
	};
}

int main(int argc, char *argv[]) {
	std::ifstream file, file2;

	if (argc < 3) { // Check that file is included
        std::cerr << "ERROR: not enough arguments" << std::endl;
        std::cerr << "    Usage:" << std::endl;
        std::cerr << "    > main_iestream INPUTFILE_GEN INPUTFILE_PROC" << std::endl;
        return -1;
    }

	const char* filePathGen = argv[1];
    const char* filePathProc = argv[2];


	file.open(filePathGen);
    file2.open(filePathProc);

    if(!file.is_open() || !file2.is_open()) { // Check file can be opened
        std::cerr << "ERROR: file can not be opened. Check file path." << std::endl;
        return -1;
    }

	auto model = std::make_shared<iestream::iestream_coupled>(
        "IEStreamCoupled", filePathGen, filePathProc);

	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("test_transducer.csv", ";");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}
