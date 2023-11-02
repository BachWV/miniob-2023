#include <string>
#include <time.h>
#include <climits>
#include <vector>
using namespace std;

// 非

// 检查闰年
bool CheckLeap(int y, int m, int d);
// 检查时间范围
bool CheckTimeRange(const char* s, int &time);

bool CheckDateFormat(const char* s);