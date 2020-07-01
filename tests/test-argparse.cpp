#include "unittestframework.h"

#include "argparse.h"
#include "argparse.h"

TEST_CASE("argparse") {
    SECTION("test") {
        const char*argv[] = {
            "pgmname",               // not counted in the option list
            "-a", "-123",            // mask 0x0001, counted in nargs,  value checked
            "--bigword",             // mask 0x0200, counted in nbig
            "-b123",                 // mask 0x0002, counted in nargs
            "-pear", "0x1234",       // mask 0x0004, counted in nargs,  value checked
            "-vvv",                  // mask 0x0400, counted in nargs, multiplicity checked
            "-xxy",                  // mask 0x4000, counted in nargs, failed multiplicity checked
            "-pTEST",                // mask 0x0008, counted in nargs, value checked
            "--apple=test",          // mask 0x0010, counted in nargs, value checked
            "--equal==test",         // mask 0x2000, counted in nargs, value checked
            "--bigword",             // mask 0x0200, counted in nbig
            "--long", "999",         // mask 0x0020, counted in nargs, value checked
            "firstfile",             // mask 0x0080, counted in nfiles, value checked
            "secondfile",            // mask 0x0100, counted in nfiles, value checked,
            "-",                     // mask 0x0040, counted in nstdin 
            "--",                    // mask 0x1000, counted in nrends
            "moreargs", "-v", "-"    // mask 0x0800, counted in nextra
        };
        int argc = sizeof(argv)/sizeof(*argv);
        int nfiles = 0;
        int nextra = 0;
        int argmask = 0;
        int nargs = 0;
        int nbig = 0;
        int nstdin = 0;
        int nrends = 0;
        for (auto& arg : ArgParser(argc, argv))
            if (nrends) {
                nextra ++;
                argmask |= 0x0800;
            }
            else switch(arg.option())
            {
                case 'a':
                    CHECK( arg.getint() == -123 );
                    argmask |= 0x0001;
                    nargs ++;
                    break;

                case 'b':
                    CHECK( arg.getuint() == 123 );
                    argmask |= 0x0002;
                    nargs ++;
                    break;
                case 'p':
                    if (arg.match("-pear")) {
                        CHECK( arg.getint() == 0x1234 );
                        argmask |= 0x0004;
                        nargs ++;
                    }
                    else {
                        CHECK( std::string(arg.getstr()) == "TEST" );
                        argmask |= 0x0008;
                        nargs ++;
                    }
                    break;
                case 'v':
                    CHECK( arg.count() == 3 );
                    argmask |= 0x0400;
                    nargs ++;
                    break;
                case 'x':
                    CHECK_THROWS( arg.count() );   // counted options must all be equal
                    argmask |= 0x4000;
                    nargs ++;
                    break;

                case '-':
                    if (arg.match("--big")) {
                        nbig++;
                        CHECK_THROWS( arg.count() );  // can't count a long option
                        argmask |= 0x0200;
                    }
                    else if (arg.match("--bigword")) {
                        // --big should match before --bigword
                        CHECK( false );
                    }
                    else if (arg.match("--unused")) {
                        CHECK( false );
                    }
                    else if (arg.match("--apple")) {
                        CHECK( std::string(arg.getstr()) == "test" );
                        argmask |= 0x0010;
                        nargs ++;
                    }
                    else if (arg.match("--equal")) {
                        CHECK( std::string(arg.getstr()) == "=test" );
                        argmask |= 0x2000;
                        nargs ++;
                    }
                    else if (arg.match("--long")) {
                        CHECK( arg.getint() == 999 );
                        argmask |= 0x0020;
                        nargs ++;
                    }
                    else if (arg.optionterminator()) {
                        nrends ++;
                        argmask |= 0x1000;
                    }
                    else {
                        INFO( "unexpected long option" );
                        CHECK( false );
                    }
                    break;
                case 0:
                    CHECK( std::string(arg.getstr()) == "-" );
                    nstdin ++;
                    argmask |= 0x0040;
                    break;
                case -1:
                    switch(nfiles++)
                    {
                        case 0:
                            CHECK( std::string(arg.getstr()) == "firstfile" );
                            argmask |= 0x0080;
                            break;
                        case 1:
                            CHECK( std::string(arg.getstr()) == "secondfile" );
                            argmask |= 0x0100;
                            break;
                        default:
                            INFO( "expected only two non options args" );
                            CHECK( false );
                    }
                    break;
                default:
                    INFO( "unexpected option" );
                    CHECK( false );
            }
        CHECK( nstdin == 1 );
        CHECK( nrends == 1 );
        CHECK( nfiles == 2 );
        CHECK( nextra == 3 );
        CHECK( nargs == 9 );
        CHECK( nbig == 2 );
        CHECK( argmask == 0x7FFF );
    }
    SECTION("testerrors") {
        const char*argv[] = {
            "pgmname",               // not counted in the option list
            "-a"        };
        int argc = sizeof(argv)/sizeof(*argv);
        int argmask = 0;
        int nargs = 0;
        for (auto& arg : ArgParser(argc, argv))
            switch(arg.option())
            {
                case 'a':
                    CHECK_THROWS( arg.getint() );
                    argmask |= 0x0001;
                    nargs ++;
                    break;
                default:
                    INFO( "unexpected option" );
                    CHECK( false );
            }
        CHECK( nargs == 1 );
        CHECK( argmask == 0x0001 );
    }

}

