BEGIN {
    print "#include <kernel/js_engine.h>"
    print "void gen_load_all_modules() {"
    print "const char* module_name;"
}

{
    gsub(/[\.\/]/, "_", $sym_name);
    print " // " $0;
    print " extern uintptr_t _binary_" $sym_name "_start;"
    print " extern size_t _binary_" $sym_name "_size;"
    print " module_name = \"" $0 "\";"
    print " js_engine_module_load(\"" $0 "\", &_binary_" $sym_name "_start, &_binary_" $sym_name "_size);"
    print ""
}

END {
    print "}"
}