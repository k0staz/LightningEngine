import os
import re
import sys
from pathlib import Path

source_dir = Path("Source")
output_dir_h = Path("Generated/Public")
output_dir_cpp = Path("Generated/Private")

variant_pattern = re.compile(
    r"DECLARE_MATERIAL_PASS_VARIANT\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)"
)

shader_classes = []
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
                if(class_name == "ShaderName"):
                    continue

                relative_header = get_relative_to_public(header_file)
                if class_name not in shader_classes:
                    shader_classes.append(class_name)
                
                formatted_header = relative_header.as_posix()
                if formatted_header not in header_files:
                    header_files.append(formatted_header)

    except Exception as e:
        print(f"Error reading {header_file}: {e}")

# Generate files
# Header file
header_file_name = output_dir_h /"MaterialShaderAutoRegistration.h"
header_content = """ #pragma once
//==========================================================
//This file is auto generated do not change the content
//==========================================================

namespace LE::Renderer
{
void RegisterAllMaterialShader();
}
"""
# CPP file
cpp_file_name = output_dir_cpp /"MaterialShaderAutoRegistration.cpp"
includes = "\n".join(f'#include "{header}"' for header in header_files)
register_calls = "\n    ".join(f"{cls}::RegisterMetaType();" for cls in shader_classes)

cpp_content = f"""#include "MaterialShaderAutoRegistration.h"
{includes}

namespace LE::Renderer
{{
    void RegisterAllMaterialShader()
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
print(f"Found {len(shader_classes)} shader classes")

sys.exit(0)