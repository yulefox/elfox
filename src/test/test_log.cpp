/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/elf.h>
#include <tut/tut.hpp>

#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/spi/factory.h>
#include <log4cplus/spi/loggingevent.h>

namespace log4cplus {
class TestAppender
    : public Appender
{
    public:
    TestAppender();
    explicit TestAppender(helpers::Properties const &);
    virtual ~TestAppender();

    virtual void close();

    static void registerAppender();

    protected:
    virtual void append(spi::InternalLoggingEvent const &);

    private:
    explicit TestAppender(TestAppender const &);
    TestAppender & operator =(TestAppender const &);
};

typedef helpers::SharedObjectPtr<TestAppender> Qt4DebugAppenderPtr;

TestAppender::TestAppender()
    : Appender()
{ }


TestAppender::TestAppender(helpers::Properties const &props)
    : Appender(props)
{ }


TestAppender::~TestAppender()
{
    destructorImpl();
}


void
TestAppender::close()
{ }


void
TestAppender::append(spi::InternalLoggingEvent const &ev)
{
    // @todo (Fox): Expose log4cplus' internal TLS to use here.
    tostringstream oss;
    layout->formatAndAppend(oss, ev);
}


void
TestAppender::registerAppender()
{
    log4cplus::spi::AppenderFactoryRegistry &reg
        = log4cplus::spi::getAppenderFactoryRegistry();
    LOG4CPLUS_REG_APPENDER(reg, TestAppender);
}
};

namespace tut {
struct log {
    log() {
    }

    ~log() {
    }
};

typedef test_group<log> factory;
typedef factory::object object;

static tut::factory tf("log");

template<>
template<>
void object::test<1>() {
    set_test_name("log");
    putchar('\n');
    LOG_FATAL("event", "%s", "F-hello");
    LOG_ERROR("event", "%s", "E-hello");
    LOG_WARN("event", "W-hello %s", "hello");
    LOG_INFO("event", "%s", "I-hello");
    LOG_TRACE("event", "%s", "T-hello");
    LOG_DEBUG("event", "%s", "D-hello");
    LOG_TEST("%s", "hello");
}
}
