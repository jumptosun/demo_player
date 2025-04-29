#ifndef CLOCK_REFERENCE_H
#define CLOCK_REFERENCE_H

#include <atomic>
#include <chrono>

namespace dpl {

class IClockReference {
public:
    virtual ~IClockReference() { };

    /**
     * @brief SetFirstPts
     * 	the stream timestamp may not begin at 0, the caller should set
     * the first pts before invoke StartClock.
     * @param first_pts
     * @return
     */
    virtual int SetFirstPts(std::chrono::milliseconds first_pts) = 0;
    virtual int StartClock() = 0;
    virtual int StopClock() = 0;
    virtual int TogglePause() = 0;

    virtual std::chrono::milliseconds GetTime() = 0;
    /**
     * @brief GetPts
     * @return
     *  return the time_past + first_pts
     */
    virtual std::chrono::milliseconds GetPts() = 0;
    /**
     * @brief SeekTime
     * 	seek the time fron the start.
     * @param now
     */
    virtual void SeekTime(std::chrono::milliseconds now) = 0;
    /**
     * @brief SeekPts
     * 	seek the time fron the first pts. now = first_pts + time_past
     * @param now
     */
    virtual void SeekPts(std::chrono::milliseconds now) = 0;
};

/**
 * @brief GetClockReference
 *  return the clock singlton. thread-safed function.
 *  but you should save the instace pointer instead of invoking this function hourly,
 *  to avoid spending of mutex.
 * @param c
 * @return
 */
extern int GetClockReference(IClockReference** c);

}

#endif // CLOCK_REFERENCE_H
