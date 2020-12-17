import os
from os import fdopen, remove
from tempfile import mkstemp
from shutil import move, copymode
import subprocess

COMPILE_WARNING = "#pragma warning (disable : 4125)\n"
PRAGMA_ONCE = "#pragma once\n"
WITH_UPROTOBUFF_FLAG_IF = "#if SL_WITH_PROTO\n"
WITH_UPROTOBUFF_FLAG_END = "#endif // SL_WITH_PROTO	\n"

# Modidy header file to be compatible with Unreal
def modify_header(header):
    # Create temp file
    fh, abs_path = mkstemp()
    with fdopen(fh, 'w') as new_file:
        new_file.write(COMPILE_WARNING)
        new_file.write(PRAGMA_ONCE)
        # Replace class declaration
        with open(header) as old_file:
            for line in old_file:
                new_file.write(line)

    # Copy the file permissions from the old file to the new file
    copymode(header, abs_path)
    # Remove original file
    remove(header)
    # Move new file
    move(abs_path, header)

# Modidy cc file to compatible with Unreal
def modify_cc(cc):
    # Create temp file
    fh, abs_path = mkstemp()
    with fdopen(fh, 'w') as new_file:
        new_file.write(WITH_UPROTOBUFF_FLAG_IF)
        # Replace class declaration
        with open(cc) as old_file:
            for line in old_file:
                new_file.write(line)
        new_file.write(WITH_UPROTOBUFF_FLAG_END)
    # Copy the file permissions from the old file to the new file
    copymode(cc, abs_path)
    # Remove original file
    remove(cc)
    # Move new file
    move(abs_path, cc)

# Get all the proto generated cc files
def get_proto_cc_files(proto_files):
    return [x.replace(".proto", "") + ".pb.cc" for x in proto_files]

# Get all the proto generated header files
def get_proto_header_files(proto_files):
    return [x.replace(".proto", "") + ".pb.h" for x in proto_files]

# Return all .proto file of current folder
def scan_proto_files():
    files_arr = os.listdir(".")
    return [x for x in files_arr if x.endswith(".proto")]

if __name__ == "__main__":
    proto_files = scan_proto_files()
    proto_headers = get_proto_header_files(proto_files)
    proto_ccs = get_proto_cc_files(proto_files)
    # Generate cpp files using protoc
    for proto_file in proto_files:
        subprocess.check_call(["protoc.exe", "--cpp_out=.", proto_file])
    for header in proto_headers:    
        modify_header(header)
    for cc in proto_ccs:
        modify_cc(cc)
