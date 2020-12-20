// DEF_PFUNC(RTYPECAP, RTYPEWORD, NAME, COMPILECODE)

DEF_PFUNC(VOID, Void, print_dbl, {
    PF_VERIFY_ARGCNT(1);
    PF_REQUIRE_CHILD_TYPE(0, Dbl);

    PF_COMPILE_CHILD(0);
    PF_O("out df:\n");
})

DEF_PFUNC(VOID, Void, print_int4, {
    PF_VERIFY_ARGCNT(1);
    PF_REQUIRE_CHILD_TYPE(0, Int4);

    PF_COMPILE_CHILD(0);
    PF_O("out dwl:\n");
})

DEF_PFUNC(VOID, Void, print_int8, {
    PF_VERIFY_ARGCNT(1);
    PF_REQUIRE_CHILD_TYPE(0, Int8);

    PF_COMPILE_CHILD(0);
    PF_O("out qw:\n");
})

DEF_PFUNC(DBL, Dbl, sqrt, {
    PF_VERIFY_ARGCNT(1);

    PF_REQUIRE_CHILD_TYPE(0, Dbl);

    PF_COMPILE_CHILD(0);
    PF_O("sqrt df:\n");
})

DEF_PFUNC(DBL, Dbl, sqrt, {
    PF_VERIFY_ARGCNT(1);

    PF_REQUIRE_CHILD_TYPE(0, Int4);

    PF_COMPILE_CHILD(0);
    PF_O("meow dwl:\n");
})

