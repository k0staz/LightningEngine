import os
import re
import sys
from pathlib import Path

source_dir = Path("../")
output_dir_h = Path("Generated/Public")
output_dir_cpp = Path("Generated/Private")

variant_pattern = re.compile(
    r"REGISTER_ASSET_TYPE\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\,\s*\"[A-Za-z_][A-Za-z0-9_]*\"\s*\)"
)

asset_types = []
header_files = []

# Find all Asset Types

def get_relative_to_public(path: Path) -> Path:
    for parent in path.parents:
        if parent.name == "Public":
            return path.relative_to(parent)

modules_list = []
if len(sys.argv) > 1:
    module_strings = sys.argv[1]
    modules_list = module_strings.split(',')

for module in modules_list:
    full_path = source_dir / module
    for header_file in source_dir.rglob("*.h"):
        try:
            with open(header_file, "r", encoding="utf-8") as f:
                contents = f.read()
                matches = variant_pattern.findall(contents)
                for asset_type in matches:
                    if(asset_type == "Type"):
                        continue

                    relative_header = get_relative_to_public(header_file)
                    if asset_type not in asset_types:
                        asset_types.append(asset_type)
                    
                    formatted_header = relative_header.as_posix()
                    if formatted_header not in header_files:
                        header_files.append(formatted_header)

        except Exception as e:
            print(f"Error reading {header_file}: {e}")

# Generate files
# Header file
header_file_name = output_dir_h /"AssetTypesAutoRegistration.h"
header_content = """#pragma once
//==========================================================
//This file is auto generated do not change the content
//==========================================================

#include "AssetManager/AssetStorageFactory.h"

namespace LE::AutoRegistration
{
void RegisterAllAssetTypes(AssetStorageFactory& Factory);
}
"""
# CPP file
cpp_file_name = output_dir_cpp /"AssetTypesAutoRegistration.cpp"
includes = "\n".join(f'#include "{header}"' for header in header_files)
construction_function = "ASSET_STORAGE_CONSTRUCTION_FUNC()"
register_calls = "\n    ".join(f"Factory.Register(AssetTypeIdGetter<{cls}>::Value, ASSET_STORAGE_CONSTRUCTION_FUNC({cls}));" for cls in asset_types)

cpp_content = f"""#include "AssetTypesAutoRegistration.h"
{includes}

namespace LE::AutoRegistration
{{
void RegisterAllAssetTypes(AssetStorageFactory& Factory)
{{
    {register_calls}
}}
}}
"""
# Write files
header_file_name.parent.mkdir(parents=True, exist_ok=True)
with open(header_file_name, "w", encoding="utf-8") as f:
    f.write(header_content)

cpp_file_name.parent.mkdir(parents=True, exist_ok=True)
with open(cpp_file_name, "w", encoding="utf-8") as f:
    f.write(cpp_content)

print(f"Generated {header_file_name.absolute()}")
print(f"Generated {cpp_file_name.absolute()}")
print(f"Found {len(asset_types)} Asset Types")

sys.exit(0)