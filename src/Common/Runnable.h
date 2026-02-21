//
// Created by CANDO on 21/02/2026.
//

#ifndef CROWDLENS_RUNNABLE_H
#define CROWDLENS_RUNNABLE_H

// An interface which can be added
// to the receiving class. We call it runnable
// it contains a single method called "run"!
class Runnable
{
public:
    virtual ~Runnable() = default;
    virtual void run() = 0;
};


#endif //CROWDLENS_RUNNABLE_H