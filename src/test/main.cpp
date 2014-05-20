/*
 * Copyright (C) 2012-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <elf/db.h>
#include <elf/net/net.h>
#include <elf/script/script.h>
#include <elf/timer.h>
#include <tut/tut.hpp>
#include <tut/tut_console_reporter.hpp>

#if defined(ELF_PLATFORM_WIN32)
#   include <vld/vld.h>
#else
#   define BEGIN_CRASH_DUMP try
#   define CATCH_CRASH_DUMP catch(...)
#endif /* ELF_PLATFORM_WIN32 */

using namespace tut;

namespace tut {
    test_runner_singleton runner;
}

void usage(void)
{
    printf("Usage:\n");
    printf("\ttest <group> [case] [repeat]\n");
    printf("\t<group>  - The name of test group, or `all` for regress.\n");
    printf("\t[case]   - The name of test case.\n");
    printf("\t[repeat] - The number of times to repeat each unique test group"
        " or case.\n\n");
}

void test(int argc, char **argv)
{
    console_reporter cb(std::cout);

    runner.get().set_callback(&cb);
    try {
        if (strcmp(argv[1], "all") == 0) {
            runner.get().run_tests();
        } else {
            runner.get().run_tests(argv[1]);
        }
    } catch(const tut::no_such_group &ex) {
        std::cerr << "No such group: " << ex.what() << std::endl;
    } catch(const tut::tut_error &ex) {
        std::cout << "General error: " << ex.what() << std::endl;
    } catch(...) {
        std::cout << "Unknown error: " << std::endl;
    }
}

int main(int argc, char **argv)
{
    // char version[16] = ELF_VERSION_STR;
    if (argc < 2) {
        usage();
        exit(EXIT_FAILURE);
    }

    BEGIN_CRASH_DUMP {
        ELF_INIT(log);
        ELF_INIT(timer);
        ELF_INIT(db);
        ELF_INIT(script);
        ELF_INIT(net);
        //ELF_INIT(http);

        test(argc, argv);

        //ELF_FINI(http);
        ELF_FINI(net);
        ELF_FINI(script);
        ELF_FINI(db);
        ELF_FINI(timer);
        ELF_FINI(log);
        exit(EXIT_SUCCESS);
    } CATCH_CRASH_DUMP {
        exit(EXIT_FAILURE);
    }
}

