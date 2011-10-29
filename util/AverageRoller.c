#include <time.h>
#include <string.h>

#include "util/AverageRoller.h"

/** Used to represent the sum and entry count for a given second. */
struct SumAndEntryCount
{
    /** Sum of all entries. */
    uint32_t sum;

    /** Number of entries. */
    uint32_t entryCount;
};

/**
 * A structure for helping calculate a rolling average over some number of seconds.
 * Each element in "seconds" array is responsable for 1 second and it is a circular array.
 */
struct AverageRoller
{
    /** The average will be calculated over this number of seconds. */
    const uint32_t windowSeconds;

    /** The number of seconds since the epoch when the average was last updated. */
    uint32_t lastUpdateTime;

    /** The index of the array when the roller was last updated. */
    uint32_t lastUpdateIndex;

    /** The sum of every entry in the last windowSeconds seconds. */
    uint32_t sum;

    /** The total number of entries in the last windowSeconds seconds. */
    uint32_t entryCount;

    /** A stored value equal to the sum divided by the entry count. */
    uint32_t average;

    /**
     * An array of entries containing sum and entry count for each second
     * in the seconds prior to the last update.
     * This array pretends to contain one entry but actually contans windowSeconds entries.
     */
    struct SumAndEntryCount seconds[1];
};

/** @see AverageRoller.h */
struct AverageRoller* AverageRoller_new(const uint32_t windowSeconds, const struct MemAllocator* allocator)
{
    struct AverageRoller* roller = allocator->calloc(
        sizeof(struct AverageRoller) + (sizeof(struct SumAndEntryCount) * (windowSeconds - 1)),
        1, allocator);

    struct AverageRoller tempRoller = {
        .windowSeconds = windowSeconds,
        .lastUpdateTime = (uint32_t) time(NULL)
    };

    memcpy(roller, &tempRoller, sizeof(struct AverageRoller));

    return roller;
}

/** @see AverageRoller.h */
uint32_t AverageRoller_getAverage(struct AverageRoller* roller)
{
    return roller->average;
}

/**
 * Update the roller with a new entry.
 *
 * @param roller the roller to update.
 * @param now the number of seconds since the epoch.
 * @param newEntry the a new number to be factored into the average.
 * @return the average over the last windowSeconds seconds.
 */
static inline uint32_t update(struct AverageRoller* roller, const time_t now, const uint32_t newEntry)
{
    uint32_t index =
        (now - roller->lastUpdateTime + roller->lastUpdateIndex) % roller->windowSeconds;

    if ((uint32_t) now > roller->lastUpdateTime) {
        roller->sum -= roller->seconds[index].sum;
        roller->entryCount -= roller->seconds[index].entryCount;
        roller->seconds[index].sum = newEntry;
        roller->seconds[index].entryCount = 1;
    } else {
        roller->seconds[index].sum += newEntry;
        roller->seconds[index].entryCount++;
    }
    roller->sum += newEntry;
    roller->entryCount++;
    roller->average = roller->sum / roller->entryCount;
    roller->lastUpdateTime = now;
    roller->lastUpdateIndex = index;

    return roller->average;
}

/** @see AverageRoller.h */
uint32_t AverageRoller_update(struct AverageRoller* roller, const uint32_t newEntry)
{
    return update(roller, time(NULL), newEntry);
}
