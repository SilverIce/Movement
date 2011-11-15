#pragma once

class RdtscTimer
{
public:
    typedef unsigned __int64 rt_time;

    explicit RdtscTimer() { reset();}

    rt_time passed() const { return passed_time;}
    rt_time avg() const { return (callsCount ? passed_time / callsCount : 0);}
    rt_time count() const { return callsCount;}
    bool InProgress() const { return inside;}

    void onCallBegin()
    {
        if (!inside) {
            inside = true;
            start_call = now();
        }
    }

    void onCallEnd()
    {
        if (inside) {
            inside = false;
            passed_time += (now() - start_call);
            ++callsCount;
        }
    }

    void Pause()
    {
        if (inside) {
            inside = false;
            passed_time += (now() - start_call);
        }
    }

    void UnPause()
    {
        if (!inside) {
            inside = true;
            start_call = now();
        }
    }

    void reset() {
        start_call = passed_time = 0;
        callsCount = 0;
        inside = false;
    }

private:
    static rt_time now()
    {
        __asm rdtsc;
    }
    rt_time start_call;
    rt_time passed_time;
    rt_time callsCount;
    bool inside;
};

class RdtscCall
{
    RdtscTimer& timer;
public:
    explicit RdtscCall(RdtscTimer& tm) : timer(tm) { timer.onCallBegin();}
    explicit RdtscCall(RdtscTimer* tm) : timer(*tm) { timer.onCallBegin();}
    ~RdtscCall() { timer.onCallEnd(); }
};

class RdtscInterrupt
{
    RdtscTimer& timer;
public:
    explicit RdtscInterrupt(RdtscTimer& tm) : timer(tm) { timer.Pause();}
    explicit RdtscInterrupt(RdtscTimer* tm) : timer(*tm) { timer.Pause();}
    ~RdtscInterrupt() { timer.UnPause(); }
};
