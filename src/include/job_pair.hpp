#ifndef JOB_PAIR
#define JOB_PAIR

#include <iostream>
#include "job.hpp"

namespace cadmium::loadbalancer {
	struct JobPair {
        int priority;
        Job job;

		JobPair(int priority, Job job): priority(priority), job(job){}

	};

	std::ostream& operator<<(std::ostream& out, const JobPair& j) {
		out << "{" << j.job.id << "," << j.job.timeGenerated << "," << j.job.timeProcessed << ", P: " << j.priority << "}";
		return out;
	}
}

#endif 