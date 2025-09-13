#include "Multithreading/Utils/JobVisualizer.h"

#include <fstream>

namespace LE
{
JobVisualizer::JobVisualizer(const std::vector<RefCountingPtr<JobNode>>& Jobs)
{
	for (const auto& job : Jobs)
	{
		UpdateJobType type = job->GetType();
		JobNodeDescriptor& descriptor = *GetCreateJobDescriptor(type);
		descriptor.JobName = job->GetName();
		descriptor.ParentUpdatePass = GetCreateUpdatePassDescriptor(job->GetUpdatePassType());
		descriptor.ParentUpdatePass->Jobs.push_back(&descriptor);

		for (const auto& dependentJob : job->GetDependentJobs())
		{
			descriptor.DependentJobs.push_back(GetCreateJobDescriptor(dependentJob->GetType()));
		}

		if (job->GetDefaultRemainingJobCount() == 0)
		{
			StartingJobs.push_back(&descriptor);
		}
	}
}

JobVisualizer::~JobVisualizer()
{
	for (auto& job : JobDescriptors)
	{
		delete job.second;
	}
	JobDescriptors.clear();

	for (auto& pass : UpdatePasses)
	{
		delete pass.second;
	}
	UpdatePasses.clear();
}

void JobVisualizer::Dump(Path SavePath)
{
	std::filesystem::create_directories(SavePath.parent_path());
	std::ofstream os(SavePath);
	if (!os)
	{
		return;
	}

	os << "digraph UpdateGraph {\n";
	os << "graph [rankdir=TB, compound=true, fontname=\"Inter\", fontsize=11, bgcolor=white];\n";
	os << "node  [shape=box, style=\"rounded, filled\", fontname=\"Inter\", fontsize=10];\n";
	os << "edge  [arrowsize=0.8, color=black];\n";
	os << "\n";

	for (const auto& updatePass : UpdatePasses)
	{
		WriteUpdatePass(os, *updatePass.second);
		os << "\n";
	}

	std::vector<JobNodeDescriptor*> currentJobs = StartingJobs;
	std::vector<JobNodeDescriptor*> nextJobs;
	while (!currentJobs.empty())
	{
		for (JobNodeDescriptor* job : currentJobs)
		{
			WriteJob(os, *job);

			for (JobNodeDescriptor* dependentJob : job->DependentJobs)
			{
				nextJobs.push_back(dependentJob);
			}
		}

		currentJobs = nextJobs;
		nextJobs.clear();
	}
	

	os << "}\n";
}

void JobVisualizer::WriteUpdatePass(std::ostream& Stream, const UpdatePassDescriptor& Descriptor)
{
	Stream << "subgraph cluster_" << Descriptor.UpdatePassName << " {\n";
	Stream << "label=\"" << Descriptor.UpdatePassName << "\";\n";
	Stream << "color=\"" << ToCssHex(Descriptor.Color) << "\";\n";
	Stream << "style=rounded;\n";
	Stream << "fontcolor=black;\n";
	Stream << "bgcolor=white;\n";

	for (const auto& job : Descriptor.Jobs)
	{
		Stream << "Job_" << job->JobName << "[label=\"" << job->JobName << "\"];\n";
	}

	Stream << "}\n";
}

void JobVisualizer::WriteJob(std::ostream& Stream, const JobNodeDescriptor& Descriptor)
{
	if (Descriptor.DependentJobs.empty())
	{
		return;
	}

	Stream << "\n";
	for (JobNodeDescriptor* job : Descriptor.DependentJobs)
	{
		Stream << "Job_" << Descriptor.JobName << " -> " << "Job_" << job->JobName << ";\n";
	}
	Stream << "\n";
}

JobVisualizer::JobNodeDescriptor* JobVisualizer::GetCreateJobDescriptor(UpdateJobType JobType)
{
	JobNodeDescriptor* descriptor = nullptr;
	if (JobDescriptors.contains(JobType))
	{
		descriptor = JobDescriptors[JobType];
	}
	else
	{
		descriptor = new JobNodeDescriptor;
		JobDescriptors[JobType] = descriptor;
	}

	return descriptor;
}

JobVisualizer::UpdatePassDescriptor* JobVisualizer::GetCreateUpdatePassDescriptor(UpdatePassType PassType)
{
	UpdatePassDescriptor* descriptor = nullptr;
	if (UpdatePasses.contains(PassType))
	{
		descriptor = UpdatePasses[PassType];
	}
	else
	{
		descriptor = new UpdatePassDescriptor;
		const UpdatePass* updatePass = UpdatePass::GetUpdatePass(PassType);
		descriptor->UpdatePassName = updatePass->GetName();
		descriptor->Color = updatePass->GetDebugColor();

		UpdatePasses[PassType] = descriptor;
	}

	return descriptor;
}
}
