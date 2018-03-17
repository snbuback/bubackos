#!/usr/bin/env python3
import os
import sys
import re

template = open("test/template.cpp").read()


def run(base_dir, debug=False):
    for root, dirs, files in os.walk(base_dir, topdown=False):
        for file in files:
            if file.startswith('test_') and (file.endswith('.c') or file.endswith('.cpp')):
                run_test(os.path.join(base_dir, file), debug)
        for dir in dirs:
            run(dir)


def run_test(file_name, debug=False):
    print("Running {}".format(file_name))
    with open("/tmp/teste.cpp", "w") as f:
        file_content = open(file_name).read()
        test_funcs = "\n".join(['run_func("{func_name}", &{func_name});'.format(func_name=func_name) for func_name in get_all_test(file_content)])

        test_file_content = template
        test_file_content = test_file_content.replace("//@test-include", file_content)
        test_file_content = test_file_content.replace("//@test-functions", test_funcs)
        f.write(test_file_content)
    run_command("x86_64-apple-darwin17.3.0-g++-7 -m64 -std=c++11 -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -I$SRC_DIR/include -I$SRC_DIR/arch/x86_64 -I. /tmp/teste.cpp -o /tmp/teste.bin", debug)
    if debug:
        run_command("gdb /tmp/teste.bin")
    else:
        run_command("/tmp/teste.bin")


def get_all_test(file_content):
    return re.findall(r'void (test_.*)\(\)', file_content)


def run_command(cmd, debug=False):
    cmd = cmd.replace("$SRC_DIR", os.path.join(os.getcwd(), 'src'))
    print("Cmd: {}".format(cmd))
    exit_code = os.system(cmd)
    if exit_code != 0:
        print("Command {} returned {}".format(cmd, exit_code))
        sys.exit(1)


if __name__ == '__main__':
    debug = len(sys.argv) > 1 and sys.argv[1] == '-debug'
    run("test", debug)

