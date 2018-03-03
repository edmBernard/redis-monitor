#pragma once
#ifndef TIME_UTILS_HPP_
#define TIME_UTILS_HPP_

namespace timeUtils {

inline std::time_t convertStrToTime(std::string stime) {
  std::tm tm;
  std::istringstream iss(stime);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S"); // or just %T in this case
  return timegm(&tm);
}

inline std::string convertTimeToStr(std::time_t time) {
  std::tm tm = *std::gmtime(&time);
  std::stringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

}; // namespace timeUtils

#endif // !TIME_UTILS_HPP_
