### usage

   
1. init

::  

    BTDC::init("CONFIG/btdc.json");

2. send topic

::

    BaseTopic *top = new BTDC::RegisterTopic();
    topic->Add("aa", 1);
    topic->Add("bb", "2");
    topic->Add("cc", 0.5f);
    topic->Send();


    EventTopic * topic = new BTDC::EventTopic(1111);
    topic->Send();


