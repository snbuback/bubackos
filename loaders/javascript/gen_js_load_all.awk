BEGIN {
    print "#include <javascript/js_engine.h>"
    print "void gen_load_all_modules() {"
}

{
    gsub(/[\.\/]/, "_", $sym_name);
    print " // " $0;
    print " extern uintptr_t _binary_" $sym_name "_start;"
    print " extern size_t _binary_" $sym_name "_size;"
    print " js_engine_module_load(\"" $0 "\", (const char*) &_binary_" $sym_name "_start, (size_t) &_binary_" $sym_name "_size);"
    print ""
}

END {
    print "}"
}