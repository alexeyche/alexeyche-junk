#include <atomic>


namespace dnn {

class SpinningBarrier
{
public:
    SpinningBarrier (unsigned int n) : n_ (n), nwait_ (0), step_(0), _fail(false) {}

    bool wait ()
    {
        unsigned int step = step_.load ();
        if(_fail.load()) {
            throw dnnInterrupt();
        }
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
            while (step_.load () == step) { if(_fail.load()) break; }
            return false;
        }
    }
    void fail() {
        _fail.store(true);
    }
protected:
    /* Number of synchronized threads. */
    const unsigned int n_;
    std::atomic<bool> _fail;
    /* Number of threads currently spinning.  */
    std::atomic<unsigned int> nwait_;

    /* Number of barrier syncronizations completed so far, 
     * it's OK to wrap.  */
    std::atomic<unsigned int> step_;
};

}