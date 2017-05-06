#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>

#include "LoggerFwd.h"

namespace logger
{
class Category
{
private:
    friend class Logger;

    std::string m_name;
    Level m_level;
    LogWriter& m_writer;

    Category(const std::string& name, const Level level, LogWriter& writer);

public:
    ~Category() = default;
    Category(const Category&) = delete;
    Category(Category&&) = delete;
    Category& operator=(const Category&) = delete;
    Category& operator=(Category&&) = delete;

    const Level level() const;
    bool isCategoryLevel(const Level level) const;

    template <class ... Args>
    void error(const std::string& fmt, Args... args);

    template <class ... Args>
    void warn(const std::string& fmt, Args... args);

    template <class ... Args>
    void info(const std::string& fmt, Args... args);

    template <class ... Args>
    void debug(const std::string& fmt, Args... args);

    template <class ... Args>
    void log(const Level l, const std::string& fmt, Args... args);
};
} // namespace logger

#include "CategoryImpl.hpp"

#endif // CATEGORY_H