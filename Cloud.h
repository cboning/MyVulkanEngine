#pragma once
#include "Vkbase/ResourcesDelegator.h"
class Cloud : public Vkbase::ResourcesDelegator
{
private:
    void init();
    void createComputePipeline();
    void computeCloudData();

public:
    Cloud();
};