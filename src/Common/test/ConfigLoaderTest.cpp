//
// Created by CANDO on 21/02/2026.
//

#include "../ConfigLoader.h"
#include <iostream>
#include <cassert>

void testConfigLoading()
{
#ifdef CONFIG_PATH
    ConfigLoader config(CONFIG_PATH);
#else
    ConfigLoader config("config.yaml");
#endif

    // Test simple value
    assert(config.getValue("Notifications:enabled") == "true");

    // Test nested value
    assert(config.getValue("Hardware_output:buzzer:volume") == "50");

    // Test list/comma value
    assert(config.getValue("Notifications:smtp_port") == "587");

    // Test default value for non-existent key
    assert(config.getValue("NonExistent:Key", "default") == "default");

    std::cout << "ConfigLoader Tests Passed!" << std::endl;
}

int main()
{
    try
    {
        testConfigLoading();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
