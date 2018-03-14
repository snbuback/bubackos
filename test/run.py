#!/usr/bin/env python3
import os
import re

template = open("test/template.cpp").read()

def run(base_dir):
    for root, dirs, files in os.walk(base_dir, topdown=False):
        for file in files:
            if file.startswith('test_') and (file.endswith('.c') or file.endswith('.cpp')):
                run_test(os.path.join(base_dir, file))
        for dir in dirs:
            run(dir)

def run_test(file_name):
    print("Running {}".format(file_name))
    with open("/tmp/teste.cpp", "w") as f:
        file_content = open(file_name).read()
        test_funcs = "\n".join(['run_func("{func_name}", &{func_name});'.format(func_name=func_name) for func_name in get_all_test(file_content)])

        test_file_content = template
        test_file_content = test_file_content.replace("//@test-include", file_content)
        test_file_content = test_file_content.replace("//@test-functions", test_funcs)
        f.write(test_file_content)
    run_command("x86_64-apple-darwin17.3.0-g++-7 -std=c++11 -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -I$SRC_DIR/include -I$SRC_DIR/arch/x86_64 -I. /tmp/teste.cpp -o /tmp/teste.bin")
    run_command("/tmp/teste.bin")


def get_all_test(file_content):
    return re.findall(r'void (test_.*)\(\)', file_content)

def run_command(cmd):
    os.system(cmd)


if __name__ == '__main__':
    run("test")

