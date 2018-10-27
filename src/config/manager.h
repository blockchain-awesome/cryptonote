#pragma once;

namespace config
{
class IManager
{
    virtual bool load() = 0;
    virtual bool save() = 0;
};
} // namespace config