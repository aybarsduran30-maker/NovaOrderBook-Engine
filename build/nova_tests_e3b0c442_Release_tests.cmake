add_test([=[OrderBookTest.AddSingleBuyOrder]=]  C:/Users/aybob/Desktop/NovaOrderBook-Engine/build/Release/nova_tests.exe [==[--gtest_filter=OrderBookTest.AddSingleBuyOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.AddSingleBuyOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:\Users\aybob\Desktop\NovaOrderBook-Engine\tests\test_orderbook.cpp:9]==]
    WORKING_DIRECTORY [==[C:/Users/aybob/Desktop/NovaOrderBook-Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.AddSingleSellOrder]=]  C:/Users/aybob/Desktop/NovaOrderBook-Engine/build/Release/nova_tests.exe [==[--gtest_filter=OrderBookTest.AddSingleSellOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.AddSingleSellOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:\Users\aybob\Desktop\NovaOrderBook-Engine\tests\test_orderbook.cpp:15]==]
    WORKING_DIRECTORY [==[C:/Users/aybob/Desktop/NovaOrderBook-Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.FullMatchExecution]=]  C:/Users/aybob/Desktop/NovaOrderBook-Engine/build/Release/nova_tests.exe [==[--gtest_filter=OrderBookTest.FullMatchExecution]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.FullMatchExecution]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:\Users\aybob\Desktop\NovaOrderBook-Engine\tests\test_orderbook.cpp:21]==]
    WORKING_DIRECTORY [==[C:/Users/aybob/Desktop/NovaOrderBook-Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.PartialFillExecution]=]  C:/Users/aybob/Desktop/NovaOrderBook-Engine/build/Release/nova_tests.exe [==[--gtest_filter=OrderBookTest.PartialFillExecution]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.PartialFillExecution]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:\Users\aybob\Desktop\NovaOrderBook-Engine\tests\test_orderbook.cpp:29]==]
    WORKING_DIRECTORY [==[C:/Users/aybob/Desktop/NovaOrderBook-Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.NoMatchWhenSpreadCrossesOpposite]=]  C:/Users/aybob/Desktop/NovaOrderBook-Engine/build/Release/nova_tests.exe [==[--gtest_filter=OrderBookTest.NoMatchWhenSpreadCrossesOpposite]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.NoMatchWhenSpreadCrossesOpposite]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:\Users\aybob\Desktop\NovaOrderBook-Engine\tests\test_orderbook.cpp:37]==]
    WORKING_DIRECTORY [==[C:/Users/aybob/Desktop/NovaOrderBook-Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.CancelOrderRemovesLiquidity]=]  C:/Users/aybob/Desktop/NovaOrderBook-Engine/build/Release/nova_tests.exe [==[--gtest_filter=OrderBookTest.CancelOrderRemovesLiquidity]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.CancelOrderRemovesLiquidity]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:\Users\aybob\Desktop\NovaOrderBook-Engine\tests\test_orderbook.cpp:46]==]
    WORKING_DIRECTORY [==[C:/Users/aybob/Desktop/NovaOrderBook-Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderBookTest.PoolAllocationRecycling]=]  C:/Users/aybob/Desktop/NovaOrderBook-Engine/build/Release/nova_tests.exe [==[--gtest_filter=OrderBookTest.PoolAllocationRecycling]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderBookTest.PoolAllocationRecycling]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:\Users\aybob\Desktop\NovaOrderBook-Engine\tests\test_orderbook.cpp:57]==]
    WORKING_DIRECTORY [==[C:/Users/aybob/Desktop/NovaOrderBook-Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(nova_tests_TESTS [==[OrderBookTest.AddSingleBuyOrder]==] [==[OrderBookTest.AddSingleSellOrder]==] [==[OrderBookTest.FullMatchExecution]==] [==[OrderBookTest.PartialFillExecution]==] [==[OrderBookTest.NoMatchWhenSpreadCrossesOpposite]==] [==[OrderBookTest.CancelOrderRemovesLiquidity]==] [==[OrderBookTest.PoolAllocationRecycling]==])
