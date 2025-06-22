#include "D3D11Utils.h"

namespace LE::D3D11
{
void VerifyD3D11Result(HRESULT Result, const char* Code, const char* Filename, uint32 Line)
{
	LE_ASSERT_DESC(false, """%s"" failed at line %d in %s", Code, Line, Filename)
}
}
