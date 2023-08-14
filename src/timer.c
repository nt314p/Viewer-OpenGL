#define GLEW_STATIC
#include <GL\glew.h>
#include "timer.h"

#define UninitializedTimerId 999999 // Cross fingers no conflicts

unsigned int timerQuery = UninitializedTimerId;

void TimerStart()
{
    if (timerQuery == UninitializedTimerId)
    {
        glGenQueries(1, &timerQuery);
    }

    glBeginQuery(GL_TIME_ELAPSED, timerQuery);
}

void TimerStop()
{
    glEndQuery(GL_TIME_ELAPSED);
}

unsigned int TimerGetNanosecondsElapsed()
{
    unsigned int elapsedNs;
    glGetQueryObjectuiv(timerQuery, GL_QUERY_RESULT, &elapsedNs);
    return elapsedNs;
}