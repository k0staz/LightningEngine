#include "Misc/Paths.h"

namespace LE
{
Path GetEngineRoot()
{
	static Path thisFile = __FILE__;
	while (thisFile.filename() != "Engine")
	{
		thisFile = thisFile.parent_path();
	}

	return thisFile;
}
}
