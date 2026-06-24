import os
import re
import argparse
import sys
import posixpath

def log_info(message: str):
    sys.stderr.write(f"[Reflection] [Shader] {message}\n")
    sys.stderr.flush()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--header-files", nargs="+", required=True)
    parser.add_argument("--gen-folder", required=True)
    parser.add_argument("--master-header", required=True)
    parser.add_argument("--master-cpp", required=True)
    parser.add_argument("--module-name", required=True)
    parser.add_argument("--depfile", required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    generated_header_files = []
    generated_cpp_files = []
    generated_namespaces = []
    tracked_headers = []

    # Generate files for headers
    for header in args.header_files:
        variant_pattern = re.compile(
            r"REGISTER_RENDER_CONTRIBUTOR_TYPE\s*\(\s*(?P<contributor>[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)*)\s*,"
            r"|"
            r"REGISTER_SHADER_PASS_TYPE\s*\(\s*(?P<shader>[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)*)\s*,"
        )

        render_contributors = []
        shader_passes = []
        if posixpath.exists(header):
            with open(header, "r", errors="ignore") as f:
                contents = f.read()
                for match in variant_pattern.finditer(contents):
                    if match.group('contributor') == "Type" or match.group('shader') == "ShaderPassType":
                        continue

                    if match.group('contributor'):
                        render_contributors.append(match.group('contributor'))
                    elif match.group('shader'):
                        shader_passes.append(match.group('shader'))  
                
                             
        if not render_contributors and not shader_passes:
            continue
        
        log_info(f"{header} - Found auto registration macro, processing")

        rel_path = posixpath.basename(header)
        file_name, _ = posixpath.splitext(rel_path)
        
        gen_h = posixpath.join(args.gen_folder, f"{file_name}.gen.h")
        gen_cpp = posixpath.join(args.gen_folder, f"{file_name}.gen.cpp")
        gen_namespace = f"{file_name}File"

        gen_header_include = f"{file_name}.gen.h"

        generated_header_files.append(gen_h)
        generated_cpp_files.append(gen_cpp)
        generated_namespaces.append(gen_namespace)
        tracked_headers.append(header)

        if args.dry_run:
            continue

        if posixpath.exists(gen_cpp) and posixpath.getmtime(header) <= posixpath.getmtime(gen_cpp):
            log_info(f"{header} - Was not changed")
            continue

        header_content = f"""#pragma once
//==========================================================
//This file is auto generated do not change the content
//==========================================================
#include "ShaderVariationRegistry.h"

namespace LE::AutoRegistration::{gen_namespace}
{{
    void RegisterAllShaderVariations(LE::Renderer::ShaderVariationRegistry& Registry);
}}
"""

        contributor_register_calls = "\n        ".join(f"Registry.RegisterRenderContributorShaderFile<{cls}>();" for cls in render_contributors)
        shader_register_calls = "\n        ".join(f"Registry.RegisterShaderPassFile<{cls}>();" for cls in shader_passes)
        
        cpp_content = f"""#include "{file_name}.gen.h"
#include "{header}"

namespace LE::AutoRegistration::{gen_namespace}
{{
    void RegisterAllShaderVariations(LE::Renderer::ShaderVariationRegistry& Registry)
    {{
        {contributor_register_calls}
        {shader_register_calls}
    }}
}}
"""
        os.makedirs(posixpath.dirname(gen_cpp), exist_ok=True)
        with open(gen_h, "w") as f: f.write(header_content)
        with open(gen_cpp, "w") as f: f.write(cpp_content)
    
    if not generated_header_files:
        log_info("No shader registration files")
        sys.exit(0)

    # Dry run output predicted generated files
    if args.dry_run:
        generated_cpp_files.append(args.master_cpp)
        generated_header_files.append(args.master_header)
        
        normalized_cpp = [posixpath.normpath(p) for p in generated_cpp_files]
        normalized_headers = [posixpath.normpath(p) for p in generated_header_files]

        all_predicted_files = normalized_cpp + normalized_headers
        log_info(f"Predicted files: {len(all_predicted_files)}")

        sys.stdout.write(";".join(all_predicted_files))
        sys.exit(0)

    # Generate master files
    master_cpp = args.master_cpp
    with open(master_cpp, "w") as f:
        rel_master_path = posixpath.basename(args.master_header)
        master_file_name, _ = posixpath.splitext(rel_master_path)
        f.write(f'#include "{master_file_name}.h"\n')
       
        for generated_header in generated_header_files:
            rel_path = posixpath.basename(generated_header)
            file_name, _ = posixpath.splitext(rel_path)
            f.write(f'#include "{file_name}.h"\n')

        f.write("\n")
        register_calls = "\n        ".join(f"{namespace}::RegisterAllShaderVariations(Registry);" for namespace in generated_namespaces)
        f.write(f"""namespace LE::AutoRegistration::{args.module_name}
{{
    void RegisterAllShaderVariations(LE::Renderer::ShaderVariationRegistry& Registry)
    {{
        {register_calls}
    }}
}}
"""
        )
    os.makedirs(posixpath.dirname(args.master_header), exist_ok=True)
    with open(args.master_header, "w") as f:
        f.write(f"""#pragma once
//==========================================================
//This file is auto generated do not change the content
//==========================================================
#include "ShaderVariationRegistry.h"
                
namespace LE::AutoRegistration::{args.module_name}
{{
    void RegisterAllShaderVariations(LE::Renderer::ShaderVariationRegistry& Registry);
}}
"""
        )
    
    #Generate depfile
    with open(args.depfile, "w") as f:
        if not tracked_headers:
            f.write(f"{master_cpp}: {__file__}\n")
        else:
            dependencies_str = " ".join(tracked_headers)
            f.write(f"{master_cpp}: {dependencies_str}\n")
    
    log_info(f"Finished: {len(generated_header_files)} files were created")

if __name__ == "__main__":
    main()