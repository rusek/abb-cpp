#include "base.h"

HitCounter * HitCounter::current_ptr = nullptr;

HitCounter::HitCounter():
    hits(0),
    expectedHits(0)
{
    ABB_ASSERT(HitCounter::current_ptr == nullptr, "HitCounter instance already set");
    HitCounter::current_ptr = this;
}

HitCounter::~HitCounter() {
    REQUIRE_EQUAL(this->hits, this->expectedHits);
    HitCounter::current_ptr = nullptr;
}

void HitCounter::hit() {
    this->hits++;
}

void HitCounter::hit(std::uint32_t hitIndex)
{
    REQUIRE_EQUAL(this->hits, hitIndex);
    this->hits++;
}

void HitCounter::expectHits(std::uint32_t expectedHits) {
    this->expectedHits = expectedHits;
}

HitCounter & HitCounter::current() {
    ABB_ASSERT(HitCounter::current_ptr != nullptr, "HitCounter instance not set");
    return *HitCounter::current_ptr;
}
