#ifndef TRANSDUCER_MODEL
#define TRANSDUCER_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <array>
#include <iostream>
#include <utility>
#include <map>
#include "job.hpp"
#include "job_pair.hpp"

namespace cadmium::loadbalancer {
	struct TransducerState {
		double clock;
		double sigma;
		double processingTime;
		int nJobsGenerated;
		int nJobsProcessed;
		bool waitingForServersProcessing;
		std::map<int, std::pair<double,double>> jobGenProc;

		explicit TransducerState(double obsTime) : clock(), 
												   sigma(obsTime), 
												   processingTime(), 
												   nJobsGenerated(), 
												   nJobsProcessed(),
												   waitingForServersProcessing(false) {}
	};

	std::ostream& operator<<(std::ostream &out, const TransducerState& s) {
		out << "{" << s.processingTime << "," << s.nJobsGenerated << "," << s.nJobsProcessed << "}";
		return out;
	}

	class Transducer : public Atomic<TransducerState> {
	 public:
		Port<JobPair> inGenerated;
		std::array<Port<Job>, 3> inProcessed;
		Port<bool> outStop;
		Port<double> averageProcessingTime;
		Port<double> throughput;
		double serverProcessingExpMean;

		Transducer(const std::string& id, double obsTime, double serverProcessingExpMean): 
							Atomic<TransducerState>(id, TransducerState(obsTime)), serverProcessingExpMean(serverProcessingExpMean) {
			inGenerated = addInPort<JobPair>("inGenerated");
			inProcessed[0] = addInPort<Job>("inProcessed1");
			inProcessed[1] = addInPort<Job>("inProcessed2");
			inProcessed[2] = addInPort<Job>("inProcessed3");
			outStop = addOutPort<bool>("outStop");
			averageProcessingTime = addOutPort<double>("averageProcessingTime");
			throughput = addOutPort<double>("throughput");
		}

		void internalTransition(TransducerState& s) const override {
			switch(s.waitingForServersProcessing) {
				case false:
					s.waitingForServersProcessing = true;
					s.sigma = serverProcessingExpMean * s.nJobsGenerated;
					break;
				default:
					s.sigma = std::numeric_limits<double>::infinity();
					break;
			}
		}

		void externalTransition(TransducerState& s, double e) const override {
			s.sigma -= e;
			s.clock += e;
			for (auto& job: inGenerated->getBag()) {
				s.nJobsGenerated += 1;
				std::cout << "Job " << job.job.id << " generated at t = " << s.clock << std::endl;
				s.jobGenProc[job.job.id] = std::make_pair(s.clock, -1);
			}
			for (auto& job: inProcessed[0]->getBag()) {
				s.nJobsProcessed += 1;
				s.processingTime += job.timeProcessed - job.timeGenerated;
				std::cout << "1) Job " << job.id << " processed at t = " << s.clock << std::endl;
				s.jobGenProc[job.id].second = s.clock;
			}
			for (auto& job: inProcessed[1]->getBag()) {
				s.nJobsProcessed += 1;
				s.processingTime += job.timeProcessed - job.timeGenerated;
				std::cout << "2) Job " << job.id << " processed at t = " << s.clock << std::endl;
				s.jobGenProc[job.id].second = s.clock;

			}
			for (auto& job: inProcessed[2]->getBag()) {
				s.nJobsProcessed += 1;
				s.processingTime += job.timeProcessed - job.timeGenerated;
				std::cout << "3) Job " << job.id << " processed at t = " << s.clock << std::endl;
				s.jobGenProc[job.id].second = s.clock;
			}
		}

		void output(const TransducerState& s) const override {
			switch(s.waitingForServersProcessing) {
				case true:
					std::cout << "End time: " << s.clock << std::endl;
					std::cout << "Jobs generated: " << s.nJobsGenerated << std::endl;
					std::cout << "Jobs processed: " << s.nJobsProcessed << std::endl;
					if (s.nJobsProcessed > 0) {
						int n = 0;
						double sum = 0;
						for(const auto& j: s.jobGenProc) {
							sum += j.second.second - j.second.first;
							n++;
						}

						std::cout << "Average Residence Time: " << (sum / n) << std::endl;
						averageProcessingTime->addMessage(sum / n);
					}
					if (s.clock > 0) {
						std::cout << "Throughput: " << (double) s.nJobsProcessed /  s.clock << std::endl;
						throughput->addMessage((double) s.nJobsProcessed / s.clock);
					}
					break;
				default:
					outStop->addMessage(true);
					break;

			}
			
		}

		[[nodiscard]] double timeAdvance(const TransducerState& s) const override {
			return s.sigma;
		}
	};
}

#endif 
