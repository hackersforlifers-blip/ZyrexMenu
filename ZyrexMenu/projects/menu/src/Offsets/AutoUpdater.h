#pragma once

namespace AutoUpdater
{
    // Downloads and applies offsets from https://offsets.imtheo.lol/
    // Returns number of offsets updated
    int update_offsets();
    
    // Downloads and applies both offsets + FFlagList pointers
    void update_all();
}
