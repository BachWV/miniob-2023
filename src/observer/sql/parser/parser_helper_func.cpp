#include"parser_helper_func.h"
#include <cstring>
#include <regex>


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
  int start = 0, end;
  for(auto& num : nums){
    int index = str.find('-', start) ;
    end = (index != -1) ? index : str.size();
    num = stoi(string(s+start, end - start));
    start = end+1;
  }
  // 基础的month和day的检查，年份的检查用mktime转换后的tm_time检查
  if(!CheckLeap(nums[0], nums[1], nums[2])){
    return false;
  }
  time=nums[0]*10000+nums[1]*100+nums[2];
  return true;

}

bool CheckDateFormat(const char* s){
  string str(s);
  std::regex  pattern("[0-9]{4}-[0-9]{1,2}-[0-9]{1,2}");
  return std::regex_match(str, pattern);
}