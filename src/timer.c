#define GLEW_STATIC
#include <GL\glew.h>
#include "timer.h"

static int isTimerInitialized = 0;
static unsigned int timerQuery = 0;

void TimerStart()
{
    if (isTimerInitialized == 0)
    {
        glGenQueries(1, &timerQuery);
        isTimerInitialized = 1;
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