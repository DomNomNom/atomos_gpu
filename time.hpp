/*******************************\
| A small unitlity to get both  |
| windows and linux times.      |
| Time is returned in seconds.  |
\*******************************/

#pragma once

// must be called once near the beginning of the program
void time_init(void);

// gives the time since the last time_dt()
// if it has not been called before: since time_init()
float time_dt(void);

// the total time since time_init()
float time_sinceInit(void);
