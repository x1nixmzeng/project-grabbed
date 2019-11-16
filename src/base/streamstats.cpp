#include "streamstats.h"

#include <algorithm>

namespace grabbed
{
    namespace base
    {
        void streamstats::read(size_t pos, size_t length)
        {
            auto res = std::find_if(m_info.begin(), m_info.end(), [=](const infoPair& pairing)
            {
                return (pos >= pairing.first) && (pos <= (pairing.first + pairing.second));
            });

            if (res == m_info.end())
            {
                m_info.push_back(std::make_pair(pos, length));
            }
            else
            {
                size_t newSum = pos + length;
                size_t currentSum = res->first + res->second;

                if (newSum > currentSum)
                {
                    auto newLength = newSum - res->first;
                    res->second = newLength;

                    currentSum = res->first + res->second;

                    // Remove all (sorted) dupes
                    auto next = res;
                    next++;

                    while (next != m_info.end())
                    {
                        if (next->first <= newSum)
                        {
                            // Check if the old sum length can be inherited
                            size_t oldSum = next->first + next->second;
                            if (oldSum > currentSum)
                            {
                                newLength = oldSum - res->first;
                                res->second = newLength;

                                currentSum = res->first + res->second;
                            }

                            next = m_info.erase(next);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }

        size_t streamstats::getTotalRead() const
        {
            size_t result{ 0 };

            for (const auto& info : m_info) {
                result += info.second;
            }

            return result;
        }
    }
}
