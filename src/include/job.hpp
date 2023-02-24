#ifndef JOB
#define JOB

#include <iostream>

namespace cadmium::loadbalancer {
	struct Job {
		int id;
		double timeGenerated;
		double timeProcessed;

		Job(int id, double timeGenerated, double timeProcessed):
			id(id), timeGenerated(timeGenerated), timeProcessed(timeProcessed) {}
		Job(int id, double timeGenerated): Job(id, timeGenerated, -1) {}
		Job(int id): Job(id, -1, -1) {};
		Job(){}
	};

	std::ostream& operator<<(std::ostream& out, const Job& j) {
		out << "{" << j.id << "," << j.timeGenerated << "," << j.timeProcessed << "}";
		return out;
	}
	std::istream& operator>> (std::istream& is, Job& job) {
        is >> job.id >> job.timeGenerated >> job.timeProcessed;
        return is;
    }
}

#endif 