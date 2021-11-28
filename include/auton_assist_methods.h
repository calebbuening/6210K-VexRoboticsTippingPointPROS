#include "main.h"
extern int sgn(double d); // Mimimcs the mathematical sgn function
extern void driveViaIMU(double dist, double rotation);
extern void driveViaTime(double ms, double vel, double rotation);
extern void driveViaGPS(double locx, double locy);
extern void turnViaIMU(double rotation);
extern void grab();
extern void ungrab(); // NOTE: This has no wait, unlike the function above
extern void liftMin();
extern void liftSmall();
extern void liftMax();
extern void liftScore();
extern void liftHang();
extern void scoreGoal();
extern void judas();
extern void updateFilters();
extern double getFilteredGPS();