#include "base.h"

HitCounter * HitCounter::currentPtr = nullptr;

HitCounter::HitCounter():
    hits(0),
    expectedHits(0)
{
    ABB_ASSERT(HitCounter::currentPtr == nullptr, "HitCounter instance already set");
    HitCounter::currentPtr = this;
}

HitCounter::~HitCounter() {
    REQUIRE_EQUAL(this->hits, this->expectedHits);
    HitCounter::currentPtr = nullptr;
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
    ABB_ASSERT(HitCounter::currentPtr != nullptr, "HitCounter instance not set");
    return *HitCounter::currentPtr;
}
