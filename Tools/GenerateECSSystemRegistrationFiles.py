import os
import re
import argparse
import sys
import posixpath

def log_info(message: str):
    sys.stderr.write(f"[Reflection ECS] {message}\n")
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
        variant_pattern = re.compile(r"REGISTER_ECS_SYSTEM\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)")
       
        system_classes = []
        if posixpath.exists(header):
            with open(header, "r", errors="ignore") as f:
                contents = f.read()
                matches = variant_pattern.findall(contents)
                for class_name in matches:
                    if(class_name == "SystemName"):
                        continue

                    system_classes.append(class_name)
                             
        if not system_classes:
            continue
        
        rel_path = posixpath.basename(header)
        file_name, _ = posixpath.splitext(rel_path)
        
        gen_h = posixpath.join(args.gen_folder, f"{file_name}.gen.h")
        gen_cpp = posixpath.join(args.gen_folder, f"{file_name}.gen.cpp")
        gen_namespace = f"{file_name}File"
        
        generated_header_files.append(gen_h)
        generated_cpp_files.append(gen_cpp)
        generated_namespaces.append(gen_namespace)
        tracked_headers.append(header)

        if args.dry_run:
            continue

        if posixpath.exists(gen_cpp) and posixpath.getmtime(header) <= posixpath.getmtime(gen_cpp):
            continue

        header_content = f"""#pragma once
        //==========================================================
        //This file is auto generated do not change the content
        //==========================================================

        #include "ECS/EcsSystem.h"

        namespace LE::AutoRegistration::{gen_namespace}
        {{
        void RegisterAllSystems(EcsSystemManager& SystemManager);
        }}
        """

        register_calls = "\n    ".join(f"SystemManager.RegisterSystem<{cls}>();" for cls in system_classes)
        cpp_content = f"""#include "{file_name}.gen.h"
        #include "{header}"

        namespace LE::AutoRegistration::{gen_namespace}
        {{
        void RegisterAllSystems(EcsSystemManager& SystemManager)
        {{
            {register_calls}
        }}
        }}
        """
        os.makedirs(posixpath.dirname(gen_cpp), exist_ok=True)
        with open(gen_h, "w") as f: f.write(header_content)
        with open(gen_cpp, "w") as f: f.write(cpp_content)
    
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
        register_calls = "\n    ".join(f"{namespace}::RegisterAllSystems(SystemManager);" for namespace in generated_namespaces)
        f.write(
            f"""
            namespace LE::AutoRegistration::{args.module_name}
            {{
            void RegisterAllSystems(EcsSystemManager& SystemManager)
            {{
                {register_calls}
            }}
            }}
            """
        )
    os.makedirs(posixpath.dirname(args.master_header), exist_ok=True)
    with open(args.master_header, "w") as f:
        f.write(
            f"""#pragma once
            //==========================================================
            //This file is auto generated do not change the content
            //==========================================================

            #include "ECS/EcsSystem.h"

            namespace LE::AutoRegistration::{args.module_name}
            {{
            void RegisterAllSystems(EcsSystemManager& SystemManager);
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

if __name__ == "__main__":
    main()