#ifndef WATERING_CORE_H
#define WATERING_CORE_H

class WateringLogic
{
public:
  int targetHour;
  int targetMin;
  int intervalDays;
  int maxCycles;

  WateringLogic(int hour, int min, int interval, int max)
  {
    targetHour = hour;
    targetMin = min;
    intervalDays = interval;
    maxCycles = max;
  }

  // 核心判断函数
  bool shouldStart(int currentHour, int currentMin, int currentDayOfYear,
                   int wateredCount, int lastWateredDay, bool hasWateredToday)
  {

    if (wateredCount >= maxCycles)
      return false;
    // MTEST:START测试
    #ifdef SYSTEM_MANUAL_TEST
    if (SYSTEM_MANUAL_TEST)
      return true;
    #endif
    // MTEST:END测试结束
    if (currentHour != targetHour || currentMin != targetMin)
      return false;
    if (hasWateredToday)
      return false;
    if (lastWateredDay == -999)
      return true;

    int daysDiff = currentDayOfYear - lastWateredDay;
    if (daysDiff < 0)
      daysDiff += 365; // 跨年处理

    if (daysDiff >= intervalDays)
      return true;

    return false;
  }
};

#endif