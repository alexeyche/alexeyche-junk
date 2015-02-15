#pragma once

#include <atomic>

class SpinningBarrier
{
public:
    SpinningBarrier (unsigned int n) : n_ (n), nwait_ (0), step_(0) {}

    bool wait ()
    {
        unsigned int step = step_.load ();

        if (nwait_.fetch_add (1) == n_ - 1)
        {
            /* OK, last thread to come.  */
            nwait_.store (0); // XXX: maybe can use relaxed ordering here ??
            step_.fetch_add (1);
            return true;
        }
        else
        {
            /* Run in circles and scream like a little girl.  */
            while (step_.load () == step)
                ;
            return false;
        }
    }

protected:
    /* Number of synchronized threads. */
    const unsigned int n_;

    /* Number of threads currently spinning.  */
    std::atomic<unsigned int> nwait_;

    /* Number of barrier syncronizations completed so far,
     * it's OK to wrap.  */
    std::atomic<unsigned int> step_;
};
