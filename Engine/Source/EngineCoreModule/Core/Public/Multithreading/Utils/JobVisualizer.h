#pragma once

#include "Math/Color.h"
#include "Misc/Paths.h"
#include "Multithreading/JobScheduler.h"

namespace LE
{
class JobVisualizer
{
	struct JobNodeDescriptor;

	struct UpdatePassDescriptor
	{
		std::vector<JobNodeDescriptor*> Jobs;
		std::string_view UpdatePassName;
		Color Color;
	};

	struct JobNodeDescriptor
	{
		std::vector<JobNodeDescriptor*> DependentJobs;
		std::string_view JobName;
		UpdatePassDescriptor* ParentUpdatePass;
	};

public:
	JobVisualizer(const std::vector<RefCountingPtr<JobNode>>& Jobs);
	~JobVisualizer();

	JobVisualizer(const JobVisualizer&) = delete;
	JobVisualizer(JobVisualizer&&) = delete;
	JobVisualizer& operator=(const JobVisualizer&) = delete;
	JobVisualizer& operator=(JobVisualizer&&) = delete;

	void Dump(Path SavePath);

private:
	void WriteUpdatePass(std::ostream& Stream, const UpdatePassDescriptor& Descriptor);
	void WriteJob(std::ostream& Stream, const JobNodeDescriptor& Descriptor);

	JobNodeDescriptor* GetCreateJobDescriptor(UpdateJobType JobType);
	UpdatePassDescriptor* GetCreateUpdatePassDescriptor(UpdatePassType PassType);

	std::unordered_map<UpdateJobType, JobNodeDescriptor*> JobDescriptors;
	std::unordered_map<UpdatePassType, UpdatePassDescriptor*> UpdatePasses;
	std::vector<JobNodeDescriptor*> StartingJobs;
};
}
