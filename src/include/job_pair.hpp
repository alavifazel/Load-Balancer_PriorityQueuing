#ifndef JOB_PAIR
#define JOB_PAIR

#include <iostream>
#include "job.hpp"

namespace cadmium::loadbalancer {
	struct JobPair {
        int priority;
        Job job;
		JobPair(int priority, Job job): priority(priority), job(job){}
		JobPair(){}
	};

	std::ostream& operator<<(std::ostream& out, const JobPair& j) {
		out << "{" << j.job.id << "," << j.job.timeGenerated << "," << j.job.timeProcessed << ", Priority: " << j.priority << "}";
		return out;
	}
	std::istream& operator>> (std::istream& is, JobPair& jobPair) {
        is >> jobPair.priority >> jobPair.job.id >> jobPair.job.timeGenerated >> jobPair.job.timeProcessed;
        return is;
    }
}

#endif 
