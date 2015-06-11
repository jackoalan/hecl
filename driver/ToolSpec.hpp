#ifndef CTOOL_SPEC
#define CTOOL_SPEC

#include "ToolBase.hpp"
#include <stdio.h>
#include <map>

class ToolSpec final : public ToolBase
{
    enum Mode
    {
        MLIST = 0,
        MENABLE,
        MDISABLE
    } mode = MLIST;
public:
    ToolSpec(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (info.args.empty())
            return;

        if (!info.project)
            throw HECL::Exception(_S("hecl spec must be ran within a project directory"));

        const auto& specs = info.project->getDataSpecs();
        HECL::SystemString firstArg = info.args[0];
        HECL::ToLower(firstArg);

        static const HECL::SystemString enable(_S("enable"));
        static const HECL::SystemString disable(_S("disable"));
        if (!firstArg.compare(enable))
            mode = MENABLE;
        else if (!firstArg.compare(disable))
            mode = MDISABLE;
        else
            return;

        if (info.args.size() < 2)
            throw HECL::Exception(_S("Speclist argument required"));

        for (auto it = info.args.begin()+1;
             it != info.args.end();
             ++it)
        {

            bool found = false;
            for (auto& spec : specs)
            {
                if (!spec.first.name.compare(*it))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                throw HECL::Exception(_S("'") + *it + _S("' is not found in the dataspec registry"));
        }
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-spec - Configure target data options\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl spec [enable|disable] [<specname>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command configures the HECL project with the user's preferred target DataSpecs.\n\n"
                  "Providing enable/disable argument will bulk-set the enable status of the provided spec(s)"
                  "list. If enable/disable is not provided, a list of supported DataSpecs is printed.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<specname>..."), _S("DataSpec name(s)"));
        help.beginWrap();
        help.wrap(_S("Specifies platform-names to enable/disable"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("spec");}

    int run()
    {
        if (!m_info.project)
        {
            for (const HECL::Database::DataSpecEntry* spec = HECL::Database::DATA_SPEC_REGISTRY;
                 spec->name.size();
                 ++spec)
            {
                if (XTERM_COLOR)
                    HECL::Printf(_S("" BOLD CYAN "%s" NORMAL "\n"), spec->name.c_str());
                else
                    HECL::Printf(_S("%s\n"), spec->name.c_str());
                HECL::Printf(_S("  %s\n"), spec->desc.c_str());
            }
            return 0;
        }

        const auto& specs = m_info.project->getDataSpecs();
        if (mode == MLIST)
        {
            for (auto& spec : specs)
            {
                if (XTERM_COLOR)
                    HECL::Printf(_S("" BOLD CYAN "%s" NORMAL ""), spec.first.name.c_str());
                else
                    HECL::Printf(_S("%s"), spec.first.name.c_str());
                if (spec.second)
                {
                    if (XTERM_COLOR)
                        HECL::Printf(_S(" " BOLD GREEN "[ENABLED]" NORMAL ""));
                    else
                        HECL::Printf(_S(" [ENABLED]"));
                }
                HECL::Printf(_S("\n  %s\n"), spec.first.desc.c_str());
            }
            return 0;
        }

        std::vector<HECL::SystemString> opSpecs;
        for (auto it = m_info.args.begin()+1;
             it != m_info.args.end();
             ++it)
        {
            HECL::SystemString itName = *it;
            HECL::ToLower(itName);
            for (auto& spec : specs)
            {
                if (!spec.first.name.compare(itName))
                {
                    opSpecs.push_back(spec.first.name);
                    break;
                }
            }
        }

        if (opSpecs.size())
        {
            if (mode == MENABLE)
                m_info.project->enableDataSpecs(opSpecs);
            else if (mode == MDISABLE)
                m_info.project->disableDataSpecs(opSpecs);
        }

        return 0;
    }
};

#endif // CTOOL_SPEC