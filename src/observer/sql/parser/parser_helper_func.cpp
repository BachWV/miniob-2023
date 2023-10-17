#include"parser_helper_func.h"
#include <cstring>
#include"event/sql_debug.h"


bool CheckLeap(int y, int m, int d)
{
    static int mon[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (y%400==0 || (y%100 && y%4==0));
    return y > 0
        && (m > 0)&&(m <= 12)
        && (d > 0)&&(d <= ((m==2 && leap)?1:0) + mon[m]);
}

// 'yyyy-mm-dd'，其中开始的双引号为第0个
// mm有可能是m，dd有可能是d
// 2001-1-1, 2001-01-1, 2001-1-01, 2001-01-01
bool CheckTimeRange(const char* s, int &time){
  string str(s);
  vector<int> nums(3);
  //sql_debug(s);
  int start = 1, end;
  for(auto& num : nums){
    int index = str.find('-', start) ;
    end = (index != -1) ? index : str.size()-1;
    num = stoi(string(s+start, end - start));
    start = end+1;
  }
  // 基础的month和day的检查，年份的检查用mktime转换后的tm_time检查
  if(!CheckLeap(nums[0], nums[1], nums[2])){
    return false;
  }

  time_t max = INT_MAX;
  struct tm tm_;
  memset(&tm_, 0, sizeof(struct tm));
  tm_.tm_year = nums[0] - 1900;
  tm_.tm_mon = nums[1] - 1;
  tm_.tm_mday = nums[2];
  tm_.tm_hour = 0;
  tm_.tm_min = 0;
  tm_.tm_sec = 0;

  // 给定的年份不能超过int最大值，且最低1970年
  time_t tm_time = mktime(&tm_);
  // if(tm_time ==2147558400) {
  //   time = 2147483647;
  //   return true;
  // }
  if(tm_time < 0 || tm_time > max){
    sql_debug("time out of range");
    sql_debug(std::to_string(tm_time).c_str());
    sql_debug(s);
   
    return false;
  }

  time = static_cast<int>(tm_time);
  return true;
}