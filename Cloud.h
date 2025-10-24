#pragma once
#include "Vkbase/VkResourcesDelegator.h"
class Cloud : public Vkbase::VkResourcesDelegator
{
private:
    void init();
    void createComputePipeline();
    void computeCloudData();

public:
    Cloud();
};