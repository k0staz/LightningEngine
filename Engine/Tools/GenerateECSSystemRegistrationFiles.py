import os
import re
import sys
from pathlib import Path

source_dir = Path("../../Source")
output_dir_h = Path("Generated/Public")
output_dir_cpp = Path("Generated/Private")

variant_pattern = re.compile(
    r"REGISTER_ECS_SYSTEM\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)"
)

system_classes = []
header_files = []

# Find all shaders

def get_relative_to_public(path: Path) -> Path:
    for parent in path.parents:
        if parent.name == "Public":
            return path.relative_to(parent)

for header_file in source_dir.rglob("*.h"):
    try:
        with open(header_file, "r", encoding="utf-8") as f:
            contents = f.read()
            matches = variant_pattern.findall(contents)
            for class_name in matches:
                if(class_name == "SystemName"):
                    continue

                relative_header = get_relative_to_public(header_file)
                if class_name not in system_classes:
                    system_classes.append(class_name)
                
                formatted_header = relative_header.as_posix()
                if formatted_header not in header_files:
                    header_files.append(formatted_header)

    except Exception as e:
        print(f"Error reading {header_file}: {e}")

# Generate files
# Header file
header_file_name = output_dir_h /"ECSSystemAutoRegistration.h"
header_content = """#pragma once
//==========================================================
//This file is auto generated do not change the content
//==========================================================

#include "ECS/EcsSystem.h"

namespace LE::ECSSystemRegistration
{
void RegisterAllSystems(EcsSystemManager& SystemManager);
}
"""
# CPP file
cpp_file_name = output_dir_cpp /"ECSSystemAutoRegistration.cpp"
includes = "\n".join(f'#include "{header}"' for header in header_files)
register_calls = "\n    ".join(f"SystemManager.RegisterSystem<{cls}>();" for cls in system_classes)

cpp_content = f"""#include "ECSSystemAutoRegistration.h"
{includes}

namespace LE::ECSSystemRegistration
{{
void RegisterAllSystems(EcsSystemManager& SystemManager)
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

print(f"Generated {header_file_name}")
print(f"Generated {cpp_file_name}")
print(f"Found {len(system_classes)} ECS Systems")

sys.exit(0)