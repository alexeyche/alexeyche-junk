
#include <time.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int day = 1;
    int mon = 1;
    int year = 1970;
    struct tm drec[1];
    static char week[4];

    if(argc > 1) {
//        sscanf(argv[1], "%d%*[.]%d%*[.]%d%*[.]", &day, &mon, &year);
        sscanf(argv[1], "%d.%d.%d", &day, &mon, &year);
    } else {
        return(-1);
    }    
    
    drec[0].tm_mday = day;
    drec[0].tm_mon = mon - 1;
    drec[0].tm_year = year - 1900;

    drec[0].tm_sec = drec[0].tm_min = drec[0].tm_hour = 0;

    if(mktime(drec) == -1) {
        return(-1);
    }

    strftime(week, 4, "%a", drec);

    puts(week);
    return(drec[0].tm_wday);
}
